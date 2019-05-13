#include<LiquidCrystal.h>
#include<Keypad.h>
//declaro los pines a los que voy a conectar cada elemento
const int EchoPin = 10;
const int TriggerPin = 9;
const int PIN_CALDERA=27;
#define LedPin 26
#define led_interior 28
#define pote A0
#define IN3 22
#define IN4 23
#define IN1 24
#define IN2 25
#define pulsador 2
#define pulsador_persiana_bajar 51
#define pulsador_persiana_subir 48
#define pulsador_luz 50
#define zumbador 8
#define ldr A1
#define DEBUG(a) 

//declaro las variables que voy a utilizar en el código
int action = 0;
int blinkLed = 0;
int ledLevel = LOW;
//const int PIN_CALDERA = 2;
int dimension=0;
const float SCALE_FACTOR = 0.05;
float userTemperature = 0.0;
long duration;
int distance, initialDistance, currentDistance, i;
int screenOffMsg = 0;
String password = "1234";
String tempPassword;
boolean activated = false; // estado de la alarma
boolean isActivated;
boolean activateAlarm = false;
boolean alarmActivated = false;
boolean enteredPassword; // estado de la contraseña para parar la alarma
boolean passChangeMode = false;
boolean passChanged = false;
const byte ROWS = 4; //4 filas
const byte COLS = 4; //4 columnas
char keypressed;
//defino los simbolos de las teclas del teclado
char keyMap[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = { 14, 15, 16, 17 }; //pines de las filas
byte colPins[COLS] = { 18, 19, 20, 21 }; //pines de las columnas
Keypad myKeypad = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(11, 12, 4, 5, 6, 7); // crea un objeto LCD. Parámetros: (rs, enable, d4, d5, d6, d7). Indico donde conecto cada elemento del pin

void setup() {

  Serial.begin(9600);
  lcd.begin(16, 2);//lcd dividido en una matriz 16x2
  Serial.setTimeout(50);
  pinMode(pulsador_persiana_bajar, INPUT);
  pinMode(pulsador_persiana_subir, INPUT);
  pinMode(pulsador_luz, INPUT);
  pinMode(led_interior, OUTPUT);
  pinMode(LedPin, OUTPUT);
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  pinMode(IN4, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(pote, INPUT);
  pinMode(pulsador, INPUT);
  pinMode(zumbador, OUTPUT);
  pinMode(ldr, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_CALDERA, OUTPUT);
  int distancia(int, int); //prototipos funciones
  void pausa(unsigned int);
  void termostato(int);
  
}

void loop() {

  float temperature;
  char *ppas;
  ppas=&password[0];
  //termostato(analogRead(pote));
  luz(digitalRead(ldr));
  persiana_subir(digitalRead(pulsador_persiana_subir));
  persiana_bajar(digitalRead(pulsador_persiana_bajar));
  //alarma(digitalRead(pulsador));
  timbre(digitalRead(pulsador));
  temperature = control_temperature(userTemperature);
  action = Serial.read();
  conexionpc(action, temperature);
  luz_interior(digitalRead(pulsador_luz));



//Código Alarma
  if (activateAlarm) {//si se selecciona la opción de activar la alarma
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("La alarma se");
    lcd.setCursor(0, 1);
    lcd.print("activara en");

    int countdown = 9; // 9 segundos de cuenta atrás antes de activar la alarma
    while (countdown != 0) {
      lcd.setCursor(13, 1);
      lcd.print(countdown);
      countdown--;
      tone(zumbador, 700, 100);
      delay(1000);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarma Activada");
    initialDistance = distancia(TriggerPin, EchoPin);//toma la distacia inicial medida por el ultrasonidos
    activateAlarm = false;
    alarmActivated = true;
  }
  if (alarmActivated == true) {//si la alarma esta activada 
    currentDistance = distancia(TriggerPin, EchoPin) + 10;//se comprueba la distancia actual con la inicial calculada antes
    if (currentDistance < initialDistance) {//si la distancia es menor de la inicial
      tone(zumbador, 1000); // Se activa el zumbador 
      lcd.clear();
      enterPassword();//se llama a la funcion para introducir la contraseña
    }
  }
  if (!alarmActivated) {//si la alarma no esta activada
    if (screenOffMsg == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("A - Activar");
      lcd.setCursor(0, 1);
      lcd.print("B - Cambiar contraseña");
      screenOffMsg = 1;
    }
    keypressed = myKeypad.getKey();
    if (keypressed == 'A') {        //Si se elige A se activa la alarma
      tone(zumbador, 1000, 200);
      activateAlarm = true;
    }
    else if (keypressed == 'B') { //si se elige la B se pide primero la contraseña actual
      lcd.clear();
      int i = 1;
      tone(zumbador, 2000, 100);
      tempPassword = "";
      lcd.setCursor(0, 0);
      lcd.print("Contrasena:");
      lcd.setCursor(0, 1);
      lcd.print(">");
      passChangeMode = true;
      passChanged = true;
      while (passChanged) {
        keypressed = myKeypad.getKey();
        if (keypressed != NO_KEY) {
          if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
            keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
            keypressed == '8' || keypressed == '9') {
            tempPassword += keypressed;
            lcd.setCursor(i, 1);
            lcd.print("*");
            i++;
            tone(zumbador, 2000, 100);
          }
        }
        if (i > 5 || keypressed == '#') {//Si excedemos el numero de digitos o pulsamos # nos la vuelve a pedir
          tempPassword = "";
          i = 1;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Contrasena:");
          lcd.setCursor(0, 1);
          lcd.print(">");
        }
        if (keypressed == '*') {
          i = 1;
          tone(zumbador, 2000, 100);
          if (password == tempPassword) {//si la contraseña es correcta se pide la nueva contraseña
            tempPassword = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            //------------------------------------
            lcd.print("Digitos:"); //nos pide el numero de digitos que queremos en la nueva contraseña
             lcd.setCursor(0, 1);
              lcd.print(">");
               while (passChangeMode) {
              keypressed = myKeypad.getKey();
              if (keypressed != NO_KEY) {
                if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
                  keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
                  keypressed == '8' || keypressed == '9') {
                  dimension+= keypressed;
                  lcd.setCursor(i, 1);
                  lcd.print("*");
                  i++;
                  tone(zumbador, 2000, 100);
                }
            lcd.clear();
            ppas=(char*)realloc(ppas,dimension*sizeof(char) );
            //----------------------------------
            lcd.print("Nueva contrasena:");
            lcd.setCursor(0, 1);
            lcd.print(">");
            while (passChangeMode) {
              keypressed = myKeypad.getKey();
              if (keypressed != NO_KEY) {
                if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
                  keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
                  keypressed == '8' || keypressed == '9') {
                  tempPassword += keypressed;
                  lcd.setCursor(i, 1);
                  lcd.print("*");
                  i++;
                  tone(zumbador, 2000, 100);
                }
              }
              if (i > dimension+1 || keypressed == '#') {//se vuelve a pedir la nueva contraseña
                tempPassword = "";
                i = 1;
                tone(zumbador, 2000, 100);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Nueva contrasena:");
                lcd.setCursor(0, 1);
                lcd.print(">");
              }
              if (keypressed == '*') {//se guarda la nueva contraseña
                i = 1;
                tone(zumbador, 2000, 100);
                password = tempPassword;
                passChangeMode = false;
                passChanged = false;
                screenOffMsg = 0;
              }
            }
          }
        }
      }
    }
  }
}
}
}
//--------------------------------------------------------------
void enterPassword() { //funcion para introducir la contraseña por teclado
  int k = 5;
  tempPassword = "";
  activated = true;
  lcd.clear();//limpiamos pantalla
  lcd.setCursor(0, 0);//cursor en la parte superior izq de la pantalla
  lcd.print(" *** ALARMA *** ");
  lcd.setCursor(0, 1);//cursor en la parte inferior izq de la pantalla
  lcd.print("Cont>");
  while (activated) {
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY) {
      if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
        keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
        keypressed == '8' || keypressed == '9') {
        tempPassword += keypressed;
        lcd.setCursor(k, 1);
        lcd.print("*");
        k++;
      }
    }
    if (k > dimension+1 || keypressed == '#') { //si la contraseña supera los digitos o se presiona # se vuelve a pedir la contraseña
      tempPassword = "";
      k = dimension;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" *** ALARMA *** ");
      lcd.setCursor(0, 1);
      lcd.print("Cont>");
    }
    if (keypressed == '*') { //si se presiona * entonces se comprueba si la contraseña introducida es correcta
      if (tempPassword == password) {
        activated = false;
        alarmActivated = false;
        noTone(zumbador);
        screenOffMsg = 0;
      }
      else if (tempPassword != password) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Error");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" *** ALARMA *** ");
        lcd.setCursor(0, 1);
        lcd.print("Cont>");
      }
    }
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------------
int distancia(int TriggerPin, int EchoPin) //funcion distancia del ultrasonidos
{
  long duration, distance;
  digitalWrite(TriggerPin, LOW);
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);

  duration = pulseIn(EchoPin, HIGH); //calculo el tiempo
  distance = duration * 10 / 292 / 2;  // calculo la distancia en cm
  return distance;
}
//------------------------------------------------------------------------------------------------------------------------------------
void termostato(int variable)//
{
  float opt;
  variable *= 0.05;//factor de conversion para cambiar la lectura del pontenciometro a grados centigrados
  //Serial.println(variable);
  if (Serial.available())
  {
    opt = Serial.parseFloat();// esto no funciona, en el serial.println se ve que coge el numero que le metes y luego un 0
    //no se porque, entonces siempre toma el 0 como referencia y siempre funciona el motor pongas lo que pongas
    Serial.println(opt);
    //scanf("%c",&opt);
    //getchar();
  }
  if (variable > opt)
  {
    digitalWrite(IN4, HIGH);//Se acciona el motor
    digitalWrite(IN3, LOW);
  }
  else
  {
    digitalWrite(IN4, LOW);
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
void luz(int valor)//enciende la luz en funcion de la lectura del ldr
{
  //Serial.println(valor);
  if (valor == HIGH)
  {
    digitalWrite(LedPin, LOW);
  }
  else
  {
    digitalWrite(LedPin, HIGH);

  }
}
//---------------------------------------------------------------------------------------------------------------------------------
void luz_interior(int valor)//función que enciende la uz en función de la lectura del pulsador
{
  if(valor==LOW)
  {
    digitalWrite(led_interior, HIGH);
  }
  else
  {
    digitalWrite(led_interior, LOW);
  }
}
//----------------------------------------------------------------------------------------------------------------------------------------
void persiana_subir(int valorPulsador)//funcion que sube la persiana si el valor del pulsador es 1
{
  //Serial.println(valorPulsador);//Mirar los pines segun sentido del motor
  if (valorPulsador == LOW)
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else
  {
    digitalWrite(IN4, LOW);
  }
}
//----------------------------------------------------------------------------------------------------------------------------------------------
void persiana_bajar(int valorPulsador)//función que baja la ersiana si el valor del pulsador es 1
{
  if(valorPulsador==LOW)
  {
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4, LOW);
  }
  else
  {
    digitalWrite(IN3, LOW);
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------
void alarma(int valorPulsador)//
{
  int cm = distancia(TriggerPin, EchoPin);
  if (valorPulsador == LOW)
  {
    if (cm <= 19)
    {
      analogWrite(zumbador, 300);
      pausa(5000);
      analogWrite(zumbador, LOW);
    }
  }
  else
  {
    analogWrite(zumbador, LOW);
  }
}
//-------------------------------------------------------------------------------------------------------------------------------------------------
void timbre(int valorPulsador)//función que activa el timbre al pulsar el pulsador
{
  if (valorPulsador == HIGH)
  {
    analogWrite(zumbador, 884);
  }
  else
  {
    digitalWrite(zumbador, LOW);
  }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void pausa(unsigned int milisegundos)//funcion para crear una pausa sin usar el delay
{
  volatile unsigned long compara = 0;
  volatile int contador = 0;
  do
  {
    if (compara != millis())
    {
      contador++;
      compara = millis();
    }
  } while (contador <= milisegundos);
  return;
}
//----------------------------------------------------------------------------------------
// control_temperature
// Lee el valor del potenciometro que simula el sensor de temperatura del pin A0.
// Lo convierte a grados centrigrados y compara ese valor con el valor deseado de
// temperatura indicado por el usuario. Si la temperatura es menor pone a 1 el pin
// donde esta conectado el arranque de la calefacción, sino lo pone a 0
//
// Necesita:
//  - userGradTemp: Valor de temperatura deseada por el usuario
//
// Retorna: el valor de temperatura leído del potenciometro
//----------------------------------------------------------------------------------------
float control_temperature(float userGradTemp)
{
  int rawTemp = 0;      // Valor del potenciometro
  float gradTemp = 0;   // Temperatura en grados


  // Se supone que el potenciometro que simula el sensor de temperatura
  // está conectado al pin A0

  // Se lee el valor del potenciometro
  rawTemp = analogRead(A0);

  // Se transforma el valor leído a grados usando el factor de escala
  // indicado
  gradTemp = rawTemp * SCALE_FACTOR;

  // Si la temperutura es menor que la deseada, se enciende la caldera
  if (gradTemp < userGradTemp)
    digitalWrite(PIN_CALDERA, HIGH);
  else
    digitalWrite(PIN_CALDERA, LOW);

  // Se devuelve el valor
  return gradTemp;
}
//---------------------------------------------------------------------------------------------------------
void conexionpc(int action, float temperature)//funcion que conecta el puerto serie al arduino
{
  switch (action)
  {
  case '1': {
    // Se indica que el led debe encenderse y no parpadear
    ledLevel = HIGH;
    blinkLed = 0;
    break;
  }

  case '2': {
    // Se indica que el led debe apagarse y no parpadear
    ledLevel = LOW;
    blinkLed = 0;
    break;
  }

  case '3': {
    // Se indica que el led debe parpadear
    blinkLed = 1;
    break;
  }

  case '4': {
    // Se envia el valor de temperatura
    Serial.print(temperature);
    break;
  }

  case '5': {
    // Se lee el valor deseado para la temperatura transformandolo en un float
    userTemperature = Serial.parseFloat();

    break;
  }
  }
  if (blinkLed)
  {
    // Se invierte la indicación para el led
    if (ledLevel == HIGH)
      ledLevel = LOW;
    else
      ledLevel = HIGH;

    // Se esperan 500 ms
    delay(500);
  }

  // Se escribe el valor deseado en el led  
  digitalWrite(LED_BUILTIN, ledLevel);
}
