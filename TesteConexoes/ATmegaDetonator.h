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
const int pinEncoderSw = 12; 
const int pinEncoder1 = A0; 
const int pinEncoder2 = A1; 
const int pinDetona = A2;

// Endereço do PF8574A
// (para PCF7584 usar 0x20)
const byte PCF8574_Addr = 0x38;
