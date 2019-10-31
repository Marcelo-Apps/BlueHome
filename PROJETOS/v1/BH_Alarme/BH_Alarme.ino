/*
 * BLUE HOME - V1.0
 * 
 * PROCESSADOR DE AUTOMAÇÃO
 * ESP-32 COM 30 PINOS
 * 
 * Autores: Marcelo Costa / Felipe Maia / Faberson Perfeito / Newton Dore
 * 
 * NOTA: ESTE PROCESSADOR PRECISA INICIALIZAR DEPOIS DO DE ALARME, POIS ELE ENVIA CÓDIGOS
 *       DE INICIALIZAÇÃO PARA O OUTRO.
 * 
 * OBS: Ainterface é feita pelo Blynk. Para maiores informações consultar o PDF que acompanha o projeto
 */


// Se estiver definido não inicializa/processa o BLYNK
//#define SEMBLYNK


// Se é depuração (gera conteudo na serial para análise)
#define DEBUG


#ifdef DEBUG  
  #define BLYNK_PRINT Serial
#endif


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Stepper.h>
#include <HardwareSerial.h>


// Parâmetros da Wi-Fi
char ssid[] = "WIFI-BH";
char pass[] = "BlueHome.IoT.2019";


// Chave de autenticação do Projeto no Blynk
//char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme

// Pinos usados para outros fins
// D16 -- U2_RX
// D17 -- U2_TX
// D04 - RESERVADO ENQUANTO FAZ COMUNICACAO SERIAL COM PC
//



#define PIN_SENSORPIR              36
#define PIN_SENSORJANELA           35
#define PIN_SENSORPORTA            34
#define PIN_RELEALARME              4

#define PIN_MOTORJANELA_I1         27
#define PIN_MOTORJANELA_I2         14
#define PIN_MOTORJANELA_I3         12
#define PIN_MOTORJANELA_I4         13

#define PIN_MOTORPORTA_I1          27
#define PIN_MOTORPORTA_I2          14
#define PIN_MOTORPORTA_I3          12
#define PIN_MOTORPORTA_I4          13

// Pinos da comunicação entre Processadores
#define PIN_SERIAL_COMUNIC_RX      16
#define PIN_SERIAL_COMUNIC_TX      17
#define SERIALRF_VELOCIDADE       900      // Velocidade da Serial de RF (entre Microcontroladores)

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


// Pinos Virtuais e Definições do Blynk
#define BLYNK_LCD                  V0
#define BLYNK_ALARMETOCANDO        V6
#define BLYNK_ALARMECONFIG         V5
#define BLYNK_JANELADIRECAO       V11
#define BLYNK_PORTADIRECAO        V12

#define MOTORJANELA_VELOCIDADE    400
#define MOTORJANELA_PASSOSVOLTA   200
#define MOTORJANELA_PASSOSVEZ      10
#define MOTORJANELA_POSFECHADA     50
#define MOTORJANELA_POSABERTA       0

#define MOTORPORTA_VELOCIDADE     400
#define MOTORPORTA_PASSOSVOLTA    200
#define MOTORPORTA_PASSOSVEZ       10
#define MOTORPORTA_POSFECHADA      50
#define MOTORPORTA_POSABERTA        0

#define ALARME_TEMPOESPERA      10000             // Espera 10s para tocar de novo se estiver armado...

// ** VARIÁVEIS GLOBAIS **

String _lcdLin1, _lcdLin2;

int _janelaPosicao, _atualJanelaDirecao, _novoJanelaDirecao, _portaPosicao, _atualPortaDirecao, _novoPortaDirecao;
int _atualAlarmeTipo, _novoAlarmeTipo;
bool _atualAlarmeTocando, _novoAlarmeTocando;
 
unsigned long _momentoDesarmouAlarme;

// Inicializa a serial para comunicação RF entre Automação e Alarme
HardwareSerial TxSerial(1);

// ** CLASSES **

// Inicializa os Motores de Passo
Stepper motorJanela(MOTORJANELA_PASSOSVOLTA, PIN_MOTORJANELA_I1, PIN_MOTORJANELA_I3, PIN_MOTORJANELA_I2, PIN_MOTORJANELA_I4); //INICIALIZA O MOTOR JANELA
Stepper motorPorta(MOTORPORTA_PASSOSVOLTA, PIN_MOTORPORTA_I1, PIN_MOTORPORTA_I3, PIN_MOTORPORTA_I2, PIN_MOTORPORTA_I4);




