/*
 * ATmegaDetonator - Funções para interagir com o PCF8574A
 * 
 * (C) 2020-2021, Daniel Quadros
 */

// Escreve um byte
void PCF8574_Write(byte dado) {
  Wire.beginTransmission(PCF8574_Addr);
  Wire.write(dado);
  Wire.endTransmission();
}

// Prepara para leitura
// Somente os pinos colocados em HIGH podem
// ser usados para entrada
void PCF8574_Release() {
  Wire.beginTransmission(PCF8574_Addr);
  Wire.write(0xFF);
  Wire.endTransmission();
}

// Lê um byte
byte PCF8574_Read(){
  Wire.requestFrom(PCF8574_Addr, 1);
  return Wire.read();
}
