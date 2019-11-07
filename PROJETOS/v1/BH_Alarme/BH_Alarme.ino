/*
 * BLUE HOME - V1.0
 * 
 * PROCESSADOR DE ALARME
 * ESP-32 COM 30 PINOS
 * 
 * Autores: Marcelo Costa / Felipe Maia / Faberson Perfeito / Newton Dore
 * 
 * NOTA: ESTE PROCESSADOR PRECISA INICIALIZAR ANTES DO DE AUTOMAÇÃO, POIS ELE RECEBE CÓDIGOS DO OUTRO.
 * 
 * OBS: Ainterface é feita pelo Blynk. Para maiores informações consultar o PDF que acompanha o projeto
 */


// Se estiver definido não inicializa/processa o BLYNK
//#define SEMBLYNK


// Se é depuração (gera conteudo na serial para análise)
#define DEBUG

//#define MOSTRADEBUGLCD


#ifdef DEBUG  
  #define BLYNK_PRINT Serial
#endif


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Stepper.h>


// Parâmetros da Wi-Fi
//char ssid[] = "WIFI-BH";
//char pass[] = "BlueHome.IoT.2019";

char ssid[] = "iMordor";
char pass[] = "MyMordor2019!";



// Chave de autenticação do Projeto no Blynk
//char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme


// Pinos usados para outros fins
// D16 -- U2_RX
// D17 -- U2_TX
//


#define PIN_SENSORPIR              36
#define PIN_SENSORJANELA           35
#define PIN_SENSORPORTA            34
#define PIN_RELEALARME              4

#define PIN_MOTORJANELA_I1         27
#define PIN_MOTORJANELA_I2         14
#define PIN_MOTORJANELA_I3         12
#define PIN_MOTORJANELA_I4         13

#define PIN_MOTORPORTA_I1          32
#define PIN_MOTORPORTA_I2          33
#define PIN_MOTORPORTA_I3          25
#define PIN_MOTORPORTA_I4          26

#define PIN_BUZZER                 15     // Buzzer na Protoboard

// Mensagens da Automacão para Alarme (RF)
#define MSGALARME_SEMACAO           0
#define MSGALARME_DESATIVA          1
#define MSGALARME_ATIVA             2
#define MSGALARME_ATIVAPASSIVO      3
#define MSGALARME_TOCAR             4
#define MSGALARME_PARATOCAR         5
#define MSGALARME_ABREPORTA         6
#define MSGALARME_FECHAPORTA        7
#define MSGALARME_ABREJANELA        8
#define MSGALARME_FECHAJANELA       9

#define ALARME_CONFIGINATIVO        1
#define ALARME_CONFIGATIVO          2
#define ALARME_CONFIGPASSIVO        3

// Pinos Virtuais e Definições do Blynk
#define BLYNK_LCD                  V0
#define BLYNK_SIRENETOCANDO        V6
#define BLYNK_ALARMECONFIG         V5
#define BLYNK_JANELADIRECAO       V11
#define BLYNK_PORTADIRECAO        V12

#define MOTORJANELA_VELOCIDADE    200
#define MOTORJANELA_PASSOSVOLTA    20
#define MOTORJANELA_PASSOSVEZ      20
#define MOTORJANELA_POSFECHADA      0    
#define MOTORJANELA_POSABERTA      75

#define MOTORPORTA_VELOCIDADE     200
#define MOTORPORTA_PASSOSVOLTA     20 
#define MOTORPORTA_PASSOSVEZ       30
#define MOTORPORTA_POSFECHADA      70
#define MOTORPORTA_POSABERTA        0

#define MOTOR_DIRECAOABRIRPOR      -1
#define MOTOR_DIRECAOFECHARPOR      1
#define MOTOR_DIRECAOABRIRJAN       1
#define MOTOR_DIRECAOFECHARJAN     -1

#define ALARME_TEMPOESPERA      10000             // Espera 10s para tocar de novo se estiver armado...

#define BEEP_NENHUM                 0
#define BEEP_BOOT                   1
#define BEEP_ALARME_INATIVO         2
#define BEEP_ALARME_ATIVO           3
#define BEEP_ALARME_PASSIVO         4

#define RADIO_TAMBUFFER            32



// ** CONSTANTES **
const byte addrRadio[6] = "23232";      // Endereço de comunicação do rádio



// ** VARIÁVEIS GLOBAIS **

String _lcdLin1, _lcdLin2;

