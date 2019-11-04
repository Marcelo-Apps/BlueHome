/*
   BLUE HOME - V1.0

   PROCESSADOR DE ALARME
   ESP-32 COM 30 PINOS

   Autores: Marcelo Costa / Felipe Maia / Faberson Perfeito / Newton Dore

   OBS: Ainterface é feita pelo Blynk. Para maiores informações consultar o PDF que acompanha o projeto
*/


// Se estiver definido não inicializa/processa o BLYNK
#define SEMBLYNK

// Se estiver definido não inicializa/processa o SERIAL
#define SEMSERIAL

// Se é depuração (gera conteudo na serial para análise)
#define DEBUG

#ifdef DEBUG
#define BLYNK_PRINT Serial
#endif

#include <WiFi.h>
#include <WiFiClient.h>
#ifndef SEMBLYNK
  #include <BlynkSimpleEsp32.h>
#endif
#include <SPI.h>
#include <Stepper.h> //INCLUSÃO DE BIBLIOTECA MOTORES

// Parâmetros da Wi-Fi
char ssid[] = "WIFI-BH";
char pass[] = "BlueHome.IoT.2019";

// Chave de autenticação do Projeto no Blynk
//char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme

#define PIN_SENSORPIR              36
#define PIN_ALARME                  2

#define PIN_SENSORJANELA           35
#define PIN_SENSORPORTA            34

#define PIN_CMDABRIRJANELA         4
#define PIN_CMDFECHARJANELA        16
#define PIN_CMDABRIRPORTA          17
#define PIN_CMDFECHARPORTA         5

#define PIN_MOTORJANELA_I1         27
#define PIN_MOTORJANELA_I2         14
#define PIN_MOTORJANELA_I3         12
#define PIN_MOTORJANELA_I4         13

#define PIN_MOTORPORTA_I1         32
#define PIN_MOTORPORTA_I2         33
#define PIN_MOTORPORTA_I3         25
#define PIN_MOTORPORTA_I4         26

// Pinos da comunicação entre Processadores
#ifdef SEMSERIAL
#define PIN_SERIAL_COMUNIC_RX      16
#define PIN_SERIAL_COMUNIC_TX      17

// Mensagens da Automacão para Alarme (RF)
#define ALARME_SEMACAO              0
#define ALARME_DESATIVA             1
#define ALARME_ATIVA                2
#define ALARME_ATIVASILENCIOSO      3 
#define ALARME_TOCAR                4
#define ALARME_PARATOCAR            5
#define ALARME_ABREPORTA            6
#define ALARME_FECHAPORTA           7
#define ALARME_ABREJANELA           8
#define ALARME_FECHAJANELA          9
#endif

// Pinos Virtuais e Definições do Blynk
#define BLYNK_LCD                  V0
#define BLYNK_JANELA              V11


// *VARIÁVEIS GLOBAIS*
char *_lcdLin1;
char *_lcdLin2;

int _passoJanelaAtual; //passo atual da porta
int _passoPortaAtual; //passo atual da porta

const int _passosPorVoltaJanela = 30; //NÚMERO DE PASSOS POR VOLTA DO MOTOR DA JANELA
const int _passosPorVoltaPorta = 30; //NÚMERO DE PASSOS POR VOLTA DO MOTOR DA PORTA

Stepper motorJanela(_passosPorVoltaJanela, PIN_MOTORJANELA_I1, PIN_MOTORJANELA_I3, PIN_MOTORJANELA_I2, PIN_MOTORJANELA_I4); //INICIALIZA O MOTOR JANELA
Stepper motorPorta(_passosPorVoltaPorta, PIN_MOTORPORTA_I1, PIN_MOTORPORTA_I3, PIN_MOTORPORTA_I2, PIN_MOTORPORTA_I4); //INICIALIZA O MOTOR PORTA

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  motorJanela.setSpeed(300); //VELOCIDADE DO MOTOR JANELA
  motorPorta.setSpeed(300); //VELOCIDADE DO MOTOR PORTA

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
  inicializaContexto();

#ifndef SEMBLYNK
  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);
#endif
}

void loop() {
#ifndef SEMBLYNK
    Blynk.run();
#endif

    cmdAbrirPorta();
    cmdFecharPorta();
    atuaNaPorta();
    verificaSensorPorta();

    cmdAbrirJanela();
    cmdFecharJanela();
    atuaNaJanela();
    verificaSensorJanela();
}


// Inicializa o Ambiente (variáveis)
void inicializaContexto (void) {
  _lcdLin1 = "";
  _lcdLin2 = "";
}

void cmdAbrirPorta(void) {
  if (digitalRead(PIN_CMDABRIRPORTA)) {
    _passoPortaAtual = 65;
  }
}

void cmdFecharPorta(void) {
  if (digitalRead(PIN_CMDFECHARPORTA)) {
    _passoPortaAtual = -1;
  }
}

void cmdAbrirJanela(void) {
  if (digitalRead(PIN_CMDABRIRJANELA)) {
    _passoJanelaAtual = 75;
  }
}

void cmdFecharJanela(void) {
  if (digitalRead(PIN_CMDFECHARJANELA)) {
    _passoJanelaAtual = -1;
  }
}

// Lê o sensor da janela se ja fechou ou abriu totalmente
void verificaSensorJanela (void) {
  if (digitalRead(PIN_SENSORJANELA)) { //se detectou que a janela ja fechou ou ja abriu totalmente
    _passoJanelaAtual = 0; //para a janela
  }
}

// Lê o sensor da porta se ja fechou ou abriu totalmente
void verificaSensorPorta (void) {
  if (digitalRead(PIN_SENSORPORTA)) { //se detectou que a porta ja fechou ou ja abriu totalmente
    _passoPortaAtual = 0; //para a porta
  }
}

void atuaNaPorta (void) {

  if (_passoPortaAtual > 0) { //porta abrindo
    motorPorta.step(_passosPorVoltaPorta);
    _passoPortaAtual--;
    // quando a variavel _passoPortaAtual chegar em 0, para a porta
    
  } else if (_passoPortaAtual < 0 && _passoPortaAtual >= -65) { //porta fechando
    motorPorta.step(-_passosPorVoltaPorta);
    _passoPortaAtual--;
  }// quando a variavel _passoPortaAtual chegar em -31, para a porta
}

void atuaNaJanela(void) {

  if (_passoJanelaAtual > 0) { //janela abrindo
    motorJanela.step(_passosPorVoltaJanela);
    _passoJanelaAtual--;
    // quando a variavel _passoJanelaAtual chegar em 0, para a janela

  } else if (_passoJanelaAtual < 0 && _passoJanelaAtual >= -75) { //janela fechando
    motorJanela.step(-_passosPorVoltaJanela);
    _passoJanelaAtual--;
  }// quando a variavel _passoJanelaAtual chegar em -31, para a janela
}

#ifndef SEMBLYNK
void printLCD (char texto[]) {
  WidgetLCD lcd(V0);
  _lcdLin1=_lcdLin2;
  _lcdLin2=texto;
  lcd.clear();
  lcd.print(0,0,_lcdLin1);
  lcd.print(0,1,_lcdLin2);
}

//Redefine o ambiente quando conectado.
BLYNK_CONNECTED () {
  // Manda o texto para o LCD
  printLCD("BH-ALARME OK");
//  printLCD("");
}
#endif
