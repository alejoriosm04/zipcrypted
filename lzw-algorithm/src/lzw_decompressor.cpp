#include "lzw_decompressor.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static const int MAX_DICT_SIZE = 4096;
static const int CLEAR_CODE    = 256;
static const int FIRST_CODE    = 257;

static bool writeAll(int fd, const void* buffer, size_t count)
{
    const unsigned char* ptr = static_cast<const unsigned char*>(buffer);
    size_t writtenSoFar = 0;
    while (writtenSoFar < count) {
        ssize_t w = write(fd, ptr + writtenSoFar, count - writtenSoFar);
        if (w < 0) {
            std::cerr << "[writeAll] Error al escribir.\n";
            return false;
        }
        if (w == 0) {
            std::cerr << "[writeAll] No se pudo avanzar en la escritura.\n";
            return false;
        }
        writtenSoFar += w;
    }
    return true;
}

static unsigned int bitBuffer = 0;
static int bitCount = 0;

static const int READ_BUF_SIZE = 4096;
static unsigned char readBuffer[READ_BUF_SIZE];
static ssize_t readBufPos = 0;
static ssize_t readBufEnd = 0;

static bool refillBuffer(int fd)
{
    if (readBufPos < readBufEnd) {
        return true;
    }

    ssize_t r = read(fd, readBuffer, READ_BUF_SIZE);
    if (r < 0) {
        std::cerr << "[refillBuffer] Error al leer del archivo.\n";
        return false;
    }
    if (r == 0) {
        return false;
    }
    readBufPos = 0;
    readBufEnd = r;
    return true;
}

static int readCode12(int fd)
{
    while (bitCount < 12) {
        if (!refillBuffer(fd)) {
            if (bitCount == 0) {
                return -1;
            } else {
                int code = bitBuffer & ((1 << bitCount) - 1);
                bitBuffer = 0;
                bitCount  = 0;
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

static void resetDictionary(std::unordered_map<int, std::string> &dict)
{
    dict.clear();
    dict.reserve(MAX_DICT_SIZE);
    for (int i = 0; i < 256; ++i) {
        dict[i] = std::string(1, static_cast<char>(i));
    }
}

void decompressLZW(const std::string &inputFile, const std::string &outputFile)
{
    int fdIn = open(inputFile.c_str(), O_RDONLY);
    if (fdIn < 0) {
        std::cerr << "[decompressLZW] Error: no se pudo abrir el archivo comprimido: "
                  << inputFile << "\n";
        return;
    }

    int fdOut = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fdOut < 0) {
        std::cerr << "[decompressLZW] Error: no se pudo crear/escribir el archivo de salida: "
                  << outputFile << "\n";
        close(fdIn);
        return;
    }

    bitBuffer = 0;
    bitCount  = 0;
    readBufPos = 0;
    readBufEnd = 0;

    std::unordered_map<int, std::string> dictionary;
    resetDictionary(dictionary);
    int nextCode = FIRST_CODE;

    int oldCode = readCode12(fdIn);
    if (oldCode < 0) {
        std::cerr << "[decompressLZW] No hay datos para descomprimir.\n";
        close(fdIn);
        close(fdOut);
        return;
    }

    std::string oldString;
    auto it = dictionary.find(oldCode);
    if (it == dictionary.end()) {
        std::cerr << "[decompressLZW] Primer código inválido.\n";
        close(fdIn);
        close(fdOut);
        return;
    }
    oldString = it->second;

    if (!writeAll(fdOut, oldString.data(), oldString.size())) {
        std::cerr << "[decompressLZW] Error al escribir la primera secuencia.\n";
        close(fdIn);
        close(fdOut);
        return;
    }

    while (true) {
        int newCode = readCode12(fdIn);
        if (newCode < 0) {
            break;
        }

        if (newCode == CLEAR_CODE) {
            resetDictionary(dictionary);
            nextCode = FIRST_CODE;
            oldCode = readCode12(fdIn);
            if (oldCode < 0) {
                break;
            }
            auto it2 = dictionary.find(oldCode);
            if (it2 == dictionary.end()) {
                std::cerr << "[decompressLZW] Código inválido tras reset.\n";
                break;
            }
            oldString = it2->second;
            if (!writeAll(fdOut, oldString.data(), oldString.size())) {
                std::cerr << "[decompressLZW] Error al escribir.\n";
                break;
            }
            continue;
        }

        std::string entry;
        auto itDict = dictionary.find(newCode);
        if (itDict != dictionary.end()) {
            entry = itDict->second;
        } else {
            entry = oldString + oldString[0];
            
            if (nextCode < MAX_DICT_SIZE) {
                dictionary[nextCode] = entry;
                nextCode++;
            }
        }

        if (!writeAll(fdOut, entry.data(), entry.size())) {
            std::cerr << "[decompressLZW] Error escribiendo 'entry'.\n";
            break;
        }

        if (nextCode < MAX_DICT_SIZE) {
            dictionary[nextCode++] = oldString + entry[0];
        }

        oldString = entry;
    }

    close(fdIn);
    close(fdOut);

    std::cout << "[decompressLZW] Descompresión finalizada: " << outputFile << "\n";
}
