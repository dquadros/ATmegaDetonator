/*
 * ATmegaDetonator - Programação do ATmega
 * 
 * (C) 2020-2021, Daniel Quadros
 */

// pinos de saída
const int pinOut[] = {
  pinVcc, pin12V, pinOE, pinWR, pinBS1, pinBS2,
  pinXA0, pinXA1, pinPAGEL, pinXTAL1,
  0
};

// comandos de programação
const byte CMD_LEID = 0x08;
const byte CMD_LEFUSES = 0x04;
const byte CMD_GRAVAFUSES = 0x40;
const byte CMD_GRAVALOCK = 0x20;
const byte CMD_APAGA = 0x80;
const byte CMD_NOP = 0x00;
const byte CMD_LEFLASH = 0x02;
const byte CMD_GRAVAFLASH = 0x10;

// Iniciação
void ATmega_init() {
  for (int i = 0; pinOut[i] != 0; i++) {
    pinMode(pinOut[i], OUTPUT);
    digitalWrite(pinOut[i], LOW);
  }
  pinMode (pinRdy, INPUT);
  PCF8574_Write(0);
}

// Coloca ATmega no modo programação paralela
bool ATmega_ModoPgm() {
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
  return digitalRead(pinRdy) == HIGH;
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

// Le os três bytes de identificação do ATmega
void ATmega_LeId(byte *id) {
  ATmega_SendCmd (CMD_LEID);
  ATmega_SendAddr (HIGH, 0);
  for (byte addr = 0; addr < 3; addr++) {
    ATmega_SendAddr (LOW, addr);
    id[addr] = ATmega_Read(LOW);
  }
}

// Dispara o apagamento da Flash
void ATmega_Erase() {
  Serial.print ("Apagando...");
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
  Serial.println ("Apagado");
}

// verifica se a Flash está limpa
bool ATmega_CheckClean(ATMEGA_CHIP *chip) {
  bool limpa = true;
  
  // Envia o comando
  ATmega_SendCmd(CMD_LEFLASH);
  // Repete para todas as páginas
  uint16_t dado;
  uint16_t page = 0;
  while (limpa && (page < chip->numPages)) {
    ATmega_SendAddr(HIGH, (byte) page);
    for (byte addr = 0; limpa && (addr < chip->pageSize); addr++) {
      // Enviar a parte baixa do endereço e ler os dois bytes da palavra
      ATmega_SendAddr(LOW, addr);
      PCF8574_Release();
      delayMicroseconds(1);
      digitalWrite (pinBS1, LOW);
      digitalWrite (pinOE, LOW);
      delayMicroseconds(1);
      dado = PCF8574_Read();
      digitalWrite (pinBS1, HIGH);
      delayMicroseconds(1);
      dado = (dado << 8) | PCF8574_Read();
      if (dado != 0xFFFF) {
        limpa = false;
        Serial.print (page, HEX);
        Serial.print (": ");
        Serial.print (addr, HEX);
        Serial.print (": ");
        Serial.println (dado, HEX);
      }
      digitalWrite (pinBS1, LOW);
      digitalWrite (pinOE, HIGH);
    }
    page++;
  }
  // Finaliza a operação
  ATmega_SendCmd(CMD_NOP);
  
  return limpa;  
}

// Grava dados da EEProm na Flash
void ATmega_Grava(uint16_t addrFlash, uint16_t addrEEProm, uint16_t tam, uint8_t pageSize) {
  Serial.print("Gravando ");
  Serial.print(addrFlash, HEX);
  Serial.print(" ");
  Serial.println(tam, HEX);
  addrFlash = addrFlash >> 1;   // converte para endereço de words
  // Envia o comando
  ATmega_SendCmd(CMD_GRAVAFLASH);
  // Loop por página
  uint16_t nGrv = 0;
  while (nGrv < tam) {
    Serial.print ("Gravando a pagina ");
    Serial.println (addrFlash >> 6, HEX);
    // Envia a parte alta do endereço
    ATmega_SendAddr(HIGH, (byte) (addrFlash >> 8));
    // Preenche o buffer da página
    for (byte cont = 0; cont < pageSize; cont++) {
      // Enviar o byte menos significativo do endereço e 
      // os dois bytes da palavra
      ATmega_SendAddr(LOW, (byte) addrFlash);
      digitalWrite (pinXA0, HIGH);
      digitalWrite (pinXA1, LOW);
      digitalWrite (pinBS1, LOW);
      PCF8574_Write(leEEProm(addrEEProm++));
      pulsaXTAL1();
      digitalWrite (pinBS1, HIGH);
      PCF8574_Write(leEEProm(addrEEProm++));
      pulsaXTAL1();
      // Pulsa PAGEL para colocar a palavra no buffer
      digitalWrite (pinPAGEL, HIGH);
      delayMicroseconds(1);
      digitalWrite (pinPAGEL, LOW);
      addrFlash++;
      nGrv += 2;
    }
    // Dispara a gravação
    digitalWrite (pinWR, LOW);
    delayMicroseconds(1);
    digitalWrite (pinWR, HIGH);
    delayMicroseconds(1);
    // Aguarda o fim da gravação
    while (digitalRead(pinRdy) == LOW) {
      delay(10);
    }
  }
  // Finaliza a programação
  ATmega_SendCmd(CMD_NOP);
  Serial.println("Gravado.");
}

// Confere o que foi gravado na Flash
bool ATmega_Verifica(uint16_t addrFlash, uint16_t addrEEProm, uint16_t tam) {  
  int erros = 0;
  Serial.println("Conferindo...");
  addrFlash = addrFlash >> 1;   // converte para endereço de words
  // Envia o comando
  ATmega_SendCmd(CMD_LEFLASH);
  // Repete para todas as páginas
  byte valor;
  byte dado;
  uint16_t nLido = 0;
  while (nLido < tam) {
    // Seleciona a parte alta do endereço
    ATmega_SendAddr(HIGH, (byte) (addrFlash >> 8));
    for (int cont = 0; cont < 256; cont++) {
      // Enviar a parte baixa do endereço e ler os dois bytes da palavra
      ATmega_SendAddr(LOW, (byte) addrFlash);
      PCF8574_Release();
      delayMicroseconds(1);
      digitalWrite (pinBS1, LOW);
      digitalWrite (pinOE, LOW);
      delayMicroseconds(1);
      dado = PCF8574_Read();
      valor = leEEProm(addrEEProm++);
      if (dado != valor) {
        erros++;
        Serial.print ("ERRO: ");
        Serial.print (addrFlash, HEX);
        Serial.print (": ");
        Serial.print (dado, HEX);
        Serial.print (" x ");
        Serial.println (valor, HEX);
      }
      digitalWrite (pinBS1, HIGH);
      delayMicroseconds(1);
      dado = PCF8574_Read();
      valor = leEEProm(addrEEProm++);
      if (dado != valor) {
        erros++;
        Serial.print ("ERRO: ");
        Serial.print (addrFlash, HEX);
        Serial.print (": ");
        Serial.print (dado, HEX);
        Serial.print (" x ");
        Serial.println (valor, HEX);
      }
      digitalWrite (pinBS1, LOW);
      digitalWrite (pinOE, HIGH);
      addrFlash++;
      nLido += 2;
    }
  }
  // Finaliza a operação
  ATmega_SendCmd(CMD_NOP);
  if (erros == 0) {
      Serial.println("Sucesso!");
  } else {
    Serial.print(erros);
    Serial.println(" erros!!!");
  }
  return erros == 0;
}

void ATmega_WriteLFUSE (byte valor) {
  ATmega_WriteFuse (LOW, LOW, valor);
}

void ATmega_WriteHFUSE (byte valor) {
  ATmega_WriteFuse (HIGH, LOW, valor);
}

void ATmega_WriteEFUSE (byte valor) {
  ATmega_WriteFuse (LOW, HIGH, valor);
}

void ATmega_WriteLOCK (byte valor) {
  // Envia o comando
  ATmega_SendCmd(CMD_GRAVALOCK);
  // indica que vai enviar um dado
  digitalWrite (pinXA0, HIGH);
  digitalWrite (pinXA1, LOW);
  // Seleciona low byte
  digitalWrite (pinBS1, LOW);
  // Coloca o dado na via de dados
  PCF8574_Write(valor);
  // Pulsa XTAL1 para registrar o dado
  pulsaXTAL1();
  // Dispara a gravação  
  digitalWrite (pinWR, LOW);
  delayMicroseconds(1);
  digitalWrite (pinWR, HIGH);
  delayMicroseconds(1);
  // Aguarda o fim da gravação
  while (digitalRead(pinRdy) == LOW) {
    delay(10);
  }
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

byte ATmega_ReadLFUSE() {
  return ATmega_ReadFuse (LOW, LOW);
}

byte ATmega_ReadHFUSE() {
  return ATmega_ReadFuse (HIGH, HIGH);
}

byte ATmega_ReadEFUSE() {
  return ATmega_ReadFuse (LOW, HIGH);
}

byte ATmega_ReadLOCK() {
  return ATmega_ReadFuse (HIGH, LOW);
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

// Lê um byte do ATmega
byte ATmega_Read (byte ordem) {
  byte dado;
  
  // Libera a via de dados
  PCF8574_Release();
  delayMicroseconds(1);
  // indica se é o byte LOW ou HIGH
  digitalWrite (pinBS1, ordem);
  // Habilita a saída do byte  
  digitalWrite (pinOE, LOW);
  delayMicroseconds(1);
  // Lê o byte
  dado = PCF8574_Read();
  // Faz o ATMega soltar a via de dados 
  digitalWrite (pinOE, HIGH);
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
