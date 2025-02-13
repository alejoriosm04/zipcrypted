#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cstring>

using namespace std;

// Definición de tipos y constantes
using aes_byte = unsigned char;

const int Nb = 4;   // Número de columnas en el estado (siempre 4 en AES)
const int Nk = 4;   // Número de palabras de la clave (4 para AES-128)
const int Nr = 10;  // Número de rondas (10 para AES-128)

// Tabla S-Box para la sustitución de bytes
const aes_byte SBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

void KeyExpansion(aes_byte key[16], aes_byte expandedKey[176]) {
    for (int i = 0; i < 16; i++) {
        expandedKey[i] = key[i];
    }
    for (int i = 16; i < 176; i++) {
        aes_byte temp = expandedKey[i - 1];
        if (i % 16 == 0) {
            temp = SBox[temp];
        }
        expandedKey[i] = expandedKey[i - 16] ^ temp;
    }
}

void AddRoundKey(aes_byte state[4][4], aes_byte roundKey[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] ^= roundKey[i][j];
        }
    }
}

// Función para multiplicar en el campo de Galois GF(2^8)
aes_byte galoisMult(aes_byte a, aes_byte b) {
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

void MixColumns(aes_byte state[4][4]) {
    aes_byte temp[4];
    for (int c = 0; c < 4; c++) {
        for (int i = 0; i < 4; i++) {
            temp[i] = state[i][c];
        }
        state[0][c] = galoisMult(temp[0], 2) ^ galoisMult(temp[1], 3) ^ temp[2] ^ temp[3];
        state[1][c] = temp[0] ^ galoisMult(temp[1], 2) ^ galoisMult(temp[2], 3) ^ temp[3];
        state[2][c] = temp[0] ^ temp[1] ^ galoisMult(temp[2], 2) ^ galoisMult(temp[3], 3);
        state[3][c] = galoisMult(temp[0], 3) ^ temp[1] ^ temp[2] ^ galoisMult(temp[3], 2);
    }
}

void ShiftRows(aes_byte state[4][4]) {
    aes_byte temp;
    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;
    
    swap(state[2][0], state[2][2]);
    swap(state[2][1], state[2][3]);
    
    temp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = temp;
}

void SubBytes(aes_byte state[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = SBox[state[i][j]];
        }
    }
}

void printState(aes_byte state[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%02x ", state[i][j]);
        }
        printf("\n");
    }
}

// Extrae una subclave (round key) de 16 bytes de la clave expandida y la organiza en una matriz 4x4.
void getRoundKey(const aes_byte expandedKey[176], int round, aes_byte roundKey[4][4]) {
    // Cada ronda usa 16 bytes: ronda 0 usa bytes 0-15, ronda 1 usa bytes 16-31, etc.
    int start = round * 16;
    for (int i = 0; i < 16; i++) {
        // Organiza la clave en formato columna (4x4)
        roundKey[i % 4][i / 4] = expandedKey[start + i];
    }
}

void AES_Encrypt(aes_byte state[4][4], const aes_byte expandedKey[176]) {
    aes_byte roundKey[4][4];
    
    // Ronda inicial: se utiliza la clave original (primer bloque de 16 bytes)
    getRoundKey(expandedKey, 0, roundKey);
    AddRoundKey(state, roundKey);
    
    // Rondas 1 a 9
    for (int round = 1; round < Nr; round++) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        getRoundKey(expandedKey, round, roundKey);
        AddRoundKey(state, roundKey);
    }
    
    // Ronda final (ronda 10): sin MixColumns
    SubBytes(state);
    ShiftRows(state);
    getRoundKey(expandedKey, Nr, roundKey);
    AddRoundKey(state, roundKey);
}

int main() {
    // 1. Definir la clave original (16 bytes para AES-128)
    aes_byte key[16] = {
        0x2b, 0x28, 0xab, 0x09,
        0x7e, 0xae, 0xf7, 0xcf,
        0x15, 0xd2, 0x15, 0x4f,
        0x16, 0xa6, 0x88, 0x3c
    };

    // 2. Generar la clave expandida (176 bytes)
    aes_byte expandedKey[176];
    KeyExpansion(key, expandedKey);

    cout << "Clave expandida (KeyExpansion):" << endl;
    for (int i = 0; i < 176; i++) {
        printf("%02x ", expandedKey[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    cout << endl;

    // 3. Definir el bloque de datos (estado) a cifrar (16 bytes organizados en matriz 4x4)
    aes_byte state[4][4] = {
        {0x32, 0x43, 0xf6, 0xa8},
        {0x88, 0x5a, 0x30, 0x8d},
        {0x31, 0x31, 0x98, 0xa2},
        {0xe0, 0x37, 0x07, 0x34}
    };

    cout << "Estado inicial:" << endl;
    printState(state);
    cout << endl;

    // 4. Cifrar el estado completo utilizando AES_Encrypt
    AES_Encrypt(state, expandedKey);

    cout << "Estado final (cifrado):" << endl;
    printState(state);
    cout << endl;

    return 0;
}
