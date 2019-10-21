

#define DEBUG

#define LED_RED 2
#define LED_GREEN 4
#define LED_BLUE 16

#define PWM_RED 0
#define PWM_GREEN 1
#define PWM_BLUE 2

#define PWM_FREQUENCIA 1000
#define PWM_BITSRESOLUCAO 8
#define PWW_LIMITE 256

#define TOUCH_MINVALOROK     2    // Menor Valor quando Touch está acionado
#define TOUCH_CHUVA         T7    // Touch 7 --> GPIO 


#define PIN_SENSORPIR       17



int touchCnt;
bool touchStatusLido, touchStatusExec;


// the setup function runs once when you press reset or power the board
void setup() {
  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  ledcAttachPin(LED_RED,PWM_RED);
  ledcAttachPin(LED_GREEN,PWM_GREEN);
  ledcAttachPin(LED_BLUE,PWM_BLUE);
  ledcSetup(PWM_RED,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_GREEN,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);
  ledcSetup(PWM_BLUE,PWM_FREQUENCIA,PWM_BITSRESOLUCAO);

  pinMode(PIN_SENSORPIR,INPUT);

  touchCnt=0;
  touchStatusExec=0;
  touchStatusLido=0; 
}



// the loop function runs over and over again forever
void loop() {
  ledcWrite(PWM_RED,random(PWW_LIMITE));
  ledcWrite(PWM_GREEN,random(PWW_LIMITE));
  ledcWrite(PWM_BLUE,random(PWW_LIMITE));
  delay(60);                       // wait for a second

  bool touchStatusAgora=(touchRead(TOUCH_CHUVA)<=TOUCH_MINVALOROK);
/*
  if (touchStatusAgora!=touchStatusExec)
  {
    if (touchStatusAgora!=touchStausLido
  }

*/  
  
#ifdef DEBUG
//  Serial.println(valTouchChuva);
  Serial.print("Valor PIR: ");
  Serial.println(digitalRead(PIN_SENSORPIR));
#endif

  if (touchStatusAgora) {
#ifdef DEBUG
  Serial.println("Chuva Desligada");
#endif
  //  digitalWrite(PIN_LEDCHUVA,LOW);
  } else
  {
#ifdef DEBUG
  Serial.println("Chuva LIGADA");
#endif
   // digitalWrite(PIN_LEDCHUVA,HIGH);
  }

}03
.
