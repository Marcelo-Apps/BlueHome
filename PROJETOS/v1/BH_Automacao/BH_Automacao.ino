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

#define MOSTRADEBUGLCD


#ifdef DEBUG  
  #define BLYNK_PRINT Serial
#endif

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <nRF24L01.h>
#include <RF24.h>


// Parâmetros da Wi-Fi
//char ssid[] = "WIFI-BH";
//char pass[] = "BlueHome.IoT.2019";

char ssid[] = "iMordor";
char pass[] = "MyMordor2019!";


// Chave de autenticação do Projeto no Blynk
char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
//char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme


// Pinos usados para outros fins
// D16 -- U2_RX
// D17 -- U2_TX
// D18 -- SPI_SCK
// D19 -- SPI_MSIO
// D23 -- SPI_MOSI
//


#define PIN_SENSORLUZ              36
#define PIN_SENSORCHUVA            39     
#define PIN_SENSORJANELA           35
#define PIN_SENSORPORTA            34
#define PIN_LEDRED                 12
#define PIN_LEDGREEN               14
#define PIN_LEDBLUE                27
#define PIN_RELELUZEXTERNA          4

#define PIN_LEITORCARD_SPI_SCK     18     // Apenas Reserva - SPI trata disto
#define PIN_LEITORCARD_SPI_MSIO    19     // Apenas Reserva - SPI trata disto
#define PIN_LEITORCARD_SPI_MOSI    23     // Apenas Reserva - SPI trata disto
#define PIN_LEITORCARD_SDA         21     // RFID
#define PIN_LEITORCARD_RST         22     // RFID

#define PIN_BUZZER                  5     // Buzzer na protoboard do Leitor RFID
#define PIN_BOTAOFECHARPORTA        2     // Botão na Protoboard do leitor RFID

// Pinos PWM
#define PWM_LEDRED                 00
#define PWM_LEDGREEN               01
#define PWM_LEDBLUE                02

// Definições PWM
#define PWM_FREQUENCIA           1000
#define PWM_BITSRESOLUCAO           8
#define PWW_LIMITE                256
#define PWM_MIN                     0
#define PWM_MAX                   255

// Mensagens da Automacão para Alarme (RF)
#define MSGALARME_SEMACAO           0
#define MSGALARME_DESATIVA          1
#define MSGALARME_ATIVA             2
#define MSGALARME_ATIVASILENCIOSO   3
#define MSGALARME_TOCAR             4
#define MSGALARME_PARATOCAR         5
#define MSGALARME_ABREPORTA         6
#define MSGALARME_FECHAPORTA        7
#define MSGALARME_ABREJANELA        8
#define MSGALARME_FECHAJANELA       9

// Pinos Virtuais e Definições do Blynk
#define BLYNK_LCD                  V0
#define BLYNK_LUZINT_R             V1
#define BLYNK_LUZINT_G             V2
#define BLYNK_LUZINT_B             V3

#define BLYNK_LUZEXTERNA          V13
#define BLYNK_LUZAUTOMATICA       V14                 
#define BLYNK_IDMORADOR           V10

#define RFID_MAXERROS               3      // Máximo de erros do RFID antes de gerar um Alarme 

#define BEEP_NENHUM                 0
#define BEEP_BOOT                   1
#define BEEP_RFID_OK                2
#define BEEP_RFID_ERRO              3



// ** CONSTANTES **
const byte addrRadio[6] = "23232";      // Endereço de comunicação do rádio



// ** VARIÁVEIS GLOBAIS **

String _lcdLin1, _lcdLin2;

bool _atualSensorLuz, _atualLuzExterna, _atualLuzAutomatica, _atualSensorChuva, _atualBotaoFechaPorta;
bool _novoSensorLuz, _novoLuzExterna, _novoLuzAutomatica, _novoSensorChuva, _novoBotaoFechaPorta;

bool _isPortaFechada, _isJanelaFechada, _isBotaoFechaPortaApertado;

int _atualMorador, _novoLuzR, _novoLuzG, _novoLuzB;
int _novoMorador, _atualLuzR, _atualLuzG, _atualLuzB;

bool _isRecebendoCorBlynk;
int _acaoAlarme, _numErrosRfid;

String _moradorNome;



