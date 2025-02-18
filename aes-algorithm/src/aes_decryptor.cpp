#include "aes_decryptor.h"
#include "Util.h"  
#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <iomanip>

// Tabla inversa de S-Box para la sustitución inversa de bytes
const aes_byte InvSBox[256] = {
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38,
    0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87,
    0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D,
    0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2,
    0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA,
    0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A,
    0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02,
    0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA,
    0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85,
    0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89,
    0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20,
    0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31,
    0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D,
    0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0,
    0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26,
    0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};


std::vector<aes_byte> hexStringToBytes(const std::string &hex) {
    std::vector<aes_byte> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        bytes.push_back(static_cast<aes_byte>(std::stoi(byteString, nullptr, 16)));
    }
    return bytes;
}

// extraer la subclave en cada ronda
static void getRoundKey(const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE], int round, aes_byte roundKey[4][4]) {
    int start = round * AES_BLOCK_SIZE;
    for (int i = 0; i < 16; i++) {
        roundKey[i % 4][i / 4] = expandedKey[start + i];
    }
}

// sustitución inversa usando InvSBox
static void InvSubBytes(aes_byte state[4][4]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] = InvSBox[state[i][j]];
}

// desplazamiento inverso de filas
static void InvShiftRows(aes_byte state[4][4]) {
    aes_byte temp;
    // shift by 1
    temp = state[1][3];
    state[1][3] = state[1][2];
    state[1][2] = state[1][1];
    state[1][1] = state[1][0];
    state[1][0] = temp;

    // shift by 2 
    std::swap(state[2][0], state[2][2]);
    std::swap(state[2][1], state[2][3]);

    // shift by 3
    temp = state[3][0];
    state[3][0] = state[3][1];
    state[3][1] = state[3][2];
    state[3][2] = state[3][3];
    state[3][3] = temp;
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

// mezcla inversa de columnas en la matriz de estado
static void InvMixColumns(aes_byte state[4][4]) {
    aes_byte temp[4];
    for (int c = 0; c < 4; c++) {
        for (int i = 0; i < 4; i++)
            temp[i] = state[i][c];

        state[0][c] = galoisMult(temp[0], 0x0E) ^ galoisMult(temp[1], 0x0B) ^ 
                      galoisMult(temp[2], 0x0D) ^ galoisMult(temp[3], 0x09);
        state[1][c] = galoisMult(temp[0], 0x09) ^ galoisMult(temp[1], 0x0E) ^ 
                      galoisMult(temp[2], 0x0B) ^ galoisMult(temp[3], 0x0D);
        state[2][c] = galoisMult(temp[0], 0x0D) ^ galoisMult(temp[1], 0x09) ^ 
                      galoisMult(temp[2], 0x0E) ^ galoisMult(temp[3], 0x0B);
        state[3][c] = galoisMult(temp[0], 0x0B) ^ galoisMult(temp[1], 0x0D) ^ 
                      galoisMult(temp[2], 0x09) ^ galoisMult(temp[3], 0x0E);
    }
}

static void AddRoundKey(aes_byte state[4][4], aes_byte roundKey[4][4]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] ^= roundKey[i][j];
}


void AES_Decrypt(aes_byte state[4][4], const aes_byte expandedKey[AES_EXPANDED_KEY_SIZE]) {
    aes_byte roundKey[4][4];

    // clave de la última ronda primero
    getRoundKey(expandedKey, AES_NR, roundKey);
    AddRoundKey(state, roundKey);
    printMatrix("After AddRoundKey (last round)", state, "Before Inverse Rounds");

    for (int round = AES_NR - 1; round > 0; round--) {
        InvShiftRows(state);
        printMatrix("After InvShiftRows", state, "Round " + std::to_string(round));
        InvSubBytes(state);
        printMatrix("After InvSubBytes", state, "Round " + std::to_string(round));
        getRoundKey(expandedKey, round, roundKey);
        AddRoundKey(state, roundKey);
        printMatrix("After AddRoundKey", state, "Round " + std::to_string(round));
        InvMixColumns(state);
    }

    InvShiftRows(state);
    printMatrix("After InvShiftRows (final)", state, "Final Round");
    InvSubBytes(state);
    printMatrix("After InvSubBytes (final)", state, "Final Round");
    getRoundKey(expandedKey, 0, roundKey);
    AddRoundKey(state, roundKey);
    printMatrix("After AddRoundKey (final)", state, "Final Round");
}

std::vector<aes_byte> removePKCS7Padding(const std::vector<aes_byte> &input) {
    if (input.empty()) return {};

    size_t padLen = input.back();  
    if (padLen > AES_BLOCK_SIZE || padLen == 0) {
        std::cerr << "Error: Padding incorrecto" << std::endl;
        return input;  
    }

    return std::vector<aes_byte>(input.begin(), input.end() - padLen);
}


