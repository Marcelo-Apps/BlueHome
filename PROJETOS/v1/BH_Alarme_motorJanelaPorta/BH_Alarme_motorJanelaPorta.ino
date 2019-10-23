/*
   BLUE HOME - V1.0

   PROCESSADOR DE ALARME
   ESP-32 COM 30 PINOS

   Autores: Marcelo Costa / Felipe Maia / Faberson Perfeito / Newton Dore

   OBS: Ainterface é feita pelo Blynk. Para maiores informações consultar o PDF que acompanha o projeto
*/


// Se é depuração (gera conteudo na serial para análise)
#define DEBUG

#ifdef DEBUG
#define BLYNK_PRINT Serial
#endif

#include <WiFi.h>
#include <WiFiClient.h>
//#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <Stepper.h> //INCLUSÃO DE BIBLIOTECA MOTORES

// Parâmetros da Wi-Fi
char ssid[] = "WIFI-BH";
char pass[] = "BlueHome.IoT.2019";

// Chave de autenticação do Projeto no Blynk
//char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme

#define PIN_SENSORPIR              36
#define PIN_ALARME                  4

#define PIN_SENSORJANELA           35
#define PIN_SENSORPORTA            34

#define PIN_CMDABRIRJANELA         26
#define PIN_CMDFECHARJANELA        25
#define PIN_CMDABRIRPORTA          33
#define PIN_CMDFECHARPORTA         32

#define PIN_MOTORJANELA_I1         27
#define PIN_MOTORJANELA_I2         14
#define PIN_MOTORJANELA_I3         12
#define PIN_MOTORJANELA_I4         13

#define PIN_MOTORPORTA_I1         2
#define PIN_MOTORPORTA_I2         18
#define PIN_MOTORPORTA_I3         19
#define PIN_MOTORPORTA_I4         23

#define BLYNK_LCD                  V0
#define BLYNK_JANELA              V11

// *VARIÁVEIS GLOBAIS*
char *_lcdLin1;
char *_lcdLin2;

bool _portaEmMovimento;
bool _janelaEmMovimento;

int _passoJanelaAtual; //passo atual da porta
int _passoPortaAtual; //passo atual da porta

const int _passosPorVoltaJanela = 30; //NÚMERO DE PASSOS POR VOLTA DO MOTOR DA JANELA
const int _passosPorVoltaPorta = 30; //NÚMERO DE PASSOS POR VOLTA DO MOTOR DA PORTA

Stepper motorJanela(_passosPorVoltaJanela, PIN_MOTORJANELA_I1, PIN_MOTORJANELA_I3, PIN_MOTORJANELA_I2, PIN_MOTORJANELA_I4); //INICIALIZA O MOTOR JANELA
Stepper motorPorta(_passosPorVoltaPorta, PIN_MOTORPORTA_I1, PIN_MOTORPORTA_I3, PIN_MOTORPORTA_I2, PIN_MOTORPORTA_I4); //INICIALIZA O MOTOR PORTA

void setup() {
  motorJanela.setSpeed(300); //VELOCIDADE DO MOTOR JANELA
  motorPorta.setSpeed(300); //VELOCIDADE DO MOTOR PORTA

  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // INICIALIZAÇÃO DO AMBIENTE
  pinMode(PIN_SENSORPIR, INPUT);
  pinMode(PIN_ALARME, OUTPUT);

  pinMode(PIN_SENSORJANELA, INPUT);
  pinMode(PIN_SENSORPORTA, INPUT);

  pinMode(PIN_CMDABRIRJANELA, INPUT);
  pinMode(PIN_CMDFECHARJANELA, INPUT);
  pinMode(PIN_CMDABRIRPORTA, INPUT);
  pinMode(PIN_CMDFECHARPORTA, INPUT);

  // Inicializa os parâmetros e ajustes iniciais
  inicializaAjustes();

  // Inicializa o Blynk
  //Blynk.begin(auth, ssid, pass);
}

