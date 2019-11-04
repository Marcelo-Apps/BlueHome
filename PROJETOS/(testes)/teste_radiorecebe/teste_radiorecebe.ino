

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


//create an RF24 object
RF24 radio(17, 16);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "23232";

void setup() {
  Serial.begin(115200);
  delay(100);

  
  radio.begin();
  
  //set the address
  radio.openReadingPipe(0, address);
  
  //Set module as receiver
  radio.startListening();
  
  Serial.println("inicializou o recebimento");

}

void loop() {
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
}
