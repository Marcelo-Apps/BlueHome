#include <HardwareSerial.h>

HardwareSerial RxSerial(1);
 
void setup()
{
  
  Serial.begin(115200);
  delay(100);
  Serial.println("inicializou o recebimento");
  RxSerial.begin(2400,SERIAL_8N1,16,17);   // (BR,Prot,RX,TX);
}

void loop() {
  String recebido = "";

  while(RxSerial.available()) {
    recebido = char(RxSerial.read());
    Serial.print(recebido);
  }
}