// ** CLASSES **

// Inicializa o RFID
MFRC522 rfid(PIN_LEITORCARD_SDA,PIN_LEITORCARD_RST);

// Inicializa o Rádio
RF24 radio(17,16);




// Inicialização do Ambiente
void setup() {
  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif
  // INICIALIZAÇÃO DO AMBIENTE
  pinMode(PIN_LEDRED,OUTPUT);
  pinMode(PIN_LEDGREEN,OUTPUT);
  pinMode(PIN_LEDBLUE,OUTPUT);
  ledcAttachPin(PIN_LEDRED,PWM_LEDRED);
  ledcAttachPin(PIN_LEDGREEN,PWM_LEDGREEN);
  ledcAttachPin(PIN_LEDBLUE,PWM_LEDBLUE);
  ledcSetup(PWM_LEDRED,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_LEDGREEN,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_LEDBLUE,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);

  pinMode(PIN_SENSORLUZ,INPUT);
  pinMode(PIN_SENSORCHUVA,INPUT);
  pinMode(PIN_RELELUZEXTERNA,OUTPUT);
  pinMode(PIN_BUZZER,OUTPUT);
  pinMode(PIN_SENSORJANELA,INPUT);
  pinMode(PIN_SENSORPORTA,INPUT);
  pinMode(PIN_BOTAOFECHARPORTA,INPUT);
  


  // Espera 1 segundo
#ifdef DEBUG
  Serial.println("MÓDULO BLUEHOME - AUTOMACAO");
  Serial.println("===========================");
  Serial.println("");
  Serial.println("esperando por 1s para aguardar a inicialização do processador de Alarme");
#endif
  delay(1000);

  // Inicializa os parâmetros e ajustes iniciais
  inicializaContexto();

  // Inicializa o RFID
  SPI.begin();
  rfid.PCD_Init();
#ifdef DEBUG
  Serial.println("Código do Leitor RFID inicializado.");
#endif

  // Inicializa o Rádio, define o canal e configura apenas para Transmissão
  radio.begin();
  radio.openWritingPipe(addrRadio);
  radio.stopListening();
  
#ifdef DEBUG
  Serial.println("Rádio inicializado para transmissão");
#endif

  // Espera 1 segundo
  delay(1000);

#ifndef SEMBLYNK
  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);
#endif

#ifdef DEBUG
  Serial.println("esperando por 1,5s após conectar-se para aguardar o processador de Alarme");
#endif
  // Espera mais 2 segundos antes de funcionar
  delay(1500);

  // Envia os parâmetros iniciai
  enviaDefinicoesIniciaisParaAlarme();

  // Toca o Bipe de Boot
  beepAsyncPlay(BEEP_BOOT);
  
#ifdef DEBUG
  Serial.println("** OPERAÇÃO DO PROCESSADOR DE AUTOMAÇÃO INICIADA**");
#endif
}




// Rotina Principal
void loop() {
#ifndef SEMBLYNK
    Blynk.run();
#endif

  verificaRFID();
  beepAsyncProcessa();
  
  ajustaAmbienteMorador();
  verificaEstaChovendo();            
  ajustaEstaChovendo();
  verificaStatusSensoresPortaJanela();
  verificaAtuaBotaFechaPorta();
  enviaMensagemPendenteAlarme();
  ajustaLuzInterna();
  verificaParamLuzAutomatica();
  verificaSensorLuz();
  ajustaSensorLuz();
  verificaBotaoLuzExterna();
  ajustaLuzExternaManual();
  delay(5);
}



