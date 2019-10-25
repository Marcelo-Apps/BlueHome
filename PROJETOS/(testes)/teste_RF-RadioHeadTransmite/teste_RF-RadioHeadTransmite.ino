#include <RH_ASK.h>
//#include <SPI.h> 
 
// Inicializa
RH_ASK driver(2000,16,17,1,false);

 
void setup()
{
  Serial.begin(115200);
  
  if (!driver.init())
    Serial.println("falha na inicialização do driver");
}

void loop() {
  const char *txt = "Teste de envio...";
   
  driver.send((uint8_t *)txt, strlen(txt));

  driver.waitPacketSent();
  Serial.print("enviou: ");
  Serial.println(txt);
  delay(2000);
}
