/**
 * Demonstração do uso do PCF8574 para
 * entrada e saída
 * 
 */

#include <Wire.h>

// Endereço do PCF8574A com pinos de endereçamento aterrados
// (para PCF7584 usar 0x20)
const byte PCF8574_Addr = 0x38;

// Conexões do PCF8574
const byte BIT_LED = 0x20;   // pino 10 ligado ao catodo do LED
                             // anodo do LED ligado via resistor de 1K a 5V
const byte BIT_BOTAO = 0x10; // pino 9 ligado ao botão
                             // outro terminal do botão ligado a terra

// Iniciação
void setup() {
  Wire.begin();
  // Pinos de entrada precisam ser configurados com nível alto
  // Nível alto apaga o LED
  PCF8574_Write(BIT_LED | BIT_BOTAO);
}

// Que seja eterno enquanto dure
void loop() {
  if (PCF8574_Read() & BIT_BOTAO) {
    // nível alto indica botão solto, apagar o LED
    PCF8574_Write(BIT_LED | BIT_BOTAO);
  } else {
    // nível baixo indica botão apertado, acender o LED
    PCF8574_Write(BIT_BOTAO);    
  }
  delay (100);
}

// Escreve um byte
void PCF8574_Write(byte dado) {
  Wire.beginTransmission(PCF8574_Addr);
  Wire.write(dado);
  Wire.endTransmission();
}

// Lê um byte
byte PCF8574_Read(){
  Wire.requestFrom(PCF8574_Addr, 1);
  return Wire.read();
}
