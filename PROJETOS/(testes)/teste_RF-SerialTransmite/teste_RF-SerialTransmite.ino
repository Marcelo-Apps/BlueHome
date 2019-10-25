#include <HardwareSerial.h>

HardwareSerial TxSerial(1);

int Contador = 0;
 
void setup()
{
  Serial.begin(115200);
  TxSerial.begin(750,SERIAL_6E2,16,17);   // (BR,Prot,RX,TX);


}

void loop() {
  Contador++;
  TxSerial.flush();
  String Texto = " #:"+String(Contador)+"# ";
  TxSerial.print(".......... 1"+Texto);
  TxSerial.print(" - 2"+Texto);
//  TxSerial.print(" - 3"+Texto);
  TxSerial.println("/");
  Serial.println("enviou...");
  delay(600);
}
