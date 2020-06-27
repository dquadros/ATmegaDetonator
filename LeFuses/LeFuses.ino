#include "ATmegaDetonator.h"
#include <Wire.h>

/*
 * ATmegaDetonator
 * Teste de leitura dos "fuses" do ATmega
 * 
 * (C) 2020, Daniel Quadros
 */

// pinos de saída
int pinOut[] = {
  pinVcc, pin12V, pinOE, pinWR, pinBS1, pinBS2,
  pinXA0, pinXA1, pinPAGEL, pinXTAL1,
  0
};

// comandos de programação
const byte CMD_LEID = 0x08;
const byte CMD_LEFUSES = 0x04;

// Iniciação
void setup() {
  for (int i = 0; pinOut[i] != 0; i++) {
    pinMode(pinOut[i], OUTPUT);
    digitalWrite(pinOut[i], LOW);
  }
  pinMode (pinRdy, INPUT);
  Wire.begin();
  PCF8574_Write(0);
  Serial.begin(115200);
  Serial.println("Leitura dos Fuses");
}

void loop() {
  byte fuses[4];
  
  Serial.println();
  Serial.println("Digite [ENTER]");
  espera();
  ATmega_ModoPgm();
  ATmega_LeFuses(fuses);
  ATmega_Desliga();
  Serial.print (fuses[0], HEX);
  Serial.print (".");
  Serial.print (fuses[1], HEX);
  Serial.print (".");
  Serial.print (fuses[2], HEX);
  Serial.print (".");
  Serial.print (fuses[3], HEX);
  Serial.println();
}

// Le os quatro bytes de "fuses" do ATmega
void ATmega_LeFuses(byte *fuses) {
  ATmega_SendCmd (CMD_LEFUSES);
  fuses[0] = ATmega_Read (LOW, LOW);   // LFUSE
  fuses[1] = ATmega_Read (HIGH, HIGH); // HFUSE
  fuses[2] = ATmega_Read (LOW, HIGH);  // EFUSE
  fuses[3] = ATmega_Read (HIGH, LOW);  // LOCK
}

// Coloca ATmega no modo programação paralela
void ATmega_ModoPgm() {
  Serial.println ("Colocando no modo de programacao paralela");
  // Garantir que está sem alimentação e reset
  digitalWrite (pinVcc, LOW);
  digitalWrite (pin12V, LOW);
  // Condição de entrada no modo programação paralela
  digitalWrite (pinPAGEL, LOW);
  digitalWrite (pinXA0, LOW);
  digitalWrite (pinXA1, LOW);
  digitalWrite (pinBS1, LOW);
  // Ligar a alimentação
  digitalWrite (pinVcc, HIGH);
  // Estes sinais devem ficar normalmente em nível alto
  digitalWrite (pinOE, HIGH);
  digitalWrite (pinWR, HIGH);
  // Aguardar e colocar 12V no reset
  delayMicroseconds(40);
  digitalWrite (pin12V, HIGH);
  // Aguardar a entrada
  delayMicroseconds(400);
  if (digitalRead(pinRdy) == HIGH) {
    Serial.println ("Sucesso!");
  } else {
    Serial.println ("Nao acionou sinal RDY...");
  }
}

// Retira o ATmega do modo programação e o desliga
void ATmega_Desliga() {
  // Desliga os 12V para sair do modo programação
  digitalWrite (pin12V, LOW);
  // Dá um tempo e desliga
  delayMicroseconds(300);
  digitalWrite (pinVcc, LOW);
  // Vamos deixar todos os pinos em nível LOW
  // Para poder tirar o chip
  for (int i = 0; pinOut[i] != 0; i++) {
    digitalWrite(pinOut[i], LOW);
  }
  PCF8574_Write(0);
}

// Envia um comando para o ATmega
void ATmega_SendCmd (byte cmd) {
  // indica que vai enviar um comando
  digitalWrite (pinXA0, LOW);
  digitalWrite (pinXA1, HIGH);
  digitalWrite (pinBS1, LOW);
  // Coloca o comando na via de dados
  PCF8574_Write(cmd);
  // Pulsa XTAL1 para registrar o comando
  pulsaXTAL1();
}

// Envia um byte de endereço para o ATmega
void ATmega_SendAddr (byte ordem, byte addr) {
  // indica que vai enviar um endereço
  digitalWrite (pinXA0, LOW);
  digitalWrite (pinXA1, LOW);
  // indica se é o byte LOW ou HIGH
  digitalWrite (pinBS1, ordem);
  // Coloca o endereço na via de dados
  PCF8574_Write(addr);
  // Pulsa XTAL1 para registrar o endereço
  pulsaXTAL1();
}

// Lê um byte do ATmega
byte ATmega_Read (int bs1, int bs2) {
  byte dado;
  
  // Libera a via de dados
  PCF8574_Release();
  delayMicroseconds(1);
  // seleciona o byte a ler
  digitalWrite (pinBS1, bs1);
  digitalWrite (pinBS2, bs2);
  // Habilita a saída do byte  
  digitalWrite (pinOE, LOW);
  delayMicroseconds(1);
  // Lê o byte
  dado = PCF8574_Read();
  // Faz o ATMega soltar a via de dados 
  digitalWrite (pinOE, HIGH);
  // Volta os sinais para o default
  digitalWrite (pinBS1, LOW);
  digitalWrite (pinBS2, LOW);
  return dado;
}

// Gera um pulso em XTAL1
void pulsaXTAL1() {
  delayMicroseconds(1);
  digitalWrite (pinXTAL1, HIGH);
  delayMicroseconds(1);
  digitalWrite (pinXTAL1, LOW);
  delayMicroseconds(1);
}


// Espera digitar ENTER
void espera() {
  while (Serial.read() != '\r') {
    delay (100);
  }
}
