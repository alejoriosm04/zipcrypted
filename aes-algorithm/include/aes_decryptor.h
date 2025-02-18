#ifndef AES_DECRYPTOR_H
#define AES_DECRYPTOR_H

#include "aes_constants.h" 
#include <vector>
#include <string>

void AES_Decrypt(aes_byte state[4][4], const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE]);

std::vector<aes_byte> hexStringToBytes(const std::string &hex);

std::vector<aes_byte> removePKCS7Padding(const std::vector<aes_byte> &input);

#endif 
