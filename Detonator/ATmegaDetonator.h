/*
    Definições globais para o ATmegaDetonator
    
    (C) 2020, Daniel Quadros
*/

// Conexões ao ATmega que será gravado
const int pinVcc = 2;
const int pin12V = 3;
const int pinOE = 4;
const int pinWR = 5;
const int pinBS1 = 6;
const int pinBS2 =7;
const int pinXA0 = 8;
const int pinXA1 = 9;
const int pinPAGEL = 10;
const int pinXTAL1 = 11;
const int pinRdy = A3;

// Encoder e Botao
const int pinEncSW = 12;
const int pinEncDT = A0;
const int pinEncCL = A1;
const int pinDetona = A2;

// Endereço do PF8574A
// (para PCF7584 usar 0x20)
const byte PCF8574_Addr = 0x38;

// Endereço I2C do display
const byte DISP_Addr = 0x3C;

// Seleção de vídeo normal ou reverso
const byte VID_NORMAL = 0x00;
const byte VID_REVERSO = 0xFF;

// Estrutura para controlar um menu
typedef struct {
  char *titulo;
  int nItens;
  char **opcoes;
} MENU;

// Ações geradas pelo Rotary Encoder e botão Detona
typedef enum { NADA, UP, DOWN, ENTER, DETONA } AcaoEnc;

// Especificação de um ATmega
typedef struct {
  byte id[3];           // bytes de identificação, FF FF FF indica fim das especificações
  byte fusesFab[4];     // valores de fábrica para LFUSE, HFUSE, EFUSE e LOCK
                        // valor 00 para EFUSE indica que ele não está presente
  char nome[17];        // nome do modelo (terminado por nul)
  int32_t addrBoot;     // endereço inicial de gravação do bootloader na Flash
  uint8_t pageSize;     // número de words de 16 bits em cada página da Flash
  uint16_t numPages;    // número de páginas na Flash
  byte bootloaders[5];  // indices dos bootloaders aplicaveis (terminado por 0xFF)
} ATMEGA_CHIP;

// Especificação de um bootloader
typedef struct {
  char nome[17];    // nome do bootloader (terminado por nul)
  byte fuses[4];    // valores para LFUSE, HFUSE, EFUSE e LOCK
  byte addrHi;      // byte mais significativo do endereço do bootloader na EEProm
} BOOTLOADER;