void setup() {

  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // INICIALIZAÇÃO DO AMBIENTE
  pinMode(PIN_SENSORPIR, INPUT);
  pinMode(PIN_SENSORJANELA, INPUT);
  pinMode(PIN_SENSORPORTA, INPUT);
  
  pinMode(PIN_RELEALARME, OUTPUT);



#ifdef DEBUG
  Serial.println("MÓDULO BLUEHOME - ALARME");
  Serial.println("========================");
  Serial.println("");
#endif

  // Inicializa os parâmetros e ajustes iniciais
  inicializaContexto();
  
  // Inicializa a Serial entre Automação e Alarme
  TxSerial.begin(SERIALRF_VELOCIDADE,SERIAL_6E2,16,17);   // (BR,Prot,RX,TX);
#ifdef DEBUG
  Serial.println("Serial entre Automação e Alarme Inicializada (via Sinal de RF)");
  Serial.println("velocidade (bps): "+String(SERIALRF_VELOCIDADE));
#endif  
  // Inicializa os parâmetros e ajustes iniciais
  inicializaContexto();

#ifndef SEMBLYNK
  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);
#endif


#ifdef DEBUG
  Serial.println("** OPERAÇÃO DO PROCESSADOR DE ALARME INICIADA**");
#endif

}






void loop() {
#ifndef SEMBLYNK
    Blynk.run();
#endif

  processaMsgAutomacao();
  verificaAlarmeTocando();
  atuaAlarmeTocando();
  verificaJanela();
  atuaJanela();
  verificaPorta();
  atuaPorta();

  delay(5);
}








void inicializaContexto (void) {
  // Pinos
  digitalWrite(PIN_RELEALARME,HIGH);

  // Variáveis
  _janelaPosicao=MOTORJANELA_POSFECHADA;
  _atualJanelaDirecao=1;
  _novoJanelaDirecao=1;
  _portaPosicao=MOTORPORTA_POSFECHADA;
  _atualPortaDirecao=1;
  _novoPortaDirecao=1;
  _momentoDesarmouAlarme=0;
  _atualAlarmeTocando=false;
  _novoAlarmeTocando=false;
  _novoAlarmeTipo=0;
  _atualAlarmeTipo=0;
  
  _lcdLin1="";
  _lcdLin2="";
  
  // Ajusta a velocidade dos Motores
  motorJanela.setSpeed(MOTORJANELA_VELOCIDADE);
  motorPorta.setSpeed(MOTORPORTA_VELOCIDADE);
}




// USO INTERNO: Retorna HIGH ou LOW dependendo do valor passado (função ord não existe aqui)
int _BoolToEstado (bool valor) {
  if (valor) return HIGH;
  else return LOW;
}




// Processa Mensagens do Processador de Automação
// Lê 20 caracterers e verifica se tem a Mensagem no formato  #:x:#. Se conseguir ler algo, ajusta os parâmetros
void processaMsgAutomacao () {
  // Lê 20 caracteres e verfica se tem MSG no formato #:x:#
  // Se tiver, ajusta o ambiente se possível.
}



/*

void cmdAbrirJanela(void) {
  if ((digitalRead(PIN_CMDABRIRJANELA)==LOW) && (posicao>MINIMO_ABERTO)) {
    direcao=-1;
 
    Serial.println("Mandou Abrir Janela");
  }
}


void cmdFecharJanela(void) {
  if ((digitalRead(PIN_CMDFECHARJANELA)==LOW) && (posicao<MAXIMO_FECHADO)) {
    direcao=1;

    Serial.println("Mandou FECHAR janela");
  }
}

*/



// Verifica a condição do alarme tocando se ela mudou
void verificaAlarmeTocando () {
//
}




// Atua no Alarme (se necessário)
void atuaAlarmeTocando () {
  if (_atualAlarmeTocando!=_novoAlarmeTocando) {
    _atualAlarmeTocando=_novoAlarmeTocando;

    Blynk.virtualWrite(BLYNK_ALARMETOCANDO,_BoolToEstado(_atualAlarmeTocando));
#ifdef DEBUG
    Serial.println("-- Mudou o Status de Alarme Tocando: "+String(_atualAlarmeTocando));
#endif
    


//
//
//
//
//
  }
}  



