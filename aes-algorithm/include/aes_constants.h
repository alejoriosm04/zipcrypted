#ifndef AES_CONSTANTS_H
#define AES_CONSTANTS_H

// Definiciones básicas del algoritmo AES
typedef unsigned char aes_byte;
const int AES_BLOCK_SIZE         = 16;
const int AES_KEY_SIZE           = 16;   // AES-128
const int AES_EXPANDED_KEY_SIZE  = 176;
const int AES_NR                 = 10;   // 10 rondas

#endif // AES_CONSTANTS_H
