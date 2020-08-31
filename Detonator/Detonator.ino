/*
 * ATmegaDetonator - Programa Principal
 * 
 * Programador Paralelo "Alta Voltagem" para microcontroladores ATmega
 * 
 * (C) 2020, Daniel Quadros
 */

/*
 * TODO
 * 
 * [ ] GravaBootloader
 * [ ] CargaConfig
 * 
 */


#include "ATmegaDetonator.h"
#include <Wire.h>
#include <TimerOne.h>
#include <I2C_eeprom.h>

const char *versao = "v0.01";

// Menu Principal
const char *opcPrincipal[] = {
  "Identifica chip",
  "Apaga ATmega",
  "Grava Bootloader",
  "Carrega Config.",
  "Sobre"
};
const MENU principal = {
  "ATmega Detonator",
  sizeof(opcPrincipal)/sizeof(char *),
  opcPrincipal
};

// Iniciação
void setup() {
  delay(100);
  Serial.begin(115200);
  Serial.println("ATmega Detonator");
  Wire.begin();
  ATmega_init();
  delay(100);
  Display_init();
  Encoder_init();
  Splash();
}

// Laço Principal
void loop() {
  Encoder_start();
  switch (leMenu (&principal)) {
    case 0:
      IdentificaChip();
      break;
    case 1:
      ApagaChip();
      break;
    case 2:
      break;
    case 3:
      criaChips();  // provisório
      break;
    case 4:
      Splash();
      break;
  }  
}

// Identifica o modelo do ATmega e informa os Fuses
void IdentificaChip() {
  byte id[3];
  
  Encoder_stop();
  Display_clear ();
  Display_print (0, 0, "Identifica", VID_NORMAL);
  Display_print (1, 0, "Aguarde...", VID_NORMAL);
  Display_write (0, 15, 0x7F, VID_NORMAL);
  if (ATmega_ModoPgm()) {
    ATMEGA_CHIP *chip;
    byte lfuse = 0, hfuse = 0, efuse = 0, lock = 0;
    bool limpa;
    ATmega_LeId (id);
    Serial.print(id[0], HEX); Serial.print(" ");
    Serial.print(id[1], HEX); Serial.print(" ");
    Serial.println(id[2], HEX);
    // Se está lendo sempre "1", ninguém está colocando dados
    bool vazio = (id[0] & id[1] & id[2]) == 0xFF;
    if (!vazio) {    
      chip = identificaChip(id);
      lfuse = ATmega_ReadLFUSE();
      hfuse = ATmega_ReadHFUSE();
      lock = ATmega_ReadLOCK();
      if ((chip != NULL) && (chip->fusesFab[2] != 0)) {
        efuse = ATmega_ReadEFUSE();
      }
      limpa = ATmega_CheckClean(chip);
    }
    ATmega_Desliga();
    Display_write (0, 15, ' ', VID_NORMAL);
    Display_clearline (1);
    if (vazio) {
      Display_print (1, 0, "Sem resposta", VID_NORMAL);
    } else {
      if (chip == NULL) {
        Display_print (1, 0, "Desconhecido", VID_NORMAL);
      } else {
        Display_print (1, 0, chip->nome, VID_NORMAL);
      }
      Display_print (2, 0, "FUSES: --:--:--", VID_NORMAL);
      Display_writehex (2,  7, lfuse);
      Display_writehex (2, 10, hfuse);
      if ((chip != NULL) && (chip->fusesFab[2] != 0)) {
        Display_writehex (2, 13, efuse);
      }
      Display_print (3, 0, "LOCK:", VID_NORMAL);
      Display_writehex (3,  7, lock);
      if (limpa) {
        Display_print (3, 10, "LIMPO", VID_NORMAL);
      }
    }
  } else {
    ATmega_Desliga();
    Display_write (0, 15, ' ', VID_NORMAL);
    Display_clearline (1);
    Display_print (1, 0, "ERRO", VID_NORMAL);
  }
  Encoder_start();
  aguardaEnter();
}

// Apaga a Flash e coloca os valores de fábrica nos fuses
void ApagaChip() {
  byte id[3];
  
  Encoder_stop();
  Display_clear ();
  Display_print (0, 0, "Apaga", VID_NORMAL);
  Display_print (1, 0, "Aguarde...", VID_NORMAL);
  Display_write (0, 15, 0x7F, VID_NORMAL);
  if (ATmega_ModoPgm()) {
    ATmega_LeId (id);
    ATMEGA_CHIP *chip = identificaChip(id);
    ATmega_Desliga();
    Display_write (0, 15, ' ', VID_NORMAL);
    Display_clearline (1);
    if (chip == NULL) {
      Display_print (1, 0, "Desconhecido", VID_NORMAL);
      Encoder_start();
      aguardaEnter();
      return;
    }
    Display_print (1, 0, chip->nome, VID_NORMAL);
    Display_print (2, 0, "DETONA?", VID_NORMAL);
    Encoder_start();
    AcaoEnc acao;
    while ((acao = tiraFilaEnc()) == NADA) {
      delay (100);
    }
    if (acao != DETONA) {
      return;
    }
    Encoder_stop();
    Display_clearline (2);
    Display_print (2, 0, "Aguarde...", VID_NORMAL);
    Display_write (0, 15, 0x7F, VID_NORMAL);
    ATmega_ModoPgm();
    ATmega_Erase();
    ATmega_WriteLFUSE (chip->fusesFab[0]);
    ATmega_WriteHFUSE (chip->fusesFab[1]);
    if (chip->fusesFab[2] != 0) {
      ATmega_WriteEFUSE (chip->fusesFab[2]);
    }
    byte lfuse = ATmega_ReadLFUSE();
    byte hfuse = ATmega_ReadHFUSE();
    byte efuse = 0x00;
    if (chip->fusesFab[2] != 0) {
      efuse = ATmega_ReadEFUSE();
    }
    bool limpa = ATmega_CheckClean(chip);
    ATmega_Desliga();
    Display_write (0, 15, ' ', VID_NORMAL);
    Display_clearline (2);
    Display_print (2, 0, "FUSES: --:--:--", VID_NORMAL);
    Display_writehex (2,  7, lfuse);
    Display_writehex (2, 10, hfuse);
    if (chip->fusesFab[2] != 0) {
      Display_writehex (2, 13, efuse);
    }
    if (limpa) {
      Display_print (3, 0, "LIMPO", VID_NORMAL);
    } else {
      Display_print (3, 0, "ERRO AO LIMPAR", VID_NORMAL);
    }
  } else {
    ATmega_Desliga();
    Display_write (0, 15, ' ', VID_NORMAL);
    Display_print (2, 0, "ERRO", VID_NORMAL);
  }
  Encoder_start();
  aguardaEnter();
}

// Tela de Apresentação
void Splash() {
  Display_clear ();
  Display_print (0, 0, "ATmega Detonator", VID_NORMAL);
  Display_print (1, 4, versao, VID_NORMAL);
  Display_print (3, 0, "Daniel Quadros", VID_NORMAL);
  delay(3000);
  Display_clear();
}
