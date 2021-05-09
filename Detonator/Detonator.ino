/*
 * ATmegaDetonator - Programa Principal
 * 
 * Programador Paralelo "Alta Voltagem" para microcontroladores ATmega
 * 
 * (C) 2020-2021, Daniel Quadros
 */

#include "ATmegaDetonator.h"
#include <Wire.h>
#include <TimerOne.h>
#include <I2C_eeprom.h>

const char *versao = "v1.00";

const PROGMEM uint8_t hexa[] = "0123456789ABCDEF";

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

char opcoes[5][17];
char *opcBootloader[5] = {
  opcoes[0], opcoes[1], opcoes[2], opcoes[3], opcoes[4]
};
MENU bootloaders = {
  "Bootloader",
  0,
  opcBootloader
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
      GravaChip();
      break;
    case 3:
      CargaConfig();
      break;
    case 4:
      dumpChips();
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
      if (chip != NULL) {
        limpa = ATmega_CheckClean(chip);
      }
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
    if (!confirmaDetona()) {
      return;
    }
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

// Apaga o chip e grava um bootloader
void GravaChip() {
  ATMEGA_CHIP *chip;
  byte id[3];
  byte indices[5];

  // 1. Identificar o chip
  Encoder_stop();
  Display_clear ();
  Display_print (0, 0, "Grava Bootloader", VID_NORMAL);
  Display_print (1, 0, "Aguarde...", VID_NORMAL);
  Display_write (0, 15, 0x7F, VID_NORMAL);
  if (ATmega_ModoPgm()) {
    ATmega_LeId (id);
    chip = identificaChip(id);
    ATmega_Desliga();
    Display_write (0, 15, ' ', VID_NORMAL);
    Display_clearline (1);
    if (chip == NULL) {
      Display_print (1, 0, "Desconhecido", VID_NORMAL);
      Encoder_start();
      aguardaEnter();
      return;
    }
  } else {
    ATmega_Desliga();
    Display_write (0, 15, ' ', VID_NORMAL);
    Display_print (2, 0, "ERRO", VID_NORMAL);
    Encoder_start();
    aguardaEnter();
    return;
  }

  // 2. Primeira confirmação
  Display_print (1, 0, chip->nome, VID_NORMAL);
  Display_print (2, 0, "DETONA: Continua", VID_NORMAL);
  if (!confirmaDetona()) {
    return;
  }

  // 3. Pede para escolher o bootloader
  bootloaders.nItens = montaMenuBoot(chip, opcBootloader, indices);
  Encoder_start();
  int opc = leMenu (&bootloaders);
  Encoder_stop();
  if (opc == (bootloaders.nItens-1)) {
    return;
  }

  // 4. Segunda confirmação
  Display_print (1, 0, opcBootloader[opc], VID_NORMAL);
  Display_print (2, 0, "DETONA: Continua", VID_NORMAL);
  Display_clearline (3);
  if (!confirmaDetona()) {
    return;
  }

  // 5. Apaga
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

  // 6. Grava código do bootloader
  BOOTLOADER *bl = infoBootloader (indices[opc]);
  ATmega_Grava (bl->addrBoot, bl->addrHi << 8, bl->tamanho, chip->pageSize);
  
  // 7. Verifica gravação
  bool ok = ATmega_Verifica (bl->addrBoot, bl->addrHi << 8, bl->tamanho);

  // 8. Programa fuses
  if (ok) {
    ATmega_WriteLFUSE (bl->fuses[0]);
    ATmega_WriteHFUSE (bl->fuses[1]);
    if (bl->fuses[2] != 0) {
      ATmega_WriteEFUSE (bl->fuses[2]);
    }
    ATmega_WriteLOCK (bl->fuses[3]);
  }

  // 9. Informa o resultado
  ATmega_Desliga();
  Display_clear ();
  Display_print (0, 0, "Grava Bootloader", VID_NORMAL);
  Display_print (1, 0, opcBootloader[opc], VID_NORMAL);
  if (ok) {
    Display_print (2, 0, "Sucesso", VID_NORMAL);
  } else {
    Display_print (2, 0, "ERRO", VID_NORMAL);
  }
  Encoder_start();
  aguardaEnter();
}

// Carga da Configuração pela Serial
void CargaConfig() {
  // Confirma
  Display_clear ();
  Display_print (0, 0, "Carga CONFIG", VID_NORMAL);
  Display_print (1, 0, "Use DETONA", VID_NORMAL);
  Display_print (2, 0, "para confirmar", VID_NORMAL);
  if (!confirmaDetona()) {
    return;
  }
  Display_clearline (1);
  Display_clearline (2);
  Display_print (1, 0, "Aguarde...", VID_NORMAL);
  
  // Recebe a configuração pela serial
  //
  // comandos aceitos
  // *              confirma Detonator pronto
  // :eeee tt dd... grava os tt bytes dd... a partir do endereço eeee (valores em hexadecimal)
  // !              encerra gravação
  //
  // O detonator responde . em caso de sucesso
  //
  byte dados[64];
  while (true) {
    switch (Serial.read()) {
      case '*':
        Serial.write ('.');
        break;
      case '!':
        Serial.write ('.');
        Display_clearline (1);
        Display_print (1, 0, "Pronto!", VID_NORMAL);
        Encoder_start();
        aguardaEnter();
        return;
      case ':':
        uint16_t addr = leHex() << 8;
        addr += leHex();
        uint8_t tam = leHex();
        for (uint8_t i = 0; i < tam; i++) {
          dados[i] = leHex();
        }
        gravaEEProm (addr, tam, dados);
        Serial.write ('.');
        break;
    }
  }
}

// Le um byte em hexadecimal da serial, ignorando espaços
byte leHex() {
  return (leDigHex() << 4) + leDigHex();
}

// Le um digito em hexadecimal da serial, ignorando espaços
byte leDigHex() {
  int car;
  do {
    car = Serial.read();
  } while ((car == -1) || (car == ' '));
  if ((car >= '0') && (car <= '9')) {
    return (byte) (car - '0');
  }
  if ((car >= 'A') && (car <= 'F')) {
    return (byte) (car - 'A' + 10);
  }
  if ((car >= 'a') && (car <= 'f')) {
    return (byte) (car - 'a' + 10);
  }
  return 0;
}

// Tela de Apresentação
void Splash() {
  Display_clear ();
  Display_print (0, 0, "ATmega Detonator", VID_NORMAL);
  Display_print (1, 5, versao, VID_NORMAL);
  Display_print (3, 0, "Daniel Quadros", VID_NORMAL);
  delay(3000);
  Display_clear();
}
