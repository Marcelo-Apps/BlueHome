#include <HardwareSerial.h>

HardwareSerial RxSerial(1);
 
void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("inicializou o recebimento");
  RxSerial.begin(750.,SERIAL_6E2,16,17);   // (BR,Prot,RX,TX);
}

void loop() {
  String recebido="";

  while(RxSerial.available()) {
//    Serial.print(".");
    recebido = char(RxSerial.read());
    Serial.print(recebido);
  }
}
