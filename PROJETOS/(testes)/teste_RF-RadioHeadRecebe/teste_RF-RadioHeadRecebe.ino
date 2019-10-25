#include <RH_ASK.h>
#include <SPI.h> 
 
// Inicializa
RH_ASK driver(2000,16,17,1,false);

 
void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.print(".");
  Serial.println("inicializando");
  Serial.println("");

  if (!driver.init())
    Serial.println("falha na inicialização do driver");

    
}

void loop() {
  uint8_t mensagem[32];
  uint8_t lenmsg = sizeof(mensagem);

  
  if (driver.recv(mensagem, &lenmsg))
  {
    Serial.print(".");
    Serial.print("Caracteres Recebidos: ");
    Serial.print(lenmsg);
    Serial.println((char*)mensagem);
  }
}