// Verifica se mudou de "direção" o movimento da Janela e ajusta o ambiente se necessário 
void verificaJanela () {
  if (_atualJanelaDirecao!=_novoJanelaDirecao) {
    _atualJanelaDirecao=_novoJanelaDirecao;
    
    Blynk.virtualWrite(BLYNK_JANELADIRECAO,_atualJanelaDirecao);
#ifdef DEBUG
    Serial.print("-- Mudou a Direcao da Janela: ");
    if (_atualJanelaDirecao==1) Serial.println("Fechando");
    else Serial.println("Abrindo");
#endif
  }
}



// Atua na Janela de acordo com a direção, posição e status do reed
void atuaJanela () {
  if (((_atualJanelaDirecao>0) && (_janelaPosicao<MOTORJANELA_POSFECHADA)) || ((_atualJanelaDirecao<0) && (_janelaPosicao>MOTORJANELA_POSABERTA))) {
    _janelaPosicao+=_atualJanelaDirecao;
    motorJanela.step(MOTORJANELA_PASSOSVEZ*_atualJanelaDirecao);    
  }
}



// Verifica se mudou de "direção" o movimento da Porta e ajusta o ambiente se necessário 
void verificaPorta () {
  if (_atualPortaDirecao!=_novoPortaDirecao) {
    _atualPortaDirecao=_novoPortaDirecao;
    
    Blynk.virtualWrite(BLYNK_PORTADIRECAO,_atualPortaDirecao);
#ifdef DEBUG
    Serial.print("-- Mudou a Direcao da Porta: ");

    if (_atualPortaDirecao==1) Serial.println("Fechando");
    else Serial.println("Abrindo");
#endif
  }
}



// Atua na Porta de acordo com a direção, posição e status do reed
void atuaPorta () {
  if (((_atualPortaDirecao>0) && (_portaPosicao<MOTORPORTA_POSFECHADA)) || ((_atualPortaDirecao<0) && (_portaPosicao>MOTORPORTA_POSABERTA))) {
    _portaPosicao+=_atualPortaDirecao;
    motorPorta.step(MOTORJANELA_PASSOSVEZ*_atualPortaDirecao);        
  }
}





//---------- FUNÇÕES DO BLYNK ----------//


void printLCD (String texto) {
#ifndef SEMBLYNK
  WidgetLCD lcd(V0);

  _lcdLin1=_lcdLin2;
  _lcdLin2=texto;
  lcd.clear();
  lcd.print(0,0,_lcdLin1);
  lcd.print(0,1,_lcdLin2);
#endif
}



// Redefine o ambiente do Blynk quando conectado (caso caia e entre novamente).
BLYNK_CONNECTED () {
  // Manda o texto para o LCD
  printLCD("BH-ALARME OK");
//  printLCD("");
//  Blynk.virtualWrite(BLYNK_IDMORADOR,_atualMorador);
//  Blynk.virtualWrite(BLYNK_LUZEXTERNA,_BoolToEstado(_atualLuzExterna));
//  Blynk.virtualWrite(BLYNK_LUZAUTOMATICA,_BoolToEstado(_atualLuzAutomatica));
//  Blynk.virtualWrite(BLYNK_JANELAABERTA,_BoolToEstado(_atualJanelaAberta));
//  Blynk.virtualWrite(BLYNK_PORTAABERTA,_BoolToEstado(_atualPortaAberta));

}



// Armazena o conteúdo da Nova direção da Janela de acordo com o Blynk
BLYNK_WRITE(BLYNK_JANELADIRECAO) {
  _novoJanelaDirecao=param.asInt();
}



// Armazena o conteúdo da Nova direção da Porta de acordo com o Blynk
BLYNK_WRITE(BLYNK_PORTADIRECAO) {
  _novoPortaDirecao=param.asInt();
}



// Armazena o conteúdo do Novo "Alarme Tocando" de acordo com o Blynk
BLYNK_WRITE(BLYNK_ALARMETOCANDO) {
  _novoAlarmeTocando=(param.asInt()==HIGH);
}
