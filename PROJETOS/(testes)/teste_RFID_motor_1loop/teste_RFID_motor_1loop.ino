#include <SPI.h>
#include <MFRC522.h>
#include <Stepper.h> //INCLUSÃO DE BIBLIOTECA

#define SS_PIN 21
#define RST_PIN 22
 
const int stepsPerRevolution = 30; //NÚMERO DE PASSOS ABRIR E FECHAR PORTA
boolean portaAbrindoFechando = false;
boolean btn = false;
int passoPortaAtual;
int tempoPortaAberta = 5;
 
Stepper myStepper(stepsPerRevolution, 2,12,14,13); //INICIALIZA O MOTOR
MFRC522 mfrc522(SS_PIN, RST_PIN); // INICIALIZA RFID

void setup() {
  pinMode(25, OUTPUT); //LEDS
  //pinMode(33, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(34, INPUT);
  
  myStepper.setSpeed(300); //VELOCIDADE DO MOTOR

  Serial.begin(9600);
  SPI.begin();     
  mfrc522.PCD_Init(); 
  Serial.println("Leitor de RFID:");
}
void loop() {
  
  String conteudo = "";
  digitalWrite(25, 0);
  digitalWrite(32, 0);
  //digitalWrite(33, 0);

  if ( mfrc522.PICC_IsNewCardPresent() && !portaAbrindoFechando)
    {
        if ( mfrc522.PICC_ReadCardSerial())
        {
           Serial.print("Tag RFID n°");
           for (byte i = 0; i < mfrc522.uid.size; i++) {
              Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
              Serial.print(mfrc522.uid.uidByte[i], HEX);
                  
              conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
              conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
            }
            conteudo.toUpperCase();
            if(conteudo.substring(1) == "17 86 7D 26"){
                 portaAbrindoFechando = true;
                 passoPortaAtual = stepsPerRevolution;
                 digitalWrite(32, 255);
                 delay(1000);
                 digitalWrite(32, 0);               
            }
            else{
               digitalWrite(25, 255);
               delay(1000);
               digitalWrite(25, 0);
            }
            Serial.println();
            mfrc522.PICC_HaltA();
        }
    }

    if(digitalRead(34) && portaAbrindoFechando){ //se detectou que a porta ja fechou ou ja abriu totalmente
         if (passoPortaAtual > 0){ //estava em processo de abrir e ja detectou que fechou, entao vai para o tempo de porta aberta
          passoPortaAtual=0;
         }else if (passoPortaAtual < 0 && passoPortaAtual > -30){ // estava em processo de fechar e ja detectou que fechou, entao reseta as variaveis
                portaAbrindoFechando = false;
                tempoPortaAberta = 5;
         }
    }
    
    if (portaAbrindoFechando){ //processo para abrir ou fehcar a porta
      if (passoPortaAtual > 0){ //porta abrindo
         myStepper.step(stepsPerRevolution); //GIRA O MOTOR
          passoPortaAtual--;
          
      }else if((passoPortaAtual == 0) && (tempoPortaAberta > 0)){ //porta totalmente aberta
        tempoPortaAberta--; //tempo para porta ficar aberta
        delay(650);
        
      }else if(passoPortaAtual <= 0 && passoPortaAtual > -30){ //porta fechando
        myStepper.step(-stepsPerRevolution);
        passoPortaAtual--;
      }else{
        portaAbrindoFechando = false; // resetando variaveis pois a porta ja abriu e fechou
        tempoPortaAberta = 5;
      }
   }
}
