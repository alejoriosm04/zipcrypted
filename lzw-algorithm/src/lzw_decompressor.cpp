#include "lzw_decompressor.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <fcntl.h>       // open
#include <unistd.h>      // read, write, close
#include <sys/types.h>
#include <sys/stat.h>

// ---------------------------
// Constantes y auxiliares
// ---------------------------
static const int MAX_DICT_SIZE = 4096;
static const int CLEAR_CODE = 256;
static const int FIRST_CODE = 257;

static bool writeAll(int fd, const void* buffer, size_t count) {
    const unsigned char* ptr = static_cast<const unsigned char*>(buffer);
    size_t written = 0;
    while(written < count) {
        ssize_t w = write(fd, ptr + written, count - written);
        if(w <= 0) {
            std::cerr << "[writeAll] Error en write.\n";
            return false;
        }
        written += w;
    }
    return true;
}

// ---------------------------
// Módulo de lectura de 12 bits
// ---------------------------
static unsigned int bitBuffer = 0;
static int bitCount = 0;

static const int READ_BUF_SIZE = 4096;
static unsigned char readBuffer[READ_BUF_SIZE];
static ssize_t readBufPos = 0;
static ssize_t readBufEnd = 0;

static bool refillBuffer(int fd) {
    if(readBufPos < readBufEnd) return true;
    ssize_t r = read(fd, readBuffer, READ_BUF_SIZE);
    if(r < 0) {
        std::cerr << "[refillBuffer] Error al leer.\n";
        return false;
    }
    if(r == 0) return false;
    readBufPos = 0;
    readBufEnd = r;
    return true;
}

static int readCode12(int fd) {
    while(bitCount < 12) {
        if(!refillBuffer(fd)) {
            if(bitCount == 0) return -1;
            else {
                int code = bitBuffer & ((1 << bitCount)-1);
                bitBuffer = 0;
                bitCount = 0;
                return code;
            }
        }
        unsigned char byteVal = readBuffer[readBufPos++];
        bitBuffer |= (byteVal << bitCount);
        bitCount += 8;
    }
    int code = bitBuffer & 0xFFF;
    bitBuffer >>= 12;
    bitCount -= 12;
    return code;
}

// ---------------------------
// Resetear el diccionario
// ---------------------------
static void resetDictionary(std::unordered_map<int, std::string>& dict) {
    dict.clear();
    dict.reserve(MAX_DICT_SIZE);
    for(int i = 0; i < 256; ++i)
        dict[i] = std::string(1, static_cast<char>(i));
}

// ---------------------------
// Descompresión LZW principal
// ---------------------------
void decompressLZW(const std::string &inputFile, const std::string &outputFile) {
    int fdIn = open(inputFile.c_str(), O_RDONLY);
    if(fdIn < 0) {
        std::cerr << "[decompressLZW] Error al abrir archivo comprimido: " << inputFile << "\n";
        return;
    }
    int fdOut = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fdOut < 0) {
        std::cerr << "[decompressLZW] Error al abrir archivo de salida: " << outputFile << "\n";
        close(fdIn);
        return;
    }

    // Reiniciar buffers globales de lectura
    bitBuffer = 0; bitCount = 0; readBufPos = 0; readBufEnd = 0;

    std::unordered_map<int, std::string> dict;
    resetDictionary(dict);
    int nextCode = FIRST_CODE;

    int oldCode = readCode12(fdIn);
    if(oldCode < 0) {
        std::cerr << "[decompressLZW] Archivo comprimido vacío o corrupto.\n";
        close(fdIn); close(fdOut);
        return;
    }
    std::string oldString = dict[oldCode];
    if(!writeAll(fdOut, oldString.data(), oldString.size())) {
        std::cerr << "[decompressLZW] Error al escribir la primera secuencia.\n";
        close(fdIn); close(fdOut);
        return;
    }

    while(true) {
        int newCode = readCode12(fdIn);
        if(newCode < 0) break;

        if(newCode == CLEAR_CODE) {
            resetDictionary(dict);
            nextCode = FIRST_CODE;
            oldCode = readCode12(fdIn);
            if(oldCode < 0) break;
            oldString = dict[oldCode];
            if(!writeAll(fdOut, oldString.data(), oldString.size())) {
                std::cerr << "[decompressLZW] Error al escribir después del CLEAR_CODE.\n";
                break;
            }
            continue;
        }

        std::string entry;
        if(dict.find(newCode) != dict.end()) {
            entry = dict[newCode];
        } else {
            // Caso especial: newCode no existe aún en el diccionario
            entry = oldString + oldString[0];
        }

        if(!writeAll(fdOut, entry.data(), entry.size())) {
            std::cerr << "[decompressLZW] Error al escribir 'entry'.\n";
            break;
        }

        // Agregar nueva entrada al diccionario
        if(nextCode < MAX_DICT_SIZE) {
            dict[nextCode++] = oldString + entry[0];
        }
        oldString = entry;
    }

    close(fdIn);
    close(fdOut);
    std::cout << "[decompressLZW] Descompresión exitosa: " << outputFile << "\n";
}