int _janelaPosicao, _atualJanelaDirecao, _novoJanelaDirecao, _portaPosicao, _atualPortaDirecao, _novoPortaDirecao;
int _atualAlarmeConfig, _novoAlarmeConfig;
bool _atualSireneTocando, _novoSireneTocando, _atualSensorPir, _novoSensorPir;
bool _isPortaFechada, _isJanelaFechada, _isPortaMovendo, _isJanelaMovendo;
 
unsigned long _sireneInativaPorSensorAte;      // A sirene não pode ser acionada antes deste tempo por conta de Sensor



// ** CLASSES **

// Inicializa os Motores de Passo
Stepper motorJanela(MOTORJANELA_PASSOSVOLTA, PIN_MOTORJANELA_I1, PIN_MOTORJANELA_I3, PIN_MOTORJANELA_I2, PIN_MOTORJANELA_I4);
Stepper motorPorta(MOTORPORTA_PASSOSVOLTA, PIN_MOTORPORTA_I1, PIN_MOTORPORTA_I3, PIN_MOTORPORTA_I2, PIN_MOTORPORTA_I4);

// Inicializa o Rádio
RF24 radio(17,16);



// Inicialização do Ambiente
void setup() {
  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif
  // INICIALIZAÇÃO DO AMBIENTE
  pinMode(PIN_SENSORPIR,INPUT);
  pinMode(PIN_SENSORJANELA,INPUT);
  pinMode(PIN_SENSORPORTA,INPUT);
  pinMode(PIN_RELEALARME,OUTPUT);
  pinMode(PIN_BUZZER,OUTPUT);

#ifdef DEBUG
  Serial.println("MÓDULO BLUEHOME - ALARME");
  Serial.println("========================");
  Serial.println("");
#endif

  // Inicializa os parâmetros e ajustes iniciais
  inicializaContexto();
  
  // Inicializa o Rádio, define o canal e configura apenas para Recepção
  radio.begin();
  radio.openReadingPipe(0,addrRadio);
  radio.startListening();
  
#ifdef DEBUG
  Serial.println("Rádio inicializado para recepção");
#endif  
  // Inicializa os parâmetros e ajustes iniciais
  inicializaContexto();

#ifndef SEMBLYNK
  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);
#endif

  // Toca o Bipe de Boot
  beepAsyncPlay(BEEP_BOOT);


#ifdef DEBUG
  Serial.println("** OPERAÇÃO DO PROCESSADOR DE ALARME INICIADA**");
#endif
}



// Rotina Principal
void loop() {
#ifndef SEMBLYNK
    Blynk.run();
#endif

  verificaStatusSensoresPortaJanela ();
  processaMsgAutomacao();
  
  verificaAlarmeConfig();
  atuaAlarmeConfig();

  beepAsyncProcessa();
  
  verificaSensorPir();
  atuaSensorPir();
  
  verificaSireneTocando();
  atuaSireneTocando();

  verificaJanela();
  atuaJanela();
  verificaPorta();
  atuaPorta();

  delay(5);
}



// Inicializa as variáveis e alguns ajustes (parte do SETUP... Está aqui para ficar mais claro o código)
void inicializaContexto (void) {
  // Inicializa Portas Necessárias
  digitalWrite(PIN_RELEALARME,HIGH);

    // Inicializa as variáveis do Beep Assíncrono
  beepAsyncInicializa();

  // Inicializa as Variáveis
  _janelaPosicao=MOTORJANELA_POSFECHADA;
  _atualJanelaDirecao=MOTOR_DIRECAOFECHARJAN;
  _novoJanelaDirecao=MOTOR_DIRECAOFECHARJAN;
  _portaPosicao=MOTORPORTA_POSFECHADA;
  _atualPortaDirecao=MOTOR_DIRECAOFECHARPOR;
  _novoPortaDirecao=MOTOR_DIRECAOFECHARPOR;
  _atualSireneTocando=false;
  _novoSireneTocando=false;
  _novoAlarmeConfig=ALARME_CONFIGINATIVO;
  _atualAlarmeConfig=ALARME_CONFIGINATIVO;
  _atualSensorPir=false;
  _novoSensorPir=false;
  _isPortaFechada=false;
  _isJanelaFechada=false;
  _isPortaMovendo=false;
  _isJanelaMovendo=false;  
  _sireneInativaPorSensorAte=0;
  _lcdLin1="";
  _lcdLin2="";

  motorJanela.setSpeed(MOTORJANELA_VELOCIDADE);
  motorPorta.setSpeed(MOTORPORTA_VELOCIDADE);  
}




// USO INTERNO: Retorna HIGH ou LOW dependendo do valor passado (função ord não existe aqui)
int _BoolToEstado (bool valor) {
  if (valor)
    return HIGH;
  else
    return LOW;
}




