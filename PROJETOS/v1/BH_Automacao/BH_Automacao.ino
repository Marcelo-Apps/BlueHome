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
#include <SPI.h>
#include <MFRC522.h>
#include <HardwareSerial.h>



// Parâmetros da Wi-Fi
char ssid[] = "WIFI-BH";
char pass[] = "BlueHome.IoT.2019";


// Chave de autenticação do Projeto no Blynk
char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
//char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme

// Pinos usados para outros fins
// D16 -- U2_RX
// D17 -- U2_TX
// D18 -- SPI_SCK
// D19 -- SPI_MSIO
// D23 -- SPI_MOSI
// D05 -- Aviso Alarme--Automacao
// D04 - RESERVADO ENQUANTO FAZ COMUNICACAO SERIAL COM PC
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

#define PIN_BEEP                    5     // Buzzer na protoboard do Leitor RFID

// Pinos da comunicação entre Processadores
#define PIN_SERIAL_COMUNIC_RX      16
#define PIN_SERIAL_COMUNIC_TX      17
#define SERIALRF_VELOCIDADE       900      // Velocidade da Serial de RF (entre Microcontroladores)

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
#define BLYNK_LUZINT_R             V1
#define BLYNK_LUZINT_G             V2
#define BLYNK_LUZINT_B             V3

#define BLYNK_JANELAABERTA        V11
#define BLYNK_PORTAABERTA         V12
#define BLYNK_LUZEXTERNA          V13
#define BLYNK_LUZAUTOMATICA       V14                 
#define BLYNK_IDMORADOR           V10


#define RFID_MAXERROS               3      // Máximo de erros do RFID antes de gerar um Alarme 


// ** VARIÁVEIS GLOBAIS **

String _lcdLin1, _lcdLin2;


bool _atualSensorLuz, _atualLuzExterna, _atualLuzAutomatica, _atualLuzManual, _atualSensorChuva, _atualJanelaAberta, _atualPortaAberta;
bool _novoSensorLuz, _novoLuzExterna, _novoLuzAutomatica, _novoLuzManual, _novoSensorChuva, _novoJanelaAberta, _novoPortaAberta;

int _atualMorador, _novoLuzR, _novoLuzG, _novoLuzB;
int _novoMorador, _atualLuzR, _atualLuzG, _atualLuzB;

bool _isRecebendoCorBlynk;
int _acaoAlarme, _numErrosRfid;

String _moradorNome;




// ** CLASSES **

// Inicializa o RFID
MFRC522 rfid(PIN_LEITORCARD_SDA,PIN_LEITORCARD_RST);

// Inicializa a serial para comunicação RF entre Automação e Alarme
HardwareSerial TxSerial(1);



// Inicialização dos parâmetos
void setup() {
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

  pinMode(PIN_SENSORLUZ, INPUT);
  pinMode(PIN_SENSORCHUVA, INPUT);
  pinMode(PIN_RELELUZEXTERNA, OUTPUT);
  pinMode(PIN_BEEP,OUTPUT);

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

  // Inicializa a Serial entre Automação e Alarme
  TxSerial.begin(SERIALRF_VELOCIDADE,SERIAL_6E2,16,17);   // (BR,Prot,RX,TX);
#ifdef DEBUG
  Serial.println("Serial entre Automação e Alarme Inicializada (via Sinal de RF)");
  Serial.println("velocidade (bps): "+String(SERIALRF_VELOCIDADE));
#endif
  // Espera 1 segundo
  delay(1000);

#ifndef SEMBLYNK
  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);
#endif
#ifdef DEBUG
  Serial.println("esperando por 2s após conectar-se para aguardar o processador de Alarme");
#endif
  // Espera mais 2 segundos antes de funcionar
  delay(2000);

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
  beepAssyncProcessa();
  ajustaAmbienteMorador();
  verificaEstaChovendo();            
  ajustaEstaChovendo();
  verificaCmdPortaAberta();
  verificaCmdJanelaAberta();
  enviaMensagemPendenteAlarme();
  ajustaLuzInterna();
  verificaParamLuzAutomatica();
  verificaSensorLuz();
  ajustaSensorLuz();
  delay(5);
}