void loop() {
  //  Blynk.run();

  if (!_portaEmMovimento) {
    cmdAbrirPorta();
    cmdFecharPorta();
  } else {
    atuaNaPorta();
    verificaSensorPorta();
  }

  if (!_janelaEmMovimento) {
    cmdAbrirJanela();
    cmdFecharJanela();
  } else {
    atuaNaJanela();
    verificaSensorJanela();
  }
}

void inicializaAjustes (void) {

  _portaEmMovimento = false;
  _janelaEmMovimento = false;

  _lcdLin1 = "";
  _lcdLin2 = "";
}

void cmdAbrirPorta(void) {
  if (digitalRead(PIN_CMDABRIRPORTA)) {
    _passoPortaAtual = 30;
    _portaEmMovimento = true;
  }
}

void cmdFecharPorta(void) {
  if (digitalRead(PIN_CMDFECHARPORTA)) {
    _passoPortaAtual = -1;
    _portaEmMovimento = true;
  }
}

void cmdAbrirJanela(void) {
  if (digitalRead(PIN_CMDABRIRJANELA)) {
    _passoJanelaAtual = 30;
    _janelaEmMovimento = true;
  }
}

void cmdFecharJanela(void) {
  if (digitalRead(PIN_CMDFECHARJANELA)) {
    _passoJanelaAtual = -1;
    _janelaEmMovimento = true;
  }
}

// Lê o sensor da janela se ja fechou ou abriu totalmente
void verificaSensorJanela (void) {
  if (digitalRead(PIN_SENSORJANELA)) { //se detectou que a janela ja fechou ou ja abriu totalmente
    if (_passoJanelaAtual > 0) { //estava em processo de abrir e ja detectou que abriu totalmente
      _passoJanelaAtual = 0;

    } else if (_passoJanelaAtual < 0 && _passoJanelaAtual > -30) { // estava em processo de fechar e ja detectou que fechou totalmente
      _passoJanelaAtual = -30;
    }
  }
}

// Lê o sensor da porta se ja fechou ou abriu totalmente
void verificaSensorPorta (void) {
  if (digitalRead(PIN_SENSORPORTA)) { //se detectou que a porta ja fechou ou ja abriu totalmente
    if (_passoPortaAtual > 0) { //estava em processo de abrir e ja detectou que abriu totalmente
      _passoPortaAtual = 0;

    } else if (_passoPortaAtual < 0 && _passoPortaAtual > -30) { // estava em processo de fechar e ja detectou que fechou totalmente
      _passoPortaAtual = -30;
    }
  }
}

void atuaNaPorta (void) {

  if (_passoPortaAtual > 0) { //porta abrindo
    motorPorta.step(_passosPorVoltaPorta);
    _passoPortaAtual--;

  } else if (_passoPortaAtual == 0) {
    _portaEmMovimento = false; //porta permanece aberta

  } else if (_passoPortaAtual <= 0 && _passoPortaAtual > -30) { //porta fechando
    motorPorta.step(-_passosPorVoltaPorta);
    _passoPortaAtual--;
  } else {
    _portaEmMovimento = false; // porta permanece fechada
  }
}

void atuaNaJanela(void) {

  if (_passoJanelaAtual > 0) { //janela abrindo
    motorJanela.step(_passosPorVoltaJanela);
    _passoJanelaAtual--;

  } else if (_passoJanelaAtual == 0) {
    _janelaEmMovimento = false; //janela permanece aberta

  } else if (_passoJanelaAtual <= 0 && _passoJanelaAtual > -30) { //janela fechando
    motorJanela.step(-_passosPorVoltaJanela);
    _passoJanelaAtual--;
  } else {
    _janelaEmMovimento = false; // janela permanece fechada
  }
}

//void printLCD (char texto[]) {
//  WidgetLCD lcd(V0);

//  _lcdLin1=_lcdLin2;
//  _lcdLin2=texto;
//  lcd.clear();
//  lcd.print(0,0,_lcdLin1);
//  lcd.print(0,1,_lcdLin2);
//}

// Redefine o ambiente quando conectado.
//BLYNK_CONNECTED () {
// Manda o texto
// printLCD("BLUEHOME - UTFPR");
// printLCD("ATIVO");
//}
