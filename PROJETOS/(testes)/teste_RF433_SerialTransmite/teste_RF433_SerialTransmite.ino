#include <HardwareSerial.h>

HardwareSerial TxSerial(1);
 
void setup()
{
  Serial.begin(115200);
  TxSerial.begin(2400,SERIAL_8N1,16,17);   // (BR,Prot,RX,TX);
}

void loop() {
  TxSerial.flush();
  TxSerial.println("Mensagem "+String(millis()/1000));
  Serial.println("enviou...");
  delay(1000);
}
