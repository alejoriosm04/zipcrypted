#include "aes_encryptor.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

// S-Box para sustitución de bytes
const aes_byte SBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Tabla de constantes Rcon
const aes_byte Rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

// Declaración adelantada de la función auxiliar para extraer la subclave
static void getRoundKey(const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE], int round, aes_byte roundKey[4][4]);

// Expande la clave original (16 bytes) a 176 bytes
void KeyExpansion(aes_byte key[AES_KEY_SIZE], aes_byte expandedKey[AES_EXPANDED_KEY_SIZE]) {
    memcpy(expandedKey, key, AES_KEY_SIZE);
    int bytesGenerated = AES_KEY_SIZE;
    int rconIteration = 1;
    aes_byte temp[4];

    while (bytesGenerated < AES_EXPANDED_KEY_SIZE) {
        for (int i = 0; i < 4; i++)
            temp[i] = expandedKey[bytesGenerated - 4 + i];

        if (bytesGenerated % AES_KEY_SIZE == 0) {
            // rotWord
            aes_byte t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            temp[0] = SBox[temp[0]];
            temp[1] = SBox[temp[1]];
            temp[2] = SBox[temp[2]];
            temp[3] = SBox[temp[3]];
            // XOR con Rcon
            temp[0] ^= Rcon[rconIteration - 1];
            rconIteration++;
        }
        for (int i = 0; i < 4; i++) {
            expandedKey[bytesGenerated] = expandedKey[bytesGenerated - AES_KEY_SIZE] ^ temp[i];
            bytesGenerated++;
        }
    }
}

static void AddRoundKey(aes_byte state[4][4], aes_byte roundKey[4][4]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] ^= roundKey[i][j];
}

static aes_byte galoisMult(aes_byte a, aes_byte b) {
    aes_byte p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1)
            p ^= a;
        bool highBit = (a & 0x80);
        a <<= 1;
        if (highBit)
            a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

static void MixColumns(aes_byte state[4][4]) {
    aes_byte temp[4];
    for (int c = 0; c < 4; c++) {
        for (int i = 0; i < 4; i++)
            temp[i] = state[i][c];
        state[0][c] = galoisMult(temp[0], 2) ^ galoisMult(temp[1], 3) ^ temp[2] ^ temp[3];
        state[1][c] = temp[0] ^ galoisMult(temp[1], 2) ^ galoisMult(temp[2], 3) ^ temp[3];
        state[2][c] = temp[0] ^ temp[1] ^ galoisMult(temp[2], 2) ^ galoisMult(temp[3], 3);
        state[3][c] = galoisMult(temp[0], 3) ^ temp[1] ^ temp[2] ^ galoisMult(temp[3], 2);
    }
}

static void ShiftRows(aes_byte state[4][4]) {
    aes_byte temp;
    // Fila 1: left shift by 1
    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;
    
    // Fila 2: left shift by 2 (intercambio)
    std::swap(state[2][0], state[2][2]);
    std::swap(state[2][1], state[2][3]);
    
    // Fila 3: left shift by 3
    temp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = temp;
}

static void SubBytes(aes_byte state[4][4]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] = SBox[state[i][j]];
}

void arrayToState(aes_byte input[AES_BLOCK_SIZE], aes_byte state[4][4]) {
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            state[row][col] = input[col * 4 + row];
}

void stateToArray(aes_byte state[4][4], aes_byte output[AES_BLOCK_SIZE]) {
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            output[col * 4 + row] = state[row][col];
}

// Función auxiliar para extraer la subclave (round key) del key expandido
static void getRoundKey(const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE], int round, aes_byte roundKey[4][4]) {
    int start = round * AES_BLOCK_SIZE;
    for (int i = 0; i < 16; i++) {
        roundKey[i % 4][i / 4] = expandedKey[start + i];
    }
}

// Función principal de encriptación AES-128 (modo ECB)
void AES_Encrypt(aes_byte state[4][4], const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE]) {
    aes_byte roundKey[4][4];
    // Ronda inicial
    getRoundKey(expandedKey, 0, roundKey);
    AddRoundKey(state, roundKey);
    
    // Rondas 1 a 9
    for (int round = 1; round < AES_NR; round++) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        getRoundKey(expandedKey, round, roundKey);
        AddRoundKey(state, roundKey);
    }
    
    // Ronda final (sin MixColumns)
    SubBytes(state);
    ShiftRows(state);
    getRoundKey(expandedKey, AES_NR, roundKey);
    AddRoundKey(state, roundKey);
}

// Aplica padding PKCS#7 a un string y retorna un vector de bytes
std::vector<aes_byte> applyPKCS7Padding(const std::string &input) {
    size_t blockSize = AES_BLOCK_SIZE;
    size_t inputLen = input.size();
    size_t padLen = blockSize - (inputLen % blockSize);
    if(padLen == 0)
        padLen = blockSize;
    std::vector<aes_byte> padded(input.begin(), input.end());
    for (size_t i = 0; i < padLen; i++) {
        padded.push_back(static_cast<aes_byte>(padLen));
    }
    return padded;
}