// Inicializa as variáveis e alguns ajustes (parte do SETUP... Está aqui para ficar mais claro o código)
void inicializaContexto (void) {
  // Inicializa Portas Necessárias
  digitalWrite(PIN_RELELUZEXTERNA,HIGH);
  ledcWrite(PWM_LEDRED,PWM_MIN);
  ledcWrite(PWM_LEDGREEN,PWM_MIN);
  ledcWrite(PWM_LEDBLUE,PWM_MIN);

  // Inicializa as variáveis do Beep Assíncrono
  beepAsyncInicializa();
  
  // Inicializa as Variáveis
  _atualSensorLuz=false;
  _novoSensorLuz=false;
  _atualLuzExterna=false;
  _novoLuzExterna=false;
  _atualLuzAutomatica=true;
  _novoLuzAutomatica=true;
  _atualSensorChuva=false;
  _novoSensorChuva=false;
  _atualMorador=0;
  _novoMorador=0;
  _atualLuzR=-1;         // É assim mesmo... Para ajustar a luz na primeira vez
  _atualLuzG=-1;
  _atualLuzB=-1;
  _novoLuzR=0;
  _novoLuzG=0;
  _novoLuzB=0;
  _isPortaFechada=false;
  _isJanelaFechada=false;
  _isBotaoFechaPortaApertado=false;
  
  _isRecebendoCorBlynk=false;

  _acaoAlarme=MSGALARME_SEMACAO;
  _numErrosRfid=0;
  
  _moradorNome="";

  _lcdLin1="";
  _lcdLin2="";

  // Coloca o RFID em modo Halt (por garantia)
  rfid.PICC_HaltA(); 
}




// Envia as definições de Status iniciais para o Processador de Alarme 
void enviaDefinicoesIniciaisParaAlarme () {
  _enviaMsgParaAlarme(MSGALARME_DESATIVA);
  delay(100);
  _enviaMsgParaAlarme(MSGALARME_FECHAPORTA);
  delay(100);
  _enviaMsgParaAlarme(MSGALARME_FECHAJANELA);
  delay(100);
}




// Função de Apoio - Envio de Mensagens (usada pelas funções do loop e de inicialização)
void _enviaMsgParaAlarme (int Mensagem) {
  String txtEnvio = "::"+String(Mensagem)+"::";
  
  radio.write(&txtEnvio[0],txtEnvio.length());
#ifdef DEBUG
   Serial.println("===>Enviada Mensagem para Processador de Alarme via RF: "+txtEnvio);
#endif
}




// USO INTERNO: Retorna HIGH ou LOW dependendo do valor passado (função ord não existe aqui)
int _BoolToEstado (bool valor) {
  if (valor)
    return HIGH;
  else
    return LOW;
}




// Verifica se tem um cartao na leitora
void verificaRFID (void) {
  
  if ((rfid.PICC_IsNewCardPresent()) && (rfid.PICC_ReadCardSerial())) {
    String chave = "";

#ifdef DEBUG

#endif

    for (byte ind=0;ind<rfid.uid.size;ind++) {
      int num=rfid.uid.uidByte[ind];
      if (ind!=0) chave.concat(".");
      if (num<0x10) chave.concat("0");
      chave.concat(String(num,HEX));
    }
#ifdef DEBUG
    Serial.println("-- ***TAG RFID Inserida - Código: ("+chave+")");
#endif
    
    rfid.PICC_HaltA();

    if (chave=="17.86.7d.26") {
      _novoMorador=1;
    } else if (chave=="79.52.99.02") {
      _novoMorador=2;
    } else if (chave=="c7.74.1a.26") {
      _novoMorador=3;
    } else {
      _novoMorador=0;
    }

    if (_novoMorador!=0) {
      _numErrosRfid=0;
      // Desativa o alarme...
      _enviaMsgParaAlarme(MSGALARME_DESATIVA);
      // Aguarda 10ms
      delay(10);
      _acaoAlarme=MSGALARME_ABREPORTA;
      beepAsyncPlay(BEEP_RFID_OK);
//      printLCD("OK: "+chave);
#ifdef DEBUG
      Serial.print(" -- RFID Autorizado: Usuário: ");
      Serial.println(_novoMorador);
#endif
    } else {
      _numErrosRfid++;
      printLCD("ERR: "+chave);
      printLCD("Tag Invalido ("+String(_numErrosRfid)+")");
      beepAsyncPlay(BEEP_RFID_ERRO);
#ifdef DEBUG
      Serial.print(" -- RFID *INVÁLIDO* -- TENTATIVAS COM ERRO: ");
      Serial.println(_numErrosRfid);
#endif
      if (_numErrosRfid==RFID_MAXERROS) {
        _acaoAlarme=MSGALARME_TOCAR;
       printLCD("ALARME ACIONADO");
#ifdef DEBUG
      Serial.println(" -- *** Excedeu Número de Acessos Inválidos - VAI DISPARAR O ALARME ***");
#endif
      }
    }
  }
}




