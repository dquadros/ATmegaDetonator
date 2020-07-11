#include "ATmegaDetonator.h"
#include <Wire.h>

/*
 * ATmegaDetonator
 * Apagamento da Flash e escrita dos Fuses com os valores de fabrica
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
const byte CMD_GRAVAFUSES = 0x40;
const byte CMD_APAGA = 0x80;
const byte CMD_NOP = 0x00;

// valores de fabrica dos fuses (ATmega328)
const byte FUSE_LOW  = 0x62;
const byte FUSE_HIGH = 0xD9;
const byte FUSE_EXT  = 0xFF;

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
  Serial.println("Apagamento da Flash");
}

void loop() {
  Serial.println();
  Serial.println("Digite [ENTER] para apagar");
  espera();
  ATmega_ModoPgm();
  ATmega_Erase();
  ATmega_WriteFuse (LOW, LOW, FUSE_LOW);
  ATmega_WriteFuse (HIGH, LOW, FUSE_HIGH);
  ATmega_WriteFuse (LOW, HIGH, FUSE_EXT);
  bool ok = true;
  if (ATmega_ReadFuse (LOW, LOW) != FUSE_LOW) {
    Serial.println ("Erro na gravação do LOW Fuse!");
    ok = false;
  }
  if (ATmega_ReadFuse (HIGH, HIGH) != FUSE_HIGH) {
    Serial.println ("Erro na gravação do HIGH Fuse!");
    ok = false;
  }
  if (ATmega_ReadFuse (LOW, HIGH) != FUSE_EXT) {
    Serial.println ("Erro na gravação do EXTENDED Fuse!");
    ok = false;
  }
  ATmega_Desliga();
  if (ok) {
    Serial.println ("Apagamento bem sucedido.");
  }
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

// Dispara o apagamento da Flash
void ATmega_Erase() {
  // Envia o comando
  ATmega_SendCmd(CMD_APAGA);
  // Dispara o apagamento
  digitalWrite (pinWR, LOW);
  delayMicroseconds(1);
  digitalWrite (pinWR, HIGH);
  delayMicroseconds(1);
  // Aguarda o fim da gravação
  while (digitalRead(pinRdy) == LOW) {
    delay(10);
  }
  // Finaliza o apagamento
  ATmega_SendCmd(CMD_NOP);
}

// Grava um fuse do ATmega
void ATmega_WriteFuse (int bs1, int bs2, byte valor) {
  // Envia o comando
  ATmega_SendCmd(CMD_GRAVAFUSES);
  // indica que vai enviar um dado
  digitalWrite (pinXA0, HIGH);
  digitalWrite (pinXA1, LOW);
  // Seleciona low byte
  digitalWrite (pinBS1, LOW);
  // Coloca o dado na via de dados
  PCF8574_Write(valor);
  // Pulsa XTAL1 para registrar o dado
  pulsaXTAL1();
  // Seleciona o fuse
  digitalWrite (pinBS1, bs1);
  digitalWrite (pinBS2, bs2);
  // Dispara a gravação  
  digitalWrite (pinWR, LOW);
  delayMicroseconds(1);
  digitalWrite (pinWR, HIGH);
  delayMicroseconds(1);
  // Aguarda o fim da gravação
  while (digitalRead(pinRdy) == LOW) {
    delay(10);
  }
  // Volta os sinais para o default
  digitalWrite (pinBS1, LOW);
  digitalWrite (pinBS2, LOW);
}

// Lê um fuse do ATmega
byte ATmega_ReadFuse (int bs1, int bs2) {
  byte dado;

  // Envia o comando
  ATmega_SendCmd(CMD_LEFUSES);
  // Libera a via de dados
  PCF8574_Release();
  delayMicroseconds(1);
  // seleciona o fuse a ler
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
