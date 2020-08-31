/**
 * Teste simples da conexão da EEProm 24C32 (32Kbits = 4Kbytes)
 */

#include <Wire.h>
#include <I2C_eeprom.h>


I2C_eeprom eep(0x50, 0x1000);
 
void setup() {
  Serial.begin(115200);
  Serial.println ("Teste EEProm 24C32");
  eep.begin();
  int tamanho = eep.determineSize();
  Serial.print ("Tamanho = ");
  Serial.print (tamanho);
  Serial.println ("KB");
}

void loop() {
  delay (100);
}
