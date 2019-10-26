/*
 * BLUE HOME - V1.0
 * 
 * PROCESSADOR DE AUTOMAÇÃO
 * ESP-32 COM 30 PINOS
 * 
 * Autores: Marcelo Costa / Felipe Maia / Faberson Perfeito / Newton Dore
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
#define PIN_SENSORCHUVA            T2     // D02
#define PIN_SENSORJANELA           35
#define PIN_SENSORPORTA            34
#define PIN_LEDRED                 12
#define PIN_LEDGREEN               14
#define PIN_LEDBLUE                27
#define PIN_LUZEXTERNA             4

#define PIN_LEITORCARD_SPI_SCK     18     // Apenas Reserva - SPI trata disto
#define PIN_LEITORCARD_SPI_MSIO    19     // Apenas Reserva - SPI trata disto
#define PIN_LEITORCARD_SPI_MOSI    23     // Apenas Reserva - SPI trata disto
#define PIN_LEITORCARD_SDA         21     // RFID
#define PIN_LEITORCARD_RST         22     // RFID

// Pinos da comunicação entre Processadores
#define PIN_SERIAL_COMUNIC_RX      16
#define PIN_SERIAL_COMUNIC_TX      17

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

#define BLYNK_JANELA              V11
#define BLYNK_PORTA               V11
#define BLYNK_LUZEXTERNA          V13
#define BLYNK_LUZAUTOMATICA       V14                 
#define BLYNK_IDUSUARIO           V15
//#define BLYNK_LUZINTERNA         10

#define RFID_MAXERROS               3      // Máximo de erros do RFID antes de gerar um Alarme 

#define SERIALRF_VELOCIDADE       750      // Velocidade da Serial de RF (entre Microcontroladores)


//#define SIZE_BUFFER     18
//#define MAX_SIZE_BLOCK  16



// ** VARIÁVEIS GLOBAIS **

String _lcdLin1, _lcdLin2;


bool _atualSensorLuz, _atualLuzExt, _atualLuzAutomatica, _atualLuzManual, _atualSensorChuva;
bool _novoSensorLuz, _novoLuzExt, _novoLuzAutomatica, _novoLuzManual, _novoSensorChuva;

int _atualMorador, _novoLuzR, _novoLuzG, _novoLuzB;
int _novoMorador, _atualLuzR, _atualLuzG, _atualLuzB;

int _acaoAlarme, _numErrosRfid;

String _moradorNome;




// ** CLASSES **

// Inicializa o RFID
MFRC522 rfid(PIN_LEITORCARD_SDA,PIN_LEITORCARD_RST);

// Inicializa a serial para comunicação RF entre Automação e Alarme
HardwareSerial TxSerial(1);



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
  pinMode(PIN_LUZEXTERNA, OUTPUT);

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


#ifndef SEMBLYNK
  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);
#endif
}



void loop() {
#ifndef SEMBLYNK
    Blynk.run();
#endif

  verificaRFID();
  enviaMensagensAlarme();
  ajustaAmbienteMorador();
  ajustaLuzInterna();
  verificaParamLuzAutomatica();
  verificaSensorLuz();
  ajustaSensorLuz();

  delay(50);
}



void inicializaContexto (void) {
  _atualSensorLuz=false;
  _novoSensorLuz=false;
  _atualLuzExt=false;
  _novoLuzExt=false;
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
  _acaoAlarme=ALARME_SEMACAO;
  _numErrosRfid=0;
  
  _moradorNome="";

  _lcdLin1="";
  _lcdLin2="";

  digitalWrite(PIN_LUZEXTERNA,HIGH);

  ledcWrite(PWM_LEDRED,PWM_MIN);
  ledcWrite(PWM_LEDGREEN,PWM_MIN);
  ledcWrite(PWM_LEDBLUE,PWM_MIN);
}



// A rotina 
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
//      printLCD("OK: "+chave);
#ifdef DEBUG
      Serial.print(" -- RFID Autorizado: Usuário: ");
      Serial.println(_novoMorador);
#endif
    } else {
      _numErrosRfid++;
       printLCD("ERR: "+chave);
       printLCD("Tag Invalido ("+String(_numErrosRfid)+")");
#ifdef DEBUG
      Serial.print(" -- RFID *INVÁLIDO* -- TENTATIVAS COM ERRO: ");
      Serial.println(_numErrosRfid);
#endif
      if (_numErrosRfid==RFID_MAXERROS) {
        _acaoAlarme=ALARME_TOCAR;
       printLCD("**Alarme Acionado**");
#ifdef DEBUG
      Serial.println(" -- *** Excedeu Número de Acessos Inválidos - VAI DISPARAR O ALARME ***");
#endif
      }
    }
  }
}



void enviaMensagensAlarme () {
  if (_acaoAlarme!=0) {
      String texto = "...........#:"+String(_acaoAlarme)+":# ";

      TxSerial.flush();
      TxSerial.print(texto);
#ifdef DEBUG
      Serial.println("-- Enviada Mensagem para Processador de Alarme via RF: "+texto);
#endif
  // Avisa que não tem mais Ações de Alarme
  _acaoAlarme=0;
  }
}



void ajustaAmbienteMorador () {
  if ((_novoMorador!=0) && (_atualMorador!=_novoMorador)) {
    
#ifdef DEBUG
    Serial.println("--Mudou de Morador. Era: "+String(_atualMorador)+", e agora é: "+String(_novoMorador));
#endif
     _atualMorador=_novoMorador;
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
  if ((_atualLuzR!=_novoLuzR) || (_atualLuzG!=_novoLuzG) || (_atualLuzB!=_novoLuzB)) {
    // Ajusta a cor...
    _atualLuzR=_novoLuzR;
    _atualLuzG=_novoLuzG;
    _atualLuzB=_novoLuzB;
    
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
        digitalWrite(PIN_LUZEXTERNA,LOW);
        Blynk.virtualWrite(BLYNK_LUZEXTERNA,LOW);
        printLCD("Luz.Ext. Ligada");
        _atualLuzManual=true;
        _novoLuzManual=true;
#ifdef DEBUG
        Serial.println(" -- Ligou a Luz Externa");
#endif
      } else {
        digitalWrite(PIN_LUZEXTERNA,HIGH);
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
        digitalWrite(PIN_LUZEXTERNA,LOW);
    
  }
}


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



// Redefine o ambiente quando conectado.
BLYNK_CONNECTED () {
  // Manda o texto para o LCD
  printLCD("BH-AUTOMACAO OK");
//  printLCD("");
}




// Mudança da Luz - Vermelha (R)
BLYNK_WRITE(BLYNK_LUZINT_R) {
  _novoLuzR=param.asInt();
#ifdef DEBUG
  Serial.println(" - Cor: Novo R: "+String(_novoLuzR));
#endif  
}



// Mudança da Luz - Verde (G)
BLYNK_WRITE(BLYNK_LUZINT_G) {
  _novoLuzG=param.asInt();
#ifdef DEBUG
  Serial.println(" - Cor: Novo G: "+String(_novoLuzG));
#endif  
}



// Mudança da Luz - Azul (B)
BLYNK_WRITE(BLYNK_LUZINT_B) {
  _novoLuzB=param.asInt();
#ifdef DEBUG
  Serial.println(" - Cor: Novo B: "+String(_novoLuzB));
#endif    
}



/*

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


*/
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
