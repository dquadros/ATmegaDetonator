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
 * Esta estrutura é montada a partir de dados recebidos do PC na opção configura
 * 
 * (C) 2020, Daniel Quadros
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


// Cria descritores para teste
// Na versão final isto será carregado pela serial
void criaChips() {
  uint16_t addr = 1;
  
  // ATmega328
  atmegaChip.id[0] = 0x1E; atmegaChip.id[1] = 0x95; atmegaChip.id[2] = 0x14;
  atmegaChip.fusesFab[0] = 0x62; atmegaChip.fusesFab[1] = 0xD9; 
  atmegaChip.fusesFab[2] = 0xFF; atmegaChip.fusesFab[3] = 0xFF; 
  atmegaChip.pageSize = 64; atmegaChip.numPages = 256;
  strcpy (atmegaChip.nome, "ATmega328");
  eep.writeBlock(addr, (byte *)&atmegaChip, sizeof(ATMEGA_CHIP));
  addr += sizeof(ATMEGA_CHIP);
  
  // ATmega328P
  atmegaChip.id[0] = 0x1E; atmegaChip.id[1] = 0x95; atmegaChip.id[2] = 0x0F;
  atmegaChip.fusesFab[0] = 0x62; atmegaChip.fusesFab[1] = 0xD9; 
  atmegaChip.fusesFab[2] = 0xFF; atmegaChip.fusesFab[3] = 0xFF; 
  atmegaChip.pageSize = 64; atmegaChip.numPages = 256;
  strcpy (atmegaChip.nome, "ATmega328P");
  eep.writeBlock(addr, (byte *)&atmegaChip, sizeof(ATMEGA_CHIP));
  addr += sizeof(ATMEGA_CHIP);

  // ATmega168
  atmegaChip.id[0] = 0x1E; atmegaChip.id[1] = 0x94; atmegaChip.id[2] = 0x06;
  atmegaChip.fusesFab[0] = 0x62; atmegaChip.fusesFab[1] = 0xDF; 
  atmegaChip.fusesFab[2] = 0xF9; atmegaChip.fusesFab[3] = 0xFF; 
  atmegaChip.pageSize = 64; atmegaChip.numPages = 128;
  strcpy (atmegaChip.nome, "ATmega168");
  eep.writeBlock(addr, (byte *)&atmegaChip, sizeof(ATMEGA_CHIP));
  addr += sizeof(ATMEGA_CHIP);

  // ATmega8
  atmegaChip.id[0] = 0x1E; atmegaChip.id[1] = 0x93; atmegaChip.id[2] = 0x07;
  atmegaChip.fusesFab[0] = 0xE1; atmegaChip.fusesFab[1] = 0xD9; 
  atmegaChip.fusesFab[2] = 0x00; atmegaChip.fusesFab[3] = 0xFF; 
  atmegaChip.pageSize = 32; atmegaChip.numPages = 128;
  strcpy (atmegaChip.nome, "ATmega8");
  eep.writeBlock(addr, (byte *)&atmegaChip, sizeof(ATMEGA_CHIP));
  addr += sizeof(ATMEGA_CHIP);

  // Fim
  atmegaChip.id[0] = 0xFF; atmegaChip.id[1] = 0xFF; atmegaChip.id[2] = 0xFF;
  eep.writeBlock(addr, (byte *)&atmegaChip, sizeof(ATMEGA_CHIP));
}
