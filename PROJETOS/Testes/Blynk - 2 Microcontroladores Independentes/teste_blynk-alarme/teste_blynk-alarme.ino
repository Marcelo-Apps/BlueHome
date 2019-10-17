

#define DEBUG

#ifdef DEBUG  
  #define BLYNK_PRINT Serial
#endif


// Includes
//#include <WiFi.h>
//#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Parâmewtros wifi
char ssid[] = "iMordor";
char pass[] = "MyMordor2019!";



// Chave de autenticação do Projeto no Blynk
//char auth[] = "g6W9NYbPd9pJR8eiZLU3JEkInTB_BXSu";      // Automação
char auth[] = "SmirD__F6Ne4BwRIgjGMHj5616mQ0or-";      // Alarme


#define LED_RED        12
#define LED_GREEN      13


#define BLYNK_LEDRG    V5




// the setup function runs once when you press reset or power the board
void setup() {
  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_GREEN,LOW);

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
BLYNK_WRITE(BLYNK_LEDRG) {
  int lido, valRed, valGreen;
  
  lido=param[0].asInt();
#ifdef DEBUG
  Serial.print("-- Lido: ");
  Serial.println(lido);
#endif

  if ((lido==1) || (lido==3)) {
    valRed=HIGH;
  } else {
    valRed=LOW;
  }
  
  if ((lido==2) || (lido==3)) {
    valGreen=HIGH;
  } else {
    valGreen=LOW;
  }

  digitalWrite(LED_RED,valRed);
  digitalWrite(LED_GREEN,valGreen);
}