void inicializaContexto (void) {
  // Pinos
  digitalWrite(PIN_RELELUZEXTERNA,HIGH);
  
  // Variáveis
  _atualSensorLuz=false;
  _novoSensorLuz=false;
  _atualLuzExterna=false;
  _novoLuzExterna=false;
  _atualLuzAutomatica=true;
  _novoLuzAutomatica=true;
  _atualLuzManual=false;
  _novoLuzManual=false;
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
  _atualJanelaAberta=false;
  _novoJanelaAberta=false;
  _atualPortaAberta=false;
  _novoPortaAberta=false;

  _isRecebendoCorBlynk=false;

  _acaoAlarme=ALARME_SEMACAO;
  _numErrosRfid=0;
  
  _moradorNome="";

  _lcdLin1="";
  _lcdLin2="";

  ledcWrite(PWM_LEDRED,PWM_MIN);
  ledcWrite(PWM_LEDGREEN,PWM_MIN);
  ledcWrite(PWM_LEDBLUE,PWM_MIN);
  
  _enviaMsgParaAlarme(ALARME_DESATIVA);
  _enviaMsgParaAlarme(ALARME_FECHAPORTA);
  _enviaMsgParaAlarme(ALARME_FECHAJANELA);
  
  // Coloca o RFID em modo Halt (por garantia)
  rfid.PICC_HaltA(); 
  
  // Inicializa as variáveis do Beep Assíncrono
  beepAssyncInicializa();
}



// Função de Apoio - Envio de Mensagens (usada pelas funções do loop e de inicialização
//OBS: O prefixo é importante, pois os primeiros caracteres são perdidos (entre 4 e 10)
void _enviaMsgParaAlarme (int Mensagem) {
  String texto = ". . . . . . #:"+String(Mensagem)+":# ";

  TxSerial.flush();
  TxSerial.print(texto);
#ifdef DEBUG
   Serial.println("===>Enviada Mensagem para Processador de Alarme via RF: "+texto);
#endif
}



// USO INTERNO: Retorna HIGH ou LOW dependendo do valor passado (função ord não existe aqui)
int _BoolToEstado (bool valor) {
  if (valor) {
    return HIGH;
  } else {
    return LOW;
  }
}




