/*
 * BLUE HOME - V1.0
 * 
 * PROCESSADOR DE AUTOMAÇÃO
 * ESP-32 COM 38 PINOS
 * 
 * Autores: Marcelo Costa / Felipe Maia / Faberson Perfeito / Newton Dore
 * 
 * OBS: Ainterface é feita pelo Blynk. Para maiores informações consultar o PDF que acompanha o projeto
 */


// Se é depuração (gera conteudo na serial para análise)
#define DEBUG


#ifdef DEBUG  
  #define BLYNK_PRINT Serial
#endif


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


// Parâmetros da Wi-Fi
char ssid[] = "WIFI-BH";
char pass[] = "BlueHome.IoT.2019";


// Chave de autenticação do Projeto no Blynk
char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
//char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme

// Pinos usados para outros fins
// D16 -- U2_RX
// D17 -- U2_TX
// D05 -- Aviso Alarme--Automacao
// D04 - RESERVADO ENQUANTO FAZ COMUNICACAO SERIAL COM PC
//



#define PIN_SENSORLUZ              04
#define PIN_SENSORCHUVA            T2     // D02
#define PIN_SENSORJANELA           22
#define PIN_SENSORPORTA            23
#define PIN_LEDRED                 18
#define PIN_LEDGREEN               19
#define PIN_LEDBLUE                21

#define PIN_AVISODOALARME          34

#define PWM_LEDRED                 00
#define PWM_LEDGREEN               01
#define PWM_LEDBLUE                02

#define BLYNK_LCD                  V0
#define BLYNK_LUZEXTERNA          V13
#define BLYNK_LUZINTERNA          V10





#define PWM_FREQUENCIA           1000
#define PWM_BITSRESOLUCAO           8
#define PWW_LIMITE                256
#define PWM_MIN                     0
#define PWM_MAX                   255

#define MOTOR_PASSOS-PORTA         20
#define MOTOR_PASSOS-JANELA        25


// *VARIÁVEIS GLOBAIS*
char *_lcdLin1;
char *_lcdLin2;






void setup() {

  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // INICIALIZAÇÃO DO AMBIENTE
  pinMode(PIN_LEDRED, OUTPUT);
  pinMode(PIN_LEDGREEN, OUTPUT);
  pinMode(PIN_LEDBLUE, OUTPUT);
  ledcAttachPin(PIN_LEDRED,PWM_LEDRED);
  ledcAttachPin(PIN_LEDGREEN,PWM_LEDGREEN);
  ledcAttachPin(PIN_LEDBLUE,PWM_LEDBLUE);
  ledcSetup(PWM_LEDRED,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_LEDGREEN,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_LEDBLUE,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);

  ledcWrite(PWM_LEDRED,PWM_MIN);
  ledcWrite(PWM_LEDGREEN,PWM_MIN);
  ledcWrite(PWM_LEDBLUE,PWM_MIN);

  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);

}



void loop() {
  Blynk.run();
}



void printLCD (char texto[]) {
  WidgetLCD lcd(V0);

  _lcdLin1=_lcdLin2;
  _lcdLin2=texto;
  lcd.clear();
  lcd.print(0,0,_lcdLin1);
  lcd.print(0,1,_lcdLin2);
}



// Redefine o ambiente quando conectado.
BLYNK_CONNECTED () {
  // Manda o texto
  printLCD("BLUEHOME - UTFPR");
  printLCD("ATIVO");
}



// MUDANÇA DE COR DA LUZ INTERNA
BLYNK_WRITE(BLYNK_LUZINTERNA) {
  int red, green, blue;

  // Lê parâmetros dos Blynk
  red=param[0].asInt();
  green=param[1].asInt();
  blue=param[2].asInt();

#ifdef DEBUG
  Serial.print("--LUZ INTERNA - Red: ");
  Serial.println(red);
  Serial.print(",  Green: ");
  Serial.println(green);
  Serial.print(",  Blue: ");
  Serial.println(blue);
#endif
  
  ledcWrite(PWM_LEDRED,red);
  ledcWrite(PWM_LEDGREEN,green);
  ledcWrite(PWM_LEDBLUE,blue);
}
