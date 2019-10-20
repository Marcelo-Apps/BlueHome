/*
 * BLUE HOME - V1.0
 * 
 * PROCESSADOR DE ALARME
 * ESP-32 COM 30 PINOS
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
#include <SPI.h>
#include <MFRC522.h>


// Parâmetros da Wi-Fi
char ssid[] = "WIFI-BH";
char pass[] = "BlueHome.IoT.2019";


// Chave de autenticação do Projeto no Blynk
//char auth[] = "Xk9Gxg4mK9pfkGEODlVaaqeyZdcNZLXZ";      // Automação
char auth[] = "UhhrKzVPwBwg-ByLNCthYxpNMTZgK41l";      // Alarme

// Pinos usados para outros fins
// D16 -- U2_RX
// D17 -- U2_TX
// D18 -- SPI_SCK
// D19 -- SPI_MSIO
// D23 -- SPI_MIOSI
// D05 -- Aviso Alarme--Automacao
// D04 - RESERVADO ENQUANTO FAZ COMUNICACAO SERIAL COM PC
//



#define PIN_SENSORPIR              36
#define PIN_SENSORJANELA           35
#define PIN_SENSORPORTA            34
#define PIN_ALARME                  4
#define PIN_MOTORJANELA_I1         23
/// MAIS DEFINES
/// MAIS DEFINES
/// MAIS DEFINES
/// MAIS DEFINES




#define BLYNK_LCD                  V0


#define BLYNK_JANELA              V11



#define MOTOR_PASSOS-PORTA         20
#define MOTOR_PASSOS-JANELA        25


// *VARIÁVEIS GLOBAIS*
char *_lcdLin1;
char *_lcdLin2;

bool _atualSensorLuz, _atualLuzExt, _atualLuzAutomatica;
bool _novoSensorLuz, _novoLuzExt, _novoLuzAutomatica;



void setup() {

  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // INICIALIZAÇÃO DO AMBIENTE
  pinMode(PIN_SENSORPIR, INPUT);
// CONFIGURAR MAIS PINOS
// CONFIGURAR MAIS PINOS
// CONFIGURAR MAIS PINOS
// CONFIGURAR MAIS PINOS
// CONFIGURAR MAIS PINOS

  
  // Inicializa os parâmetros e ajustes iniciais
  inicializaAjustes();

  // Inicializa o Blynk
  Blynk.begin(auth, ssid, pass);

}



void loop() {
  Blynk.run();

  verificaParamLuzAutomatica();
  verificaSensorLuz();
  atuaSensorLuz();

  delay(100);
}



void inicializaAjustes (void) {
  _atualSensorLuz=false;
  _novoSensorLuz=false;
  _atualLuzExt=false;
  _novoLuzExt=false;
  _atualLuzAutomatica=true;
  _novoLuzAutomatica=true;
  

  _lcdLin1="";
  _lcdLin2="";

  digitalWrite(PIN_LUZEXTERNA,HIGH);

}



void verificaParamLuzAutomatica (void) {
  if (_atualLuzAutomatica!=_novoLuzAutomatica) {
#ifdef DEBUG
     Serial.print("-- Mudou o Estado do Parâmetro Luz Automática");
     Serial.print(" -- NOVO estado: ");
     Serial.println(_novoLuzAutomatica);
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
void atuaSensorLuz (void) {
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
#ifdef DEBUG
        Serial.println(" -- Ligou a Luz Externa");
#endif
      } else {
        digitalWrite(PIN_LUZEXTERNA,HIGH);
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



void printLCD (char texto[]) {
  WidgetLCD lcd(V0);

  _lcdLin1=_lcdLin2;
  _lcdLin2=texto;
  lcd.clear();
  lcd.print(0,0,_lcdLin1);
  lcd.print(0,1,_lcdLin2);
}




///


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



// Apenas lê o conteúdo da Luz Automática (a atualização é no novo ciclo)
BLYNK_WRITE(BLYNK_LUZAUTOMATICA) {
  _novoLuzAutomatica=(param.asInt()!=0);
}
