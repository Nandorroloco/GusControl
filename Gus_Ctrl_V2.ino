//////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////// 
// 
//  GusCtrl V2
//
//
//  License: http://creativecommons.org/licenses/by-sa/3.0/deed.es
//
//  Created by; nandorroloco
//  year: 2021
//////////////////////////////////////////////////////////////////
//  beta: version 1.1         06/04/2021
//////////////////////////////////////////////////////////////////
//  beta: version 2.0         01/06/2021
//           Sin nombre en los canales, ocupa mucha memoria el manejo de la eeprom
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//  alfa: version 2.1         08/08/2021
//           Pequeñas correciones, firmware liberado
//////////////////////////////////////////////////////////////////

// para el manejo del sensor de temperatura y humedad  (github)

#include <DHT.h>

//
// inicialización y declaración de variables globales
#define FALSE 0
#define TRUE 1

// Centinela para fin de comando, dependerá del emulador de terminal... poner el caracter que sea más adecuado, o añadir a la lista
#define FINCMD1 '\n'
#define FINCMD2 '\r'

const int pin_entradaAN[4] = {14, 15, 16, 17 };           // entradas analógicas para sensores  A0, A1, A2, A3
const int pin_salidaAN[4] = {9, 11, 6, 10 };     // salidas analógicas para PWM
int valor_salidaAN[4] = { 0, 0, 0, 0 };       // valores iniciales de las salidas analógicas

const int pin_salidaD[8] = { 2, 3, 4, 5, 7, 8, 18,19 };         // salidas digitales para Reles

int lectura = 0;


int destello = 0;
int i_destello = 80;

const int pin_warning = 13;

// donde conectarmos el DHT
#define DHTPIN 12          // what digital pin we're connected to
#define DHTTYPE DHT22      // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float h ; 
float t ;

         // maquina de estados
int estado = 000;   // estado inicial
int canal = 0;    // canal seleccionado por los comandos
int ratio = 0;
char accion = 'X';  
char comando = 'X'; 
char salida[60];

int ch = 0;
char in_c;


// funciones para leer y escribir en una salida PWD, sus valores van de 0..255 pero se codifican en porcentaje  0..99 además como es el control de un FET se emite el valor complementario 

int lee_PWM ( int i )
{
    return ( valor_salidaAN[i]);
}

void escribe_PWM ( int i, int valor )
{
    valor_salidaAN[i] = valor;
    analogWrite( pin_salidaAN[i], map(valor, 0, 99, 255, 0));
}


// inicialización dinámica
void setup() {
  // put your setup code here, to run once:
  for ( int i = 0; i < 4; i++)
          escribe_PWM( i, 0);

  pinMode( pin_warning, OUTPUT);
  digitalWrite( pin_warning, LOW);

  dht.begin();                        // inicializa el sensor de temperatura y humedad
  
  pinMode( pin_salidaD[0], OUTPUT);
  pinMode( pin_salidaD[1], OUTPUT);
  pinMode( pin_salidaD[2], OUTPUT);
  pinMode( pin_salidaD[3], OUTPUT);
  pinMode( pin_salidaD[4], OUTPUT);
  pinMode( pin_salidaD[5], OUTPUT);
  pinMode( pin_salidaD[6], OUTPUT);
  pinMode( pin_salidaD[7], OUTPUT); 
  
  for ( int i = 0; i < 8; i++)    // ponemos todos los reles a 1, porque usa lógica negativa
   digitalWrite( pin_salidaD[i], HIGH);


  // initialize serial communications 
  Serial.begin(9600);

#ifdef CON_BLUETOOTH  
  Serial.print("AT");  
  //Espera de medio segundo según datasheet entre envio de comandos AT  
  delay(500);  
  //Cambio de nombre donde se envia AT+NAME y seguido el nombre que deseemos  
  Serial.print("AT+NAMEGusCtrl_v2.1");  
  //Espera de medio segundo según datasheet entre envio de comandos AT  
  delay(500);  
  /*Cambio de la velocidad del modulo en baudios  
  Se envia AT+BAUD y seguido el numero correspondiente:  
     
   1 --> 1200 baudios   
   2 --> 2400 baudios  
   3 --> 4800 baudios  
   4 --> 9600 baudios (por defecto)  
   5 --> 19200 baudios  
   6 --> 38400 baudios  
   7 --> 57600 baudios  
   8 --> 115200 baudios  
  */ 
  
  Serial.print("AT+BAUD4");  
  //Espera de medio segundo según datasheet entre envio de comandos AT  
  delay(500);  
   //Configuracion Password, se envia AT+PIN y seguido password que queremos  
   Serial.print("AT+PIN1234");  
  delay(500);
   Serial.println();

#endif
 
   Serial.println("GusCtrl_v2.1");
   Serial.println("----------");
}