// USO INTERNO: retorna o texto da direção em gerúndo (executando)
String _getStrDirecao (int direcao) {
  if (direcao<0)
    return "Abrindo";
  else
    return "Fechando";
}




// USO INTERNO: retorna o texto da direção após término (executrado)
String _getStrDirecaoFim (int direcao) {
  if (direcao<0)
    return "Aberta";
  else
    return "Fechada";
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




// Processa Mensagens do Processador de Automação
// Lê 20 caracterers e verifica se tem a Mensagem no formato  #x:#. Se conseguir ler algo, ajusta os parâmetros
void processaMsgAutomacao () {
  if (radio.available())
  {
    char bufRec[RADIO_TAMBUFFER]={0};
    byte comando;
    String txtLCD = "";
    
    radio.read(&bufRec,sizeof(bufRec));

#ifdef DEBUG
    Serial.println("--> Mensagem da Automação: "+String(bufRec));
#endif

    if ((bufRec[0]==':') && (bufRec[1]==':') && (bufRec[3]==':') && (bufRec[4]==':')) {
      comando=byte(bufRec[2])-48;

#ifdef DEBUG
      Serial.println("  --> Comando: "+String(comando));
//      printLCD("Cmd_Al-Au->"+String(comando));
#endif
      switch (comando) {
        case MSGALARME_DESATIVA      :  _novoAlarmeConfig=ALARME_CONFIGINATIVO;
                                        _novoSireneTocando=false;
                                        txtLCD="Cmd: Alarm.Des";
                                        break;

        case MSGALARME_ATIVA         :  _novoAlarmeConfig=ALARME_CONFIGATIVO;
                                        txtLCD="Cmd: Alarm.Atv";
                                        break;

        case MSGALARME_ATIVAPASSIVO  :  _novoAlarmeConfig=ALARME_CONFIGPASSIVO;
                                        txtLCD="Cmd: Alarm.Pas";
                                        break;
                                          
        case MSGALARME_TOCAR         :  _novoSireneTocando=true;
                                        txtLCD="Cmd: Sirene Lig";
                                        break;

        case MSGALARME_PARATOCAR     :  _novoSireneTocando=false;
                                        txtLCD="Cmd: Sirene Des";
                                        break;

        case MSGALARME_ABREPORTA     :  _novoPortaDirecao=MOTOR_DIRECAOABRIRPOR;
                                        txtLCD="Cmd: Porta Abre";
                                        break;

        case MSGALARME_FECHAPORTA    :  _novoPortaDirecao=MOTOR_DIRECAOFECHARPOR;
                                        txtLCD="Cmd: Porta Fecha";
                                        break;

        case MSGALARME_ABREJANELA    :  _novoJanelaDirecao=MOTOR_DIRECAOABRIRJAN;
                                        txtLCD="Cmd: Janela Abre";
                                        break;

        case MSGALARME_FECHAJANELA   :  _novoJanelaDirecao=MOTOR_DIRECAOFECHARJAN;
                                        txtLCD="Cmd: Janela Fech";
                                        break;
      }
#ifdef MOSTRADEBUGLCD
      if (txtLCD!="") printLCD(txtLCD);
#endif
    }
  }
}




// Verifica a configuração do Alarme
void verificaAlarmeConfig () {
//
}




// Atua na Configuração do Alarme (se necessário)
void atuaAlarmeConfig () {
  
  if (_atualAlarmeConfig!=_novoAlarmeConfig) {

    // Se ativará o alarme agora (ativo ou passivo), registra o tempo em que os sensores Não Dispararão a Sirene (Tempo de Espera)
    if (_atualAlarmeConfig==ALARME_CONFIGINATIVO) {
      _sireneInativaPorSensorAte=millis()+ALARME_TEMPOESPERA;
    }
    
    _atualAlarmeConfig=_novoAlarmeConfig;

    switch (_atualAlarmeConfig) {
      case ALARME_CONFIGINATIVO  :  beepAsyncPlay(BEEP_ALARME_INATIVO);
                                    printLCD("Alarme Desligado");
                                    _novoSireneTocando=false;
                                    break;
      case ALARME_CONFIGATIVO    :  beepAsyncPlay(BEEP_ALARME_ATIVO);
                                    printLCD("Alarme ATIVO");
                                    break;
      default :
        beepAsyncPlay(BEEP_ALARME_PASSIVO);
        _novoSireneTocando=false;
        printLCD("Alarme PASSIVO");
    }
    // Marca o tempo de espera 
    
    Blynk.virtualWrite(BLYNK_ALARMECONFIG,_atualAlarmeConfig);
#ifdef DEBUG
    Serial.println("-- Mudou a Configuração do Alarme: "+String(_atualAlarmeConfig));
#endif
  }
}  




// Verifica/Valida a condição do alarme tocando se ela mudou
void verificaSireneTocando () {
  // Premissa: Se estiver em Modo Passivo, NUNCA pode deixar o alarme tocar (se estiver desligado, 3 RFIDs inválidos dispara)
  if ((_atualAlarmeConfig==ALARME_CONFIGPASSIVO) && (_novoSireneTocando)) {
    _novoSireneTocando=false;
    // Precisa avisar o Blynk, pois a mudança de estado pode ter ocorrido por lá
    Blynk.virtualWrite(BLYNK_SIRENETOCANDO,LOW);
    // APENAS Avisa no LCD que houve violação do Alarme
    printLCD("ALARM.SILENCIOSO");
  }
}




// Atua na Sirente do Alarme (se necessário)
void atuaSireneTocando () {
  if (_atualSireneTocando!=_novoSireneTocando) {
    _atualSireneTocando=_novoSireneTocando;

    Blynk.virtualWrite(BLYNK_SIRENETOCANDO,_BoolToEstado(_atualSireneTocando));
    digitalWrite(PIN_RELEALARME,!_BoolToEstado(_atualSireneTocando));

    if (_atualSireneTocando) 
      printLCD("*SIRENE ATIVA*");
    else {
      printLCD("*Sirene Inativa*");
      // Não permite que a sirene 'arme de novo'por um certo momento...
      _sireneInativaPorSensorAte=millis()+ALARME_TEMPOESPERA;
    }
   
#ifdef DEBUG
    Serial.println("-- Mudou o Status de Sirene Tocando: "+String(_atualSireneTocando));
#endif
  }
}




// Verifica o Sensor PIR. (Premissa: Apenas se NÃO ESTIVER INATIVO
void verificaSensorPir () {
  if (_atualAlarmeConfig!=ALARME_CONFIGINATIVO) {
    bool lido = ((digitalRead(PIN_SENSORPIR)==HIGH));
    
    if (lido!=_novoSensorPir) {
      _novoSensorPir=lido;
  #ifdef DEBUG
      Serial.println("- Sensor PIR Mudou de Estado. Ficou = "+String(_novoSensorPir));
  #endif
    }
  }
}




void atuaSensorPir () {
  if (_atualSensorPir!=_novoSensorPir) {
    _atualSensorPir=_novoSensorPir;

    if (_atualSensorPir) {
      printLCD("*INVASÃO - PIR*");

      // Não toca a sirene se estiver em espera
      if (_sireneInativaPorSensorAte > millis()) {
        printLCD("Sirene em Espera");
      } else {
        _novoSireneTocando=true;
      }
    }
  }
}




// Verifica se mudou de "direção" o movimento da Janela e ajusta o ambiente se necessário 
void verificaJanela () {
  if (_atualJanelaDirecao!=_novoJanelaDirecao) {
    _atualJanelaDirecao=_novoJanelaDirecao;

    printLCD("Janela "+_getStrDirecao(-_atualJanelaDirecao));
    _isJanelaMovendo=true;
    
    Blynk.virtualWrite(BLYNK_JANELADIRECAO,_atualJanelaDirecao);
#ifdef DEBUG
    Serial.println("-- Mudou a Direcao da Janela: "+_getStrDirecao(-_atualJanelaDirecao));
#endif
  }
}



void atuaJanela () {
  if (_isJanelaMovendo) {
    if (_atualJanelaDirecao>0) {
      if (_janelaPosicao<MOTORJANELA_POSABERTA) {
        _janelaPosicao++;
        motorJanela.step(MOTORJANELA_PASSOSVEZ);
      }
      
      // Se chegou ao fim, para...
      if (_janelaPosicao>=MOTORJANELA_POSABERTA) {
        _isJanelaMovendo=false;
      }
    } else {
      if (_janelaPosicao>MOTORJANELA_POSFECHADA) {
        _janelaPosicao--;
        motorJanela.step(-MOTORJANELA_PASSOSVEZ);
      }

      // Se chegou ao fim, para...
      if (_janelaPosicao<=MOTORJANELA_POSFECHADA) {
        _isJanelaMovendo=false;
      }
    }
#ifdef DEBUG
    Serial.println("  - Moveu Janela. Posicao Atual = "+String(_janelaPosicao));
#endif
    // Se não está mais movendo, acabou...
    
    if (!_isJanelaMovendo) {
      printLCD("Janela "+_getStrDirecaoFim(-_atualJanelaDirecao));
#ifdef DEBUG
      Serial.println("--Janela OK. Status = "+_getStrDirecaoFim(-_atualJanelaDirecao));
#endif
    }
  }
}




// Verifica se mudou de "direção" o movimento da Porta e ajusta o ambiente se necessário 
void verificaPorta () {
  if (_atualPortaDirecao!=_novoPortaDirecao) {
    _atualPortaDirecao=_novoPortaDirecao;

    printLCD("Porta "+_getStrDirecao(_atualPortaDirecao));
    _isPortaMovendo=true;
    
    Blynk.virtualWrite(BLYNK_PORTADIRECAO,_atualPortaDirecao);
//    printLCD("Porta "+_getStrDirecao(_atualPortaDirecao));
#ifdef DEBUG
    Serial.println("-- Mudou a Direcao da Porta: "+_getStrDirecao(_atualPortaDirecao));
#endif
  }
}



void atuaPorta () {
  if (_isPortaMovendo) {
    if (_atualPortaDirecao>0) {
      if (_portaPosicao<MOTORPORTA_POSFECHADA) {
        _portaPosicao++;
        motorPorta.step(MOTORPORTA_PASSOSVEZ);
      }
      
      // Se chegou ao fim, para...
      if (_portaPosicao>=MOTORPORTA_POSFECHADA) {
        _isPortaMovendo=false;
      }
    } else {
      if (_portaPosicao>MOTORPORTA_POSABERTA) {
        _portaPosicao--;
        motorPorta.step(-MOTORPORTA_PASSOSVEZ);
      }

      // Se chegou ao fim, para...
      if (_portaPosicao<=MOTORPORTA_POSABERTA) {
        _isPortaMovendo=false;
      }
    }
#ifdef DEBUG
    Serial.println("  - Moveu Porta. Posicao Atual = "+String(_portaPosicao));
#endif
    // Se não está mais movendo, acabou...
    
    if (!_isPortaMovendo) {
      printLCD("Porta "+_getStrDirecaoFim(_atualPortaDirecao));
#ifdef DEBUG
      Serial.println("--Porta OK. Status = "+_getStrDirecaoFim(_atualPortaDirecao));
#endif
    }
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
#define BEEPSCNT_BOOT              5
int __beepAsync_ArrayBoot[BEEPSCNT_BOOT] = {200,20,100,20,200};

#define BEEPSCNT_ALARMEINATIVO     3
int __beepAsync_ArrayAlarmeInativo[BEEPSCNT_ALARMEINATIVO] = {100,30,200};

#define BEEPSCNT_ALARMEATIVO       5
int __beepAsync_ArrayAlarmeAtivo[BEEPSCNT_ALARMEATIVO] = {300,50,300,50,500};

#define BEEPSCNT_ALARMEPASSIVO     5
int __beepAsync_ArrayAlarmePassivo[BEEPSCNT_ALARMEPASSIVO] = {150,50,150,50,200};



// Toca o Bipe selecionado
void beepAsyncPlay (int tipoBeep) {
  switch (tipoBeep) {
    case BEEP_BOOT   :         __beepAsyncTocaSeq(&__beepAsync_ArrayBoot[0],BEEPSCNT_BOOT);
                               break;

    case BEEP_ALARME_INATIVO : __beepAsyncTocaSeq(&__beepAsync_ArrayAlarmeInativo[0],BEEPSCNT_ALARMEINATIVO);
                               break;

    case BEEP_ALARME_ATIVO :   __beepAsyncTocaSeq(&__beepAsync_ArrayAlarmeAtivo[0],BEEPSCNT_ALARMEATIVO);
                               break;

    case BEEP_ALARME_PASSIVO : __beepAsyncTocaSeq(&__beepAsync_ArrayAlarmePassivo[0],BEEPSCNT_ALARMEPASSIVO);
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
  printLCD("BH-ALARME OK");
//  printLCD("");
  Blynk.virtualWrite(BLYNK_SIRENETOCANDO,_BoolToEstado(_atualSireneTocando));
  Blynk.virtualWrite(BLYNK_ALARMECONFIG,_atualAlarmeConfig);
  Blynk.virtualWrite(BLYNK_JANELADIRECAO,_atualJanelaDirecao);
  Blynk.virtualWrite(BLYNK_PORTADIRECAO,_atualPortaDirecao);
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
BLYNK_WRITE(BLYNK_SIRENETOCANDO) {
  _novoSireneTocando=(param.asInt()==HIGH);
}




// Armazena o conteúdo da Nova "Configuração do Alarme" de acordo com o Blynk
BLYNK_WRITE(BLYNK_ALARMECONFIG) {
  _novoAlarmeConfig=param.asInt();
}