void verificaStatusSensoresPortaJanela () {
  bool lidoPortaFechada = (digitalRead(PIN_SENSORPORTA)==LOW);
  bool lidoJanelaFechada = (digitalRead(PIN_SENSORJANELA)==LOW);


  if (_isPortaFechada!=lidoPortaFechada) {
    _isPortaFechada=lidoPortaFechada;

#ifdef DEBUG
    Serial.println("- Porta Fechada Mudou de Status para "+String(_isPortaFechada));
#endif
  }
  
  if (_isJanelaFechada!=lidoJanelaFechada) {
    _isJanelaFechada=lidoJanelaFechada;
  
#ifdef DEBUG
    Serial.println("- Janela Fechada Mudou de Status para "+String(_isJanelaFechada));
#endif
  }
}




// Verifica se o Botão de Fechar a Porta (na Protoboard do RFID) está apertado
// OBS: Como ele gera uma Ação para o Alarme, só executa se não tiver Ações Pendentes
void verificaAtuaBotaFechaPorta () {
  if (_acaoAlarme==0) {
    bool lidoBotaoFechaPortaApertado = (digitalRead(PIN_BOTAOFECHARPORTA)==HIGH);
  
    if (lidoBotaoFechaPortaApertado!=_isBotaoFechaPortaApertado)
    {
      _isBotaoFechaPortaApertado=lidoBotaoFechaPortaApertado;
  
  #ifdef DEBUG
      Serial.println("- Botão de Fechar a Porta mudou de Estado. . Ficou = "+String(_isBotaoFechaPortaApertado));
  #endif
      
      if (_isBotaoFechaPortaApertado) {
        if (_isPortaFechada) {
          printLCD("Porta já Fechada"); 
        }
        else {
          _acaoAlarme=MSGALARME_FECHAPORTA;
          printLCD("Fechou a Porta");
#ifdef DEBUG
        Serial.println("  -- Sinalizou para Fechar a Porta");
#endif
        }
      }
    }
  }
}




void verificaEstaChovendo () {
  bool lido = (digitalRead(PIN_SENSORCHUVA)==LOW);

  // Só registra se o "novo" estado já não estava modificado 
  if (lido!=_novoSensorChuva) {
    _novoSensorChuva=lido;
    
#ifdef DEBUG
    Serial.println("-- Sensor de Chuva Mudou de Estado. Ficou = "+String(_novoSensorChuva));
#endif
  }
}



// OBS: Só pode ajustar o sensor de chuva se não tiver mensagem de alarme pendente para processamento
void ajustaEstaChovendo () {
  if (_acaoAlarme==0) {
    if (_atualSensorChuva!=_novoSensorChuva) {
      _atualSensorChuva=_novoSensorChuva;
  
      if (_atualSensorChuva) {
        printLCD("Esta Chovendo");
  #ifdef DEBUG
        Serial.println("  -- Começou a Chover");
  #endif
        // Se janela está aberta, sinaliza abertura, senão apenas mostra no LCD que janela já fechada
        if (!_isJanelaFechada) {
          _acaoAlarme=MSGALARME_FECHAJANELA;
          printLCD("Fechando Janela");
  #ifdef DEBUG
          Serial.println("  -- Sinalizou para Fechar a Janela");
  #endif
        } else {
          printLCD("Jan.Tava Fechada");
        }
      } else {
        printLCD("Parou de Chover");
  #ifdef DEBUG
      Serial.println("  -- Parou de Chover");
  #endif
      }
    }
  }
}




// Envia Mensagem Pendente (se tiver) para o Processador de Alarme
void enviaMensagemPendenteAlarme () {
  if (_acaoAlarme!=0) {
    _enviaMsgParaAlarme(_acaoAlarme);
    _acaoAlarme=0;
  }
}




