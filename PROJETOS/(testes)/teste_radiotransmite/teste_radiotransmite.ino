

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


//create an RF24 object
RF24 radio(17, 16);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "23232
";

void setup() {
  Serial.begin(115200);
  delay(100);

  
  radio.begin();
  
  //set the address
  radio.openWritingPipe(address);
  
  //Set module as transmitter
  radio.stopListening();
  Serial.println("inicializou o envio");

}

void loop() {
  //Send message to receiver
  const char text[] = "Hello World";
  radio.write(&text, sizeof(text));
  Serial.print("Mandou");
  
  delay(1000);
}
