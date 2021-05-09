/*
 * ATmegaDetonator - Manipulação das informações na EEProm 24C32
 * 
 * As informações sobre os modelos de ATmega suportados e os bootloaders são armazenadas na
 * EEProm da seguinte forma:
 * 
 * - byte mais significativo do endereço da tabela de bootloaders
 * - especificação dos ATmegas:    "n" estruturas ATMEGA_CHIP encerradas por uma com FF FF FF em id
 * - especificação dos Bootloader: "m" estruturas BOOTLOADER, primeira fica em um endereço xx00
 * - código dos Bootloaders:       bootloader começam sempre em endereço xx00 e tem 512 bytes
 * 
 * Esta estrutura é gravada pelo comandos recebidos do PC na opção configura
 * 
 * (C) 2020-2021, Daniel Quadros
 */

I2C_eeprom eep(0x50, 0x1000);

ATMEGA_CHIP atmegaChip;
BOOTLOADER bootloader;

// Procura a identificação de um chip
// Se achar retorna ponteiro para estrura com a descrição
// Se não achar retorna NULL
ATMEGA_CHIP *identificaChip (byte *idLido) {
  uint16_t addr = 1;
  while (true) {
    eep.readBlock(addr, (byte *)&atmegaChip, sizeof(ATMEGA_CHIP));
    if (memcmp (atmegaChip.id, "\xFF\xFF\xFF", 3) == 0) {
      return NULL;
    } else if (memcmp (atmegaChip.id, idLido, 3) == 0) {
      return &atmegaChip;
    }
    addr += sizeof(ATMEGA_CHIP);
  }
}

// Monta menu de bootloaders para um chip
int montaMenuBoot (ATMEGA_CHIP *chip, char *opcBootloader[], byte indice[]) {
  uint16_t addrBoot = eep.readByte(0) << 8;
  int i = 0;
  while (chip->bootloaders[i] != 0xFF) {
    uint16_t addr = addrBoot + chip->bootloaders[i]*sizeof(BOOTLOADER);
    eep.readBlock(addr, (byte *)&bootloader, sizeof(BOOTLOADER));
    strcpy (opcBootloader[i], bootloader.nome);
    indice[i] = chip->bootloaders[i];
    i++;
  }
  strcpy (opcBootloader[i], "Desiste");
  return i+1;
}

// Carrega uma definição de bootloader
BOOTLOADER *infoBootloader (int indice) {
  uint16_t addrBoot = eep.readByte(0) << 8;
  uint16_t addr = addrBoot + indice*sizeof(BOOTLOADER);
  eep.readBlock(addr, (byte *)&bootloader, sizeof(BOOTLOADER));
  return &bootloader;
}

// Grava dados na EEProm
void gravaEEProm (uint16_t addr, uint8_t tam, byte *dados) {
  eep.writeBlock (addr, dados, tam);
}

// Le um byte da EEProm
byte leEEProm (uint16_t addr) {
  return eep.readByte(addr);
}

// Para teste da carga
void dumpChips() {
  Serial.println ("Chips");
  uint16_t addr = 1;
  int ultBoot = -1;
  while (true) {
    eep.readBlock(addr, (byte *)&atmegaChip, sizeof(ATMEGA_CHIP));
    if (memcmp (atmegaChip.id, "\xFF\xFF\xFF", 3) == 0) {
      break;
    }
    printHex (atmegaChip.id[0], " ");
    printHex (atmegaChip.id[1], " ");
    printHex (atmegaChip.id[2], " / ");
    printHex (atmegaChip.fusesFab[0], " ");
    printHex (atmegaChip.fusesFab[1], " ");
    printHex (atmegaChip.fusesFab[2], " ");
    printHex (atmegaChip.fusesFab[3], " / ");
    printHex (atmegaChip.pageSize, " ");
    printHex (atmegaChip.numPages >> 8, NULL);
    printHex (atmegaChip.numPages & 0xFF, " / ");
    for (int i = 0; i < 5; i++) {
      printHex (atmegaChip.bootloaders[i], " ");
      if ((atmegaChip.bootloaders[i] != 255) && (atmegaChip.bootloaders[i] > ultBoot)) {
        ultBoot = atmegaChip.bootloaders[i];
      }
    }
    Serial.println (atmegaChip.nome);
    addr += sizeof(ATMEGA_CHIP);
  }

  Serial.println ("Bootloaders");
  addr = eep.readByte(0) << 8;
  for (int i = 0; i <= ultBoot; i++) {
    eep.readBlock(addr, (byte *)&bootloader, sizeof(BOOTLOADER));
    printHex (bootloader.fuses[0], " ");
    printHex (bootloader.fuses[1], " ");
    printHex (bootloader.fuses[2], " ");
    printHex (bootloader.fuses[3], " / ");
    printHex (bootloader.addrHi, " / ");
    printHex (bootloader.addrBoot >> 8, NULL);
    printHex (bootloader.addrBoot & 0xFF, " / ");
    printHex (bootloader.tamanho >> 8, NULL);
    printHex (bootloader.tamanho & 0xFF, " ");
    Serial.println (bootloader.nome);
    addr += sizeof(BOOTLOADER);
  }  
}

void printHex (byte valor, char *sep) {
  char aux[3];
  aux[0] = pgm_read_byte(&hexa[valor >> 4]);
  aux[1] = pgm_read_byte(&hexa[valor & 0x0F]);
  aux[2] = 0;
  Serial.print(aux);
  if (sep != NULL) {
    Serial.print(sep);
  }
}
