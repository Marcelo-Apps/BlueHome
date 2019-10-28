
// Rotina de testes do motor de passsos e sirene (beep)

// Se é depuração (gera conteudo na serial para análise)
#define DEBUG


//#include <SPI.h>
#include <Stepper.h>



#define PIN_SENSORPIR              36
#define PIN_ALARME                  4

#define PIN_SENSORJANELA           34

#define PIN_CMDABRIRJANELA         36
#define PIN_CMDFECHARJANELA        39

#define PIN_MOTORJANELA_I1         27
#define PIN_MOTORJANELA_I2         14
#define PIN_MOTORJANELA_I3         12
#define PIN_MOTORJANELA_I4         13

#define PIN_BEEP                   32







int _passoJanelaAtual; //passo atual da porta

const int _passosPorVoltaJanela = 10; //NÚMERO DE PASSOS POR VOLTA DO MOTOR DA JANELA

Stepper motorJanela(_passosPorVoltaJanela, PIN_MOTORJANELA_I1, PIN_MOTORJANELA_I3, PIN_MOTORJANELA_I2, PIN_MOTORJANELA_I4); //INICIALIZA O MOTOR JANELA





#define BEEPS_TOTAL       6
int _beepTempos[BEEPS_TOTAL] = {2000,500,2000,500,2000,100}; 
int _beepPos=0;
bool _beepTocando;
unsigned long _beepLimite;


void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  motorJanela.setSpeed(100); //VELOCIDADE DO MOTOR JANELA


  // INICIALIZAÇÃO DO AMBIENTE
  pinMode(PIN_SENSORPIR, INPUT);
  pinMode(PIN_ALARME, OUTPUT);

  pinMode(PIN_SENSORJANELA, INPUT);


  pinMode(PIN_BEEP, OUTPUT);
  
  pinMode(PIN_CMDABRIRJANELA, INPUT);
  pinMode(PIN_CMDFECHARJANELA, INPUT);

}







void loop() {
    cmdAbrirJanela();
    cmdFecharJanela();
    atuaNaJanela();
    verificaSensorJanela();
    atuaBeep();

    delay(10);
    _beepPos=-1;
    _beepMudaEstado();
    _beepTocando=false;
    digitalWrite(PIN_BEEP,LOW);
}



void atuaBeep () {
  if (_beepPos>0) {
    if (millis()>_beepLimite) {
      _beepMudaEstado();
    }
  }
}



void _beepMudaEstado () {
  int valor;
 
  if (_beepPos>0) _beepPos--;

  if (_beepPos<0) {
    _beepTocando=false;
    _beepLimite=0;
  } else {
    _beepTocando=!_beepTocando;
    _beepLimite=millis()+_beepTempos[_beepPos];
  }
  
  if (_beepTocando) {
    digitalWrite(PIN_BEEP,HIGH);
    Serial.println("Beep Tocando");
  } else {
    digitalWrite(PIN_BEEP,LOW);
    Serial.println("Beep Sem Som");
  }
}



void _beepInicia () {
  if (_beepPos<0) {
    Serial.println("Iniciou Beep");
    _beepTocando=true;
    _beepPos=BEEPS_TOTAL;
    _beepMudaEstado();
  }
   
}




void cmdAbrirJanela(void) {
  if (digitalRead(PIN_CMDABRIRJANELA)==LOW) {
    _passoJanelaAtual = 30;

    _beepInicia();
 
    Serial.println("Mandou Abrir Janela");

    
  }
}


void cmdFecharJanela(void) {
  if (digitalRead(PIN_CMDFECHARJANELA)==LOW) {
    _passoJanelaAtual = -1;

    Serial.println("Mandou FECHAR janela");
  }
}


// Lê o sensor da janela se ja fechou ou abriu totalmente
void verificaSensorJanela (void) {
  if (digitalRead(PIN_SENSORJANELA)==LOW) { //se detectou que a janela ja fechou ou ja abriu totalmente
    _passoJanelaAtual = 0; //para a janela

    Serial.println("****PARA JANELA***");
    
  }
}


void atuaNaJanela(void) {

  if (_passoJanelaAtual > 0) { //janela abrindo
    motorJanela.step(_passosPorVoltaJanela);
    _passoJanelaAtual--;
    // quando a variavel _passoJanelaAtual chegar em 0, para a janela

  } else if (_passoJanelaAtual < 0 && _passoJanelaAtual >= -30) { //janela fechando
    motorJanela.step(-_passosPorVoltaJanela);
    _passoJanelaAtual--;
  }// quando a variavel _passoJanelaAtual chegar em -31, para a janela

  Serial.print("Passo: : ");
  Serial.println(_passoJanelaAtual);
}