void ajustaAmbienteMorador () {
  if ((_novoMorador!=0) && (_atualMorador!=_novoMorador)) {
    
#ifdef DEBUG
    Serial.println("--Mudou de Morador. Era: "+String(_atualMorador)+", e agora é: "+String(_novoMorador));
#endif
    _atualMorador=_novoMorador;
    // Informa que NÃO está recebendo cor do Blynk
    _isRecebendoCorBlynk=false;
    // ** Parâmetros Fixados no Código - Apenas para POC
    if (_atualMorador==1) {
      _novoLuzR=200;
      _novoLuzG=255;
      _novoLuzB=0;
      _moradorNome="Marcelo";
    } else if (_atualMorador==2) {
      _novoLuzR=255;
      _novoLuzG=50;
      _novoLuzB=50;
      _moradorNome="Felipe";
    } else if (_atualMorador==3) {
      _novoLuzR=100;
      _novoLuzG=100;
      _novoLuzB=255;
      _moradorNome="Newton";
    } else {
      _novoLuzR=0;
      _novoLuzG=0;
      _novoLuzB=0;
      _moradorNome="";
    }

    if (_moradorNome!="") {
    printLCD("Ola "+_moradorNome);
    // Atualiza o Morador no Blynk
    Blynk.virtualWrite(BLYNK_IDMORADOR,_atualMorador);
    }
  }
}




void ajustaLuzInterna () {
  // Nota: Não faz a mudança se o Blynk ainda estiver recebendo as novas cores
  if ((!_isRecebendoCorBlynk)  && ((_atualLuzR!=_novoLuzR) || (_atualLuzG!=_novoLuzG) || (_atualLuzB!=_novoLuzB))) {
    // Ajusta a cor...
    _atualLuzR=_novoLuzR;
    _atualLuzG=_novoLuzG;
    _atualLuzB=_novoLuzB;
    // Informa que NÃO está recebendo cor do Blynk
    _isRecebendoCorBlynk=false;
    
    ledcWrite(PWM_LEDRED,_atualLuzR);
    ledcWrite(PWM_LEDGREEN,_atualLuzG);
    ledcWrite(PWM_LEDBLUE,_atualLuzB);

    Blynk.virtualWrite(BLYNK_LUZINT_R,_atualLuzR);
    Blynk.virtualWrite(BLYNK_LUZINT_G,_atualLuzG);
    Blynk.virtualWrite(BLYNK_LUZINT_B,_atualLuzB);
#ifdef DEBUG
    Serial.println("--Ajustou a Luz Interna: (R: "+String(_novoLuzR)+", G: "+String(_novoLuzG)+", B: "+String(_novoLuzB)+")");
#endif
  }
}




// Lê o estado do sensor de Luz
void verificaSensorLuz (void) {
  _novoSensorLuz=(digitalRead(PIN_SENSORLUZ)==HIGH);
}




void verificaParamLuzAutomatica (void) {
  if (_atualLuzAutomatica!=_novoLuzAutomatica) {
    _atualLuzAutomatica=_novoLuzAutomatica;

    if (_atualLuzAutomatica) {
      // Esta atgribuição força a execução do "ajustaSensorLuz"
      _atualSensorLuz=!_novoSensorLuz;
    }
#ifdef DEBUG
     Serial.println("-- Mudou o Estado do Parâmetro Luz Automática");
     Serial.println(" -- NOVO estado: "+String(_novoLuzAutomatica));
#endif
  }
}




// Atua na luz externa se o Sensor de Luz modificou e está em luz automática
void ajustaSensorLuz (void) {
  if (_atualSensorLuz!=_novoSensorLuz) {

    _atualSensorLuz=_novoSensorLuz;
    
    // Mostra na tela que escureceu ou Clareou
    if (_atualSensorLuz)
      printLCD("Céu Escureceu");
    else
      printLCD("Céu Clareou");
      
#ifdef DEBUG
    Serial.println("-- Mudou o Estado do Sensor de Luz para = "+String(_atualSensorLuz));
#endif

    // Executa a ação de Luz Automática estiver ativa
    if (_atualLuzAutomatica) {
      if (_atualSensorLuz) {
        digitalWrite(PIN_RELELUZEXTERNA,LOW);
        Blynk.virtualWrite(BLYNK_LUZEXTERNA,HIGH);
        printLCD("Luz.Ext. Ligada");
        _atualLuzExterna=true;
        _novoLuzExterna=true;
  #ifdef DEBUG
        Serial.println(" -- Ligou a Luz Externa Automaticamente");
  #endif
      } else {
        digitalWrite(PIN_RELELUZEXTERNA,HIGH);
        Blynk.virtualWrite(BLYNK_LUZEXTERNA,LOW);
        printLCD("Luz.Ext. Deslig.");
        _atualLuzExterna=false;
        _novoLuzExterna=false;
  #ifdef DEBUG
        Serial.println(" -- Desligou a Luz Externa Automaticamente");
  #endif
      }
    }
  }
}




