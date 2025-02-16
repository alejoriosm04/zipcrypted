#ifndef AES_ENCRYPTOR_H
#define AES_ENCRYPTOR_H

#include <vector>
#include <string>

// Definiciones básicas del algoritmo AES
typedef unsigned char aes_byte;
const int AES_BLOCK_SIZE         = 16;
const int AES_KEY_SIZE           = 16;   // AES-128
const int AES_EXPANDED_KEY_SIZE  = 176;
const int AES_NR                 = 10;   // 10 rondas

// Expande la clave original (16 bytes) a 176 bytes
void KeyExpansion(aes_byte key[AES_KEY_SIZE], aes_byte expandedKey[AES_EXPANDED_KEY_SIZE]);

// Cifra un bloque de 16 bytes (estado 4x4) usando AES-128
void AES_Encrypt(aes_byte state[4][4], const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE]);

// Convierte un arreglo lineal de 16 bytes a una matriz 4x4 (por columnas)
void arrayToState(aes_byte input[AES_BLOCK_SIZE], aes_byte state[4][4]);

// Convierte una matriz 4x4 a un arreglo lineal de 16 bytes
void stateToArray(aes_byte state[4][4], aes_byte output[AES_BLOCK_SIZE]);

// Aplica padding PKCS#7 a un string y retorna un vector de bytes resultante
std::vector<aes_byte> applyPKCS7Padding(const std::string &input);

#endif // AES_H
