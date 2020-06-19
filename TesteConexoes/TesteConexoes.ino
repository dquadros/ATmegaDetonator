#include "ATmegaDetonator.h"
#include <Wire.h>

/*
 * ATmegaDetonator
 * Teste das conexões ao ATmega que será gravado
 * 
 * (C) 2020, Daniel Quadros
 */

struct {
  char *nome;
  int pino;
} conexoes[] = {
  { "Vcc ", pinVcc },
  { "12V ", pin12V },
  { "OE  ", pinOE },
  { "WR  ", pinWR },
  { "BS1 ", pinBS1 },
  { "BS2 ", pinBS2 },
  { "XA0 ", pinXA0 },
  { "XA1 ", pinXA1 },
  { "PGL ", pinPAGEL },
  { "XTL1", pinXTAL1 },
  { "", 0 }
};

// Inicia todos os pinos como saída e nível baixo
void setup() {
  int i = 0;
  while (conexoes[i].pino != 0) {
    pinMode(conexoes[i].pino, OUTPUT);
    digitalWrite(conexoes[i].pino, LOW);
    i++;
  }
  Wire.begin();
  PCF8574_Write(0);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Testando!");
}

// Aciona cada um dos pinos por 5 segundos
void loop() {
  static int i = 0;
  static int j = 0;

  if (conexoes[i].pino == 0) {
    if (j == 8) {
      PCF8574_Write(0);
      i = j = 0;
    } else {
      PCF8574_Write(1 << j);
      Serial.print("D");
      Serial.print(j);
      Serial.print("\n");
      espera();
      j++;
      return;
    }
  }
  Serial.print(conexoes[i].nome);
  digitalWrite(conexoes[i].pino, HIGH);
  espera();
  digitalWrite(conexoes[i].pino, LOW);
  Serial.print("\n");
  i++;
}

void espera() {
  while (Serial.read() < 0) {
    delay (100);
  }
}

void PCF8574_Write(byte dado) {
  Wire.beginTransmission(PCF8574_Addr);
  Wire.write(dado);
  Wire.endTransmission();
}