// Verifica e corrige o botão de luz Externa Acessa se a Luz Automática estiver atuiva
void verificaBotaoLuzExterna (void) {
  if ((_atualLuzExterna!=_novoLuzExterna) && (_atualLuzAutomatica)) {
    // Reverte o estado para o atual
    _novoLuzExterna=_atualLuzExterna;
    Blynk.virtualWrite(BLYNK_LUZEXTERNA,_BoolToEstado(_atualLuzExterna));
#ifdef DEBUG
    Serial.println("-- CORREÇÃO DE ESTADO: Tentou mudar Luz Externa com Luz Automática Ativa.");
#endif
  }
}



// Ajusta a Luz Manualmente SE a luz automática não estiver ativa
void ajustaLuzExternaManual (void) {
  if ((_atualLuzExterna!=_novoLuzExterna)  && (!_atualLuzAutomatica)) {
    
    _atualLuzExterna=_novoLuzExterna;
    Blynk.virtualWrite(BLYNK_LUZEXTERNA,_BoolToEstado(_atualLuzExterna));
    

    if (_atualLuzExterna) {
      printLCD("Luz.Ext. Ligada");
      digitalWrite(PIN_RELELUZEXTERNA,LOW);
    } else {
      printLCD("Luz.Ext.Deslig.");
      digitalWrite(PIN_RELELUZEXTERNA,HIGH);
    }
#ifdef DEBUG
    Serial.println("-- LUZ EXTERNA MANUAL: Mudou estado para "+String(_atualLuzExterna));
#endif
  }
}




//---------- FUNÇÕES DO BEEP ASSÍNCRONO ----------//
// Estas funções permitem criar um toque personalizado que é programado em um Array de Inteiros
// O toque é assíncrono e utiliza millis() para definir se está na hora de trocar
// A função beepAsyncProcessa trata dos detalhes da troca de Estado


int *__beepAsyncPtr;
int __beepAsyncCnt, __beepAsyncPos;
bool __beepAsyncIsTocando;
unsigned long __beepAsyncTempoLimite;

//---- Arrays de Beeps pré-definidos - Neste aplicativo só Temos estes ----//
#define BEEPSCNT_BOOT               3
int __beepAsync_ArrayBoot[BEEPSCNT_BOOT] = {400,20,100};

#define BEEPSCNT_OK                 1
int __beepAsync_ArrayOK[BEEPSCNT_OK] = {100};

#define BEEPSCNT_ERR                5
int __beepAsync_ArrayERR[BEEPSCNT_ERR] = {80,50,80,50,120};




// Toca o Bipe selecionado
void beepAsyncPlay (int tipoBeep) {
  switch (tipoBeep) {
    case BEEP_BOOT   :    __beepAsyncTocaSeq(&__beepAsync_ArrayBoot[0],BEEPSCNT_BOOT);
                          break;

    case BEEP_RFID_OK :   __beepAsyncTocaSeq(&__beepAsync_ArrayOK[0],BEEPSCNT_OK);
                            break;

    case BEEP_RFID_ERRO : __beepAsyncTocaSeq(&__beepAsync_ArrayERR[0],BEEPSCNT_ERR);
                            break;
     default :
       beepAsyncInicializa();
  }
}




// (CHAMAR NO LOOP) - Processa o Beep se estiver tocando (diferente de -1)
void beepAsyncProcessa () {
  if (__beepAsyncPos!=-1) {
    if (millis()>__beepAsyncTempoLimite) {
      __beepAsyncMudaEstado();
    }
  }
}



// Inicializa o estado do Bipe
void beepAsyncInicializa () {
  __beepAsyncCnt=0;
  __beepAsyncPos=-1;
  __beepAsyncIsTocando=false;
  digitalWrite(PIN_BUZZER,LOW);
}



