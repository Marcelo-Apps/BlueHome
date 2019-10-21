

#define DEBUG

#ifdef DEBUG  
  #define BLYNK_PRINT Serial
#endif


// Includes
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Parâmewtros wifi
char ssid[] = "WIFI-BH";
char pass[] = "BlueHome.IoT.2019";

// Chave de autenticação do Projeto no Blynk
char auth[] = "g6W9NYbPd9pJR8eiZLU3JEkInTB_BXSu";      // Automação
//char auth[] = "SmirD__F6Ne4BwRIgjGMHj5616mQ0or-";      // Alarme


#define LED_RED 2
#define LED_GREEN 4
#define LED_BLUE 16

#define PWM_RED 0
#define PWM_GREEN 1
#define PWM_BLUE 2

#define PWM_FREQUENCIA 1000
#define PWM_BITSRESOLUCAO 8
#define PWW_LIMITE      256

#define PWM_MAXIMO      255

#define BLYNK_LEDRGB    V10




// the setup function runs once when you press reset or power the board
void setup() {
  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  ledcAttachPin(LED_RED,PWM_RED);
  ledcAttachPin(LED_GREEN,PWM_GREEN);
  ledcAttachPin(LED_BLUE,PWM_BLUE);
  ledcSetup(PWM_RED,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_GREEN,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_BLUE,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);

  ledcWrite(PWM_RED,PWM_MAXIMO);
  ledcWrite(PWM_GREEN,PWM_MAXIMO);
  ledcWrite(PWM_BLUE,PWM_MAXIMO);
  
  Blynk.begin(auth, ssid, pass);
}



// the loop function runs over and over again forever
void loop() {
  Blynk.run();
  

}




/*
BLYNK_READ(V11) {
  //
}
*/


// Redefine o ambiente quando conectado.
BLYNK_CONNECTED () {
//
//
}


// Processa boão "Moton on/off"
BLYNK_WRITE(BLYNK_LEDRGB) {
  int red, green, blue;

  // Lê parâmetros dos Blynk
  red=param[0].asInt();
  green=param[1].asInt();
  blue=param[2].asInt();

#ifdef DEBUG
  Serial.print("-- Red: ");
  Serial.println(red);
  Serial.print(",  Green: ");
  Serial.println(green);
  Serial.print(",  Blue: ");
  Serial.println(blue);
#endif
  
  ledcWrite(PWM_RED,red);
  ledcWrite(PWM_GREEN,green);
  ledcWrite(PWM_BLUE,blue);
}