void parpadea()
{
     destello++;
     if ( destello == i_destello)
   {
     digitalWrite ( pin_warning, HIGH );   
   }
     if ( destello == (2* i_destello))
   {
   destello = 0;
   digitalWrite ( pin_warning, LOW );
   }
}


void actua_dht()
{
  Serial.println("Consulta sonda DHT22");
  h = dht.readHumidity(); 
  t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)){
    Serial.println("Error en sensor DHT");
//    return;
  }

  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print("\t");
  Serial.print("Temp: ");
  Serial.print(t);
  Serial.println("C");
}


void actua_pwm( )
{
if ( canal == 0 )
      {
      for ( int i = 0; i < 4; i++)    // leemos todos los canales PWM y escribimos su resutldato
         {
         switch (accion)
               {
         case '?':        // Consulta de los PWM
          sprintf(salida, "/PWM : %d nivel: %d", i+1, lee_PWM(i) );
          Serial.println(salida);
          break;
          case '+':
              ratio = 99;
          escribe_PWM( i, ratio);
          break;
          case '-':
              ratio =  0;
          escribe_PWM( i, ratio);
          break;         
          case 'R':
          escribe_PWM( i, ratio);
          break;
          default:
                break;
           }
      }
            }
        else
           {
      switch (accion)
               {
         case '?':        // Consulta de los PWM
          sprintf(salida, "PWM : %d nivel: %d",  canal, lee_PWM(canal-1) );
          Serial.println(salida);
          break;
          case '+':
              ratio = 99;
          escribe_PWM( canal-1, ratio);
          break;
          case '-':
              ratio =  0;
          escribe_PWM( canal-1, ratio);
          break;         
          case 'R':
          escribe_PWM( canal-1, ratio);
          break;
          default:
                break;
           }
      }
}   

void actua_anl()
{
int i=0;
int value = 0;
float celsius = 0.0; 

 switch(canal)
      {
   case 0:
      for ( i = 0; i < 2; i++)    // leemos dos sondas LM35
         {
         delay(10);
         value = analogRead(pin_entradaAN[i]);
         celsius = value *(500.0/ 1023.0); 
         sprintf(salida, "Sonda_T(%d) Temperatura :",  i+1 );
         Serial.print(salida);
         Serial.println(celsius );
         }
      for (; i < 4; i++)    // leemos el resto de los canales analógicos y escribimos su resutldato
         {
         delay(10);
         sprintf(salida, "Anl : %d Valor: %d",  i+1, analogRead(pin_entradaAN[i]) );
         Serial.println(salida);
         }
      break;
   case 1: case 2:
      value = analogRead(pin_entradaAN[canal-1]);
      celsius = value *(500.0/ 1023.0);
      sprintf(salida, "Sonda_T(%d) Temperatura :",  canal );
      Serial.print(salida);
      Serial.println(celsius );
      break;
   default:
      sprintf(salida, "Anl : %d Valor: %d",  canal, analogRead(pin_entradaAN[canal-1]) );
      Serial.println(salida);
      }
}
      
