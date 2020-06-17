#include "ATmegaDetonator.h"

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
  Serial.begin(115200);
  Serial.println();
  Serial.println("Testando!");
}

// Aciona cada um dos pinos por 5 segundos
void loop() {
  static int i = 0;

  Serial.print(conexoes[i].nome);
  digitalWrite(conexoes[i].pino, HIGH);
  delay (5000);
  digitalWrite(conexoes[i].pino, LOW);
  Serial.print("\r");
  i++;
  if (conexoes[i].pino == 0) {
    i = 0;
  }
}
