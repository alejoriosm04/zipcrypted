#ifndef AES_DECRYPTOR_H
#define AES_DECRYPTOR_H

#include "aes_constants.h" 
#include <vector>
#include <string>

// Expande la clave original (16 bytes) a 176 bytes

// Descifra un bloque de 16 bytes (estado 4x4) usando AES-128
void AES_Decrypt(aes_byte state[4][4], const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE]);

// Convierte una cadena hexadecimal a un vector de bytes
std::vector<aes_byte> hexStringToBytes(const std::string &hex);

// Remueve el padding PKCS#7 después de la desencriptación
std::vector<aes_byte> removePKCS7Padding(const std::vector<aes_byte> &input);

#endif // AES_DECRYPTOR_H
