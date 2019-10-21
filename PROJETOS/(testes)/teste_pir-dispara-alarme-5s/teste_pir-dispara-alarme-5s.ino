

#define DEBUG


#define PIN_SENSORPIR       36

#define PIN_ALARME          4






// the setup function runs once when you press reset or power the board
void setup() {
  // Se DEBUG, inicializa a serial
#ifdef DEBUG
  Serial.begin(115200);
#endif  
  pinMode(PIN_SENSORPIR, INPUT);
  pinMode(PIN_ALARME, OUTPUT);

  digitalWrite(PIN_ALARME,HIGH);

}



// the loop function runs over and over again forever
void loop() {

#ifdef DEBUG
  Serial.print("Valor PIR: ");
  Serial.println(digitalRead(PIN_SENSORPIR));
#endif
 
  if (digitalRead(PIN_SENSORPIR)==HIGH) {
#ifdef DEBUG
  Serial.println("Acionou o alarme");
#endif
    digitalWrite(PIN_ALARME,LOW);
    delay(5000);
    digitalWrite(PIN_ALARME,HIGH);
#ifdef DEBUG
  Serial.println("Acionou o alarme");
#endif
  }
  delay(500);
}