void __beepAsyncMudaEstado () {
  if (__beepAsyncPos!=-1) {
    
    if (__beepAsyncPos<__beepAsyncCnt) {
      __beepAsyncIsTocando=!__beepAsyncIsTocando;
      __beepAsyncTempoLimite=millis()+__beepAsyncPtr[__beepAsyncPos];
      __beepAsyncPos++;
    }
    else {
      __beepAsyncPos=-1;
      __beepAsyncIsTocando=false;
    }
  
    if (__beepAsyncIsTocando) {
      digitalWrite(PIN_BUZZER,HIGH);
//      Serial.println("Beep Tocando");
    } else {
      digitalWrite(PIN_BUZZER,LOW);
//      Serial.println("Beep Sem Som");
    }
  }
}



void __beepAsyncTocaSeq (int *ptrBeep, int cntBeep) {
  __beepAsyncPtr=ptrBeep;
  __beepAsyncCnt=cntBeep;

  __beepAsyncPos=0;
  __beepAsyncIsTocando=false;
  __beepAsyncMudaEstado();
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
  printLCD("BH-AUTOMACAO OK");
//  printLCD("");
  Blynk.virtualWrite(BLYNK_IDMORADOR,_atualMorador);
  Blynk.virtualWrite(BLYNK_LUZEXTERNA,_BoolToEstado(_atualLuzExterna));
  Blynk.virtualWrite(BLYNK_LUZAUTOMATICA,_BoolToEstado(_atualLuzAutomatica));
  
  // Nestes três é preciso ser o "_novo", pois o "_atual" tem -1 no início
  Blynk.virtualWrite(BLYNK_LUZINT_R,_novoLuzR);
  Blynk.virtualWrite(BLYNK_LUZINT_G,_novoLuzG);
  Blynk.virtualWrite(BLYNK_LUZINT_B,_novoLuzB);

  printLCD("Ola "+_moradorNome);
}



// Mudança da Luz - Vermelha (R)
BLYNK_WRITE(BLYNK_LUZINT_R) {
  _novoLuzR=param.asInt();
  // Informa que está recebendo cor (vem R, G e B)
  _isRecebendoCorBlynk=true;
#ifdef DEBUG
  Serial.println(" - Cor: Novo R: "+String(_novoLuzR));
#endif  
}



// Mudança da Luz - Verde (G)
BLYNK_WRITE(BLYNK_LUZINT_G) {
  _novoLuzG=param.asInt();
  // Informa que está recebendo cor (vem R, G e B)
  _isRecebendoCorBlynk=true;
#ifdef DEBUG
  Serial.println(" - Cor: Novo G: "+String(_novoLuzG));
#endif  
}



// Mudança da Luz - Azul (B)
BLYNK_WRITE(BLYNK_LUZINT_B) {
  _novoLuzB=param.asInt();
  // Informa que JÁ RECEBEU (vem R, G e B)
  _isRecebendoCorBlynk=false;
#ifdef DEBUG
  Serial.println(" - Cor: Novo B: "+String(_novoLuzB));
#endif    
}




BLYNK_WRITE(BLYNK_IDMORADOR) {
  int valor = param.asInt();
  // Morador = 0 não deveria existir (existe porquê pode não ter ninguém no local
  if (valor!=0) {
    _novoMorador=valor;
#ifdef DEBUG
    Serial.println("-->Mudança de Morador pelo Controlador. Código = "+String(_novoMorador));
#endif    
  }
  else if (_atualMorador!=0) {
    // Se tentou passar 0 e já tem um morador, muda para este
    Blynk.virtualWrite(BLYNK_IDMORADOR,_atualMorador);
  }
}




// Apenas lê o conteúdo da Luz Automática (a atualização é no novo ciclo)
BLYNK_WRITE(BLYNK_LUZAUTOMATICA) {
  _novoLuzAutomatica=(param.asInt()!=0);
}




// Apenas lê o conteúdo da Luz Manual (a atualização é no novo ciclo)
// Tem um detalhe: Se luz automática = ON, não pode mexer aqui.
BLYNK_WRITE(BLYNK_LUZEXTERNA) {
  _novoLuzExterna=(param.asInt()!=0);
#ifdef DEBUG  
  Serial.println("-->Mudança via Blynk de Luz Externa. Atual = "+String(_novoLuzExterna));
#endif    
}
