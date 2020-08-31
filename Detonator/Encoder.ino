/*
 * ATmegaDetonator - Tratamento do Enconder
 * 
 * (C) 2020, Daniel Quadros
 */

// Fila da ações lidas do Rotary Encoder
const byte NBITS_FILA = 4;
const byte tamFilaEnc = 1 << NBITS_FILA;
volatile AcaoEnc filaEnc[tamFilaEnc];
volatile byte poeEnc = 0;
volatile byte tiraEnc = 0;

// Iniciação do módulo
void Encoder_init() {
  // Configura os pinos
  pinMode(pinEncSW, INPUT);
  pinMode(pinEncDT, INPUT);
  pinMode(pinEncCL, INPUT);
  pinMode(pinDetona, INPUT_PULLUP);
  
  // Configura timer1
  Timer1.initialize(1000);  // 1 milisegundo
  Timer1.stop();            // fica quieto até precisar
}

// Inicia monitoração do Encoder
void Encoder_start() {
  // Limpa a fila
  poeEnc = tiraEnc = 0;
  // Liga o timer e habilita interrupção de tempo
  Timer1.start();
  Timer1.attachInterrupt(trataEncoder);
}

// Encerra monitoração do Encoder
void Encoder_stop() {
  Timer1.stop();
  Timer1.detachInterrupt();
}

// Coloca uma ação na fila
// Só chamar na interrupção
void inline poeFilaEnc(AcaoEnc acao) {
  byte temp = (poeEnc + 1) & (tamFilaEnc-1);
  if (temp != tiraEnc) {
    filaEnc[poeEnc] = acao;
    poeEnc = temp;
  }
}

// Retira uma ação da fila
// Não chamar na interrupção
AcaoEnc tiraFilaEnc() {
  AcaoEnc ret = NADA;
  noInterrupts();
  if (poeEnc != tiraEnc) {
    ret = filaEnc[tiraEnc];
    byte temp = (tiraEnc + 1) & (tamFilaEnc-1);
    tiraEnc = temp;
  }
  interrupts();
  return ret;
}

/**
 * Rotina chamada periodicamente pelo Timer2
 * Precisa ser rápida para não interferir nas
 * interrupções de I2C da biblioteca Wire
 */
void trataEncoder() {
  static byte swAnt = 1;
  static byte clkAnt = 1;
  static byte detonaAnt = 1;
  static byte swDb = 1;
  static byte clkDb = 1;
  static byte detonaDb = 1;

  // Vamos ler direto do hw para ganhar tempo
  byte sw = (PINB & 0x10) != 0;      // D12 = PB4
  byte dt = (PINC & 0x01) != 0;      // A0  = PC0
  byte clk = (PINC & 0x02) != 0;     // A1  = PC1
  byte detona = (PINC & 0x04) != 0;  // A2  = PC2

  // Debounce da chave
  if (sw == swAnt) {
    if (sw != swDb) {
      if (sw) {
        poeFilaEnc(ENTER);
      }
      swDb = sw;
    }
  } else {
    swAnt = sw;
  }

  // Debounce do encoder
  if (clkAnt == clk) {
    if (clk != clkDb) {
      if (clk) {
        poeFilaEnc(dt == clk? UP : DOWN);
      }
      clkDb = clk;
    }
  } else {
    clkAnt = clk;
  }
  
  // Debounce botão Detona
  if (detona == detonaAnt) {
    if (detona != detonaDb) {
      if (detona) {
        poeFilaEnc(DETONA);
      }
      detonaDb = detona;
    }
  } else {
    detonaAnt = detona;
  }
}

/*
 * Aguarda pressionar o botão do encoder
 */
void aguardaEnter() {
  while (tiraFilaEnc() != ENTER) {
    delay(100);
  }
}