// Verifica se tem um cartao na leitora
void verificaRFID (void) {
  
  if ((rfid.PICC_IsNewCardPresent()) && (rfid.PICC_ReadCardSerial())) {
    String chave = "";

#ifdef DEBUG
    Serial.print("-- ***TAG RFID Inserida - Código: ");
#endif

    for (byte ind=0;ind<rfid.uid.size;ind++) {
      int num=rfid.uid.uidByte[ind];
      if (ind!=0) chave.concat(".");
      if (num<0x10) chave.concat("0");
      chave.concat(String(num,HEX));
    }
#ifdef DEBUG
    Serial.println("("+chave+")");
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
      _acaoAlarme=ALARME_DESATIVA;
      beepAssyncPlay(true);
//      printLCD("OK: "+chave);
#ifdef DEBUG
      Serial.print(" -- RFID Autorizado: Usuário: ");
      Serial.println(_novoMorador);
#endif
    } else {
      _numErrosRfid++;
      printLCD("ERR: "+chave);
      printLCD("Tag Invalido ("+String(_numErrosRfid)+")");
      beepAssyncPlay(false);
#ifdef DEBUG
      Serial.print(" -- RFID *INVÁLIDO* -- TENTATIVAS COM ERRO: ");
      Serial.println(_numErrosRfid);
#endif
      if (_numErrosRfid==RFID_MAXERROS) {
        _acaoAlarme=ALARME_TOCAR;
       printLCD("ALARME ACIONADO");
#ifdef DEBUG
      Serial.println(" -- *** Excedeu Número de Acessos Inválidos - VAI DISPARAR O ALARME ***");
#endif
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
    Serial.println("-- Sensor de Chuva Mudou de Estado. Ficou = "+String(_novoSensorChuva)+")");
#endif
  }
}




void ajustaEstaChovendo () {
  if (_atualSensorChuva!=_novoSensorChuva) {
    _atualSensorChuva=_novoSensorChuva;

    if (_atualSensorChuva) {
      printLCD("Esta Chovendo");
#ifdef DEBUG
      Serial.println("  -- Começou a Chover");
#endif
      // Se janela está fechada, sinaliza abertura, senão apenas mostra no LCD que janela já fechada
      if (_novoJanelaAberta) {
        _novoJanelaAberta=false;
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




void verificaCmdJanelaAberta () {
  // Se tiver comandos a executar no alarme só muda status da janela no próximo ciclo
  if ((_acaoAlarme==0) && (_atualJanelaAberta!=_novoJanelaAberta)) {

    _atualJanelaAberta=_novoJanelaAberta;
    
    if (_atualJanelaAberta) {
      _acaoAlarme=ALARME_ABREJANELA;
      printLCD("Abriu a Janela");
    } else {
      _acaoAlarme=ALARME_FECHAJANELA;
      printLCD("Fechou a Janela");
    }
    
    Blynk.virtualWrite(BLYNK_JANELAABERTA,_BoolToEstado(_atualJanelaAberta));
    
#ifdef DEBUG
    Serial.println("-- Solicitou para Mudar Estado de Janela Aberta para "+String(_atualJanelaAberta));
#endif
  }
}




void verificaCmdPortaAberta () {
  // Se tiver comandos a executar no alarme só muda status da janela no próximo ciclo
  if ((_acaoAlarme==0) && (_atualPortaAberta!=_novoPortaAberta)) {

    _atualPortaAberta=_novoPortaAberta;
    
    if (_atualPortaAberta) {
      _acaoAlarme=ALARME_ABREPORTA;
    } else {
      _acaoAlarme=ALARME_FECHAPORTA;
    }
    
    Blynk.virtualWrite(BLYNK_PORTAABERTA,_BoolToEstado(_atualPortaAberta));
    
#ifdef DEBUG
    Serial.println("-- Solicitou para Mudar Estado de Porta Aberta para "+String(_atualJanelaAberta));
#endif
  }
}




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
      _moradorNome="Faberson";
    } else {
      _novoLuzR=0;
      _novoLuzG=0;
      _novoLuzB=0;
      _moradorNome="";
    }

    if (_moradorNome!="") {
    printLCD("Ola "+_moradorNome);
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




void verificaParamLuzAutomatica (void) {
  if (_atualLuzAutomatica!=_novoLuzAutomatica) {
#ifdef DEBUG
     Serial.println("-- Mudou o Estado do Parâmetro Luz Automática");
     Serial.println(" -- NOVO estado: "+String(_novoLuzAutomatica));
#endif
  }
}




// Lê o sensor de Luz SE JÁ não estiver modificado
void verificaSensorLuz (void) {
  if (_atualSensorLuz==_novoSensorLuz) {
    _novoSensorLuz=(digitalRead(PIN_SENSORLUZ)==HIGH);
  }
}




// Atua no sensor de Luz SE Precisar
void ajustaSensorLuz (void) {
  if ((_atualSensorLuz!=_novoSensorLuz)  || (_atualLuzAutomatica!=_novoLuzAutomatica)) {

    // Faz a atribuição de Atual=Novo
    _atualSensorLuz=_novoSensorLuz;
    _atualLuzAutomatica=_novoLuzAutomatica;

#ifdef DEBUG
      Serial.println("-- Mudou o Estado do Sensor de Luz");
#endif

    if (_atualLuzAutomatica) {
      // Executa a ação
      if (_atualSensorLuz) {
        digitalWrite(PIN_RELELUZEXTERNA,LOW);
        Blynk.virtualWrite(BLYNK_LUZEXTERNA,LOW);
        printLCD("Luz.Ext. Ligada");
        _atualLuzManual=true;
        _novoLuzManual=true;
#ifdef DEBUG
        Serial.println(" -- Ligou a Luz Externa");
#endif
      } else {
        digitalWrite(PIN_RELELUZEXTERNA,HIGH);
        Blynk.virtualWrite(BLYNK_LUZEXTERNA,HIGH);
        printLCD("Luz.Ext. Deslig.");
        _atualLuzManual=false;
        _novoLuzManual=false;
#ifdef DEBUG
        Serial.println(" -- Desligou a Luz Externa");
#endif
      }
    } else {
#ifdef DEBUG
      Serial.println(" -- *NÃO FEZ NADA -- LUZ AUTOMÁTICA ESTÁ DESLIGADA*");
#endif
    }
  }
}


//INCOMPLETO
//INCOMPLETO
//INCOMPLETO
//INCOMPLETO
//INCOMPLETO
//INCOMPLETO
//INCOMPLETO
//INCOMPLETO
// Atua no comando de Luz Manual (se não estiver no automático)
void atuaLuzManual (void) {
  if ((_atualLuzManual!=_novoLuzManual)  && (!_atualLuzAutomatica)) {
        digitalWrite(PIN_RELELUZEXTERNA,LOW);
    
  }
}




//---------- FUNÇÕES DO BEEP ASSÍNCRONO ----------//
// Estas funções permitem criar um toque personalizado que é programado em um Array de Inteiros
// O toque é assíncrono e utiliza millis() para definir se está na hora de trocar
// A função beepAssyncProcessa trata dos detalhes da troca de Estado


int *__beepAssyncPtr;
int __beepAssyncCnt, __beepAssyncPos;
bool __beepAssyncIsTocando;
unsigned long __beepAssyncTempoLimite;

//---- Arrays de Beeps pré-definidos - Neste aplicativo só Temos estes ----//
#define BEEPSCNT_OK       1
int __beepAssync_ArrayOK[BEEPSCNT_OK] = {100};

#define BEEPSCNT_ERR      5
int __beepAssync_ArrayERR[BEEPSCNT_ERR] = {80,50,80,50,120};




void beepAssyncPlay (bool isOk) {
  if (isOk) {
    __beepAssyncTocaSeq(&__beepAssync_ArrayOK[0],BEEPSCNT_OK);
  } else {
    __beepAssyncTocaSeq(&__beepAssync_ArrayERR[0],BEEPSCNT_ERR);
  }
}



// Processa o Beep se estiver tocando (diferente de -1)
void beepAssyncProcessa () {
  if (__beepAssyncPos!=-1) {
    if (millis()>__beepAssyncTempoLimite) {
      __beepAssyncMudaEstado();
    }
  }
}



void beepAssyncInicializa () {
  __beepAssyncCnt=0;
  __beepAssyncPos=-1;
  __beepAssyncIsTocando=false;
  pinMode(PIN_BEEP,OUTPUT);
  digitalWrite(PIN_BEEP,LOW);
}



void __beepAssyncMudaEstado () {
  if (__beepAssyncPos!=-1) {
    
    if (__beepAssyncPos<__beepAssyncCnt) {
      __beepAssyncIsTocando=!__beepAssyncIsTocando;
      __beepAssyncTempoLimite=millis()+__beepAssyncPtr[__beepAssyncPos];
      __beepAssyncPos++;
    }
    else {
      __beepAssyncPos=-1;
      __beepAssyncIsTocando=false;
    }
  
    if (__beepAssyncIsTocando) {
      digitalWrite(PIN_BEEP,HIGH);
//      Serial.println("Beep Tocando");
    } else {
      digitalWrite(PIN_BEEP,LOW);
//      Serial.println("Beep Sem Som");
    }
  }
}



void __beepAssyncTocaSeq (int *ptrBeep, int cntBeep) {
  __beepAssyncPtr=ptrBeep;
  __beepAssyncCnt=cntBeep;

  __beepAssyncPos=0;
  __beepAssyncIsTocando=false;
  __beepAssyncMudaEstado();
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
  Blynk.virtualWrite(BLYNK_JANELAABERTA,_BoolToEstado(_atualJanelaAberta));
  Blynk.virtualWrite(BLYNK_PORTAABERTA,_BoolToEstado(_atualPortaAberta));
  
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



BLYNK_WRITE(BLYNK_PORTAABERTA) {
  _novoPortaAberta=param.asInt();
#ifdef DEBUG
  Serial.println("-->Mudança de Estado da Porta pelo Controlador. Aberta = "+String(_novoPortaAberta));
#endif    
}



BLYNK_WRITE(BLYNK_JANELAABERTA) {
  _novoJanelaAberta=param.asInt();
#ifdef DEBUG
  Serial.println("-->Mudança de Estado da Janela pelo Controlador. Aberta = "+String(_novoJanelaAberta));
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

/*
// Apenas lê o conteúdo da Luz Manual (a atualização é no novo ciclo)
// Tem um detalhe: Se luz automática = ON, não pode mexer aqui.
BLYNK_WRITE(BLYNK_LUZEXTERNA) {
  bool lidoLuzManual=(param.asInt()!=0);
  
  if (!_novoLuzAutomatica) {
    _novoLuzManual=lidoLuzManual;
#ifdef DEBUG
  Serial.print("--Redefiniu Luz Manual");
#endif
  } else if (lidoLuzManual!=_atualLuzManual)
  {
#ifdef DEBUG
  Serial.print("--TENTOU MUDAR LUZ MANUAL, MAS NÃO PODE");
#endif
    if (_atualLuzManual) {
      Blynk.virtualWrite(BLYNK_LUZEXTERNA,HIGH);
    } else
    {
      Blynk.virtualWrite(BLYNK_LUZEXTERNA,LOW);
    }
  }
}
*/