void actua_rele( )                    // usa lógica negativa, de manera que el estado se representa negado, y la inicialización es HIGH
{
  if ( canal == 0 )
      {
      for ( int i = 0; i < 8; i++)    // leemos todos los reles y escribimos su resutldato
         {
         switch (accion)
               {
         case '?':        // Consulta de los reles
          sprintf(salida, "Rele : %d Estado: %d", i+1, !digitalRead(pin_salidaD[i]) );
          Serial.println(salida);
          break;
          case '+':
                            digitalWrite( pin_salidaD[i], LOW);
          break;
          case '-':
                            digitalWrite( pin_salidaD[i], HIGH);
          break;         
          default:
                break;
           }
      }
            }
        else
           {
      switch (accion)
               {
         case '?':        // Consulta el rele
          sprintf(salida, "Rele : %d Estado: %d",  canal, !digitalRead(pin_salidaD[canal-1]) );
          Serial.println(salida);
          break;
          case '+':
                            digitalWrite( pin_salidaD[canal-1], LOW);
          break;
          case '-':
                            digitalWrite( pin_salidaD[canal-1], HIGH);
          break;         
          default:
                break;
           }
     }
}   
  
// Implementa el autómata de estados


void protocolo(char c)
{
switch (estado){
   case 000:      //estado inicial
   switch (c) {
      case '?':
         Serial.println ();
         Serial.println ( "Comandos implementados:");
         Serial.println (":A[0/<n>]? :T/:T? :R[<n>]<+/-/?>  :P[<n>]<+/-/?/r0..r99>");
         Serial.println ("--------------------------------------------------------");
         Serial.println (":A      -Analogico solo consulta");
         Serial.println ("    [0/<n>]?, 1..4 numero de entrada");          
         Serial.println (":T/:T?      -Temperatura y humedad solo consulta");
         Serial.println ("    sin parametros");   
         Serial.println (":R parte para Reles");
         Serial.println ("    [0/<n>] 1..8 numero de Rele");  
         Serial.println ("      <+/-/?> comando, + enciende, - apaga, ? consulta");
         Serial.println (":P parte de modulacion de potencia");
         Serial.println ("    [0/<n>] 1..4 numero de canal");  
         Serial.println ("      <+/-/?/r..r99> comando, + maximo, - minimo, ? consulta, r ratio");         
         estado = 000;          // no cambia de estado
         break;
      case ':':
         estado = 001;          //inicio de comando
         break;
       }
      comando = 'X';
      accion = 'X';
      ratio = 0;
      break;
   case 001:      //estado inicio de comando
   switch (c) {
      case 'A': case 'a':
         comando = 'A';
         estado = 010;    
         break;
      case 'R': case 'r':
         comando = 'R';
         estado = 020;    
         break;     
      case 'P': case 'p':
         comando = 'P';
         estado = 030;    
         break;
      case 'T': case 't':
         comando = 'T';
         estado = 070;         
         break;
      default:
      Serial.println ("-- Comando incorrecto ---");     
      estado = 000;           // nos lleva al estado inicial
      }
   break;
// bloque ANALOGICO
   case 010:      //estado identifica el canal  
     switch (c) {
      case '0': case '1': case '2': case '3': case '4':
         canal = (int) c - (int)'0';
         estado = 011;          //canal determinado
         break;
      case '?':
         canal = 0;
         accion = '?';
         estado = 999;          //Estado accion
         break;
      default:
         Serial.println ("-- Error en canal --");
         estado = 000;           // nos lleva al estado inicial
        }
      break;
   case 011:     //estado identifica acción
      switch (c) {
      case '?':
         accion = '?';
         estado = 999;          //Estado accion
         break;
      default:
         Serial.println ("-- Error en acción --");
         estado = 000;           // nos lleva al estado inicial
      }
   break;
   
// bloque Reles
   case 020:      //estado identifica el canal
   switch (c) {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
         canal = (int) c - (int)'0';
         estado = 021;          //canal determinado
         break;
      case '?':
         canal = 0;
         accion = '?';
         estado = 999;          //Estado accion
         break;
   default:
      Serial.println ("-- Error en canal --");
      estado = 000;           // nos lleva al estado inicial
      }
   break;
    case 021:     //estado identifica acción
   switch (c) {
      case '?': case '+': case '-':
         accion = c;
         estado = 999;          //Estado accion
         break;
   default:
      Serial.println ("-- Error en acción --");
      estado = 000;           // nos lleva al estado inicial
      }
   break;
// bloque PWM
   case 030:      //estado identifica el canal
   switch (c) {
      case '0': case '1': case '2': case '3': case '4':
         canal = (int) c - (int)'0';
         estado = 031;          //canal determinado
         break;
      case '?':
         canal = 0;
         accion = '?';
         estado = 999;          //Estado accion
         break;
   default:
      Serial.println ("-- Error en canal --");
      estado = 000;           // nos lleva al estado inicial
      }
   break;
    case 031:     //estado identifica acción 
   switch (c) {
      case '?': case '+': case '-':
         accion = c;
         estado = 999;          //Estado accion
         break;
      case 'r': case 'R':
         accion = 'R' ;
         estado = 032;                 
         break;      
   default:
      Serial.println ("-- Error en acción --");
      estado = 000;           // nos lleva al estado inicial
      }    
   break;
  case 032:      //estado inicio de comando primer dígito
   switch (c)   {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
         ratio = (int) c - (int)'0';
         estado = 033;                 
         break;
      default:
      Serial.println ("-- Error en ratio --");
      estado = 000;           // nos lleva al estado inicial
      }
    break;
  case 033:     //estado inicio de comando segundo dígito
   switch (c)   {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
         ratio =  ratio *10 + ((int) c - (int)'0');
         estado = 999;                 
         break;
      case FINCMD1:
      case FINCMD2:
         estado = -1;         // señaliza que hay que ejecutar una acción            
         break;
      default:
      Serial.println ("-- Error en ratio --");
      estado = 000;           // nos lleva al estado inicial
      }
   break;

// bloque Temperatura
   case 070:     
      canal = 0;
      accion = '?'; 
      switch (c) 
      {
      case FINCMD1:
      case FINCMD2:
         estado = -1;         // señaliza que hay que ejecutar una acción    
         break;        
      case '?':
         estado = 999;          //Estado accion
         break;
      default:
         Serial.println ("-- Error en acción --");
         estado = 000;           // nos lleva al estado inicial  
      }
      break;
 case 999:  //esperando el terminador 
   switch (c)   {
      case FINCMD1:
      case FINCMD2:
         estado = -1;         // señaliza que hay que ejecutar una acción            
         break;
      default:
      Serial.println ("-- Terminador no esperado --");
      estado = 000;           // nos lleva al estado inicial  
      }
   break;
  default:
     estado = 000;            // nos lleva al estado inicial  
   break;
  }
}


void actua()
{

   sprintf(salida, "CMD: %c,  Acc: %c, C: %d, ratio: %d", comando, accion, canal, ratio);
   Serial.println(salida);

 switch (comando) {
  case 'A':
         actua_anl();
         break;
  case 'T':
         actua_dht();
         break;
  case 'P':
         actua_pwm( );
         break;
  case 'R':
         actua_rele( );
         break;
      }
 
 // inicialización para el siguiente comando
 comando = 'X';
 accion = 'X';
 ratio = 0;
}

// bucle infinito

void loop() 
{
  // put your main code here, to run repeatedly:
 delay(10);

if ( Serial.available() > 0 )
    {
    in_c= Serial.read();
 //   Serial.write(in_c);     //echo para poder ver lo que escribimos  (hay que configurar el terminal con eco local)
    protocolo( in_c );
    if ( estado == -1 )       // el estado -1 indica ejecución del comando decodificado
      {
      actua();
      estado = 000;
      }
    }
parpadea();
}
