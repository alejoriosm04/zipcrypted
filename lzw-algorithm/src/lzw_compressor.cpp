#include "lzw_compressor.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static const int MAX_DICT_SIZE = 4096;
static const size_t SAMPLE_SIZE = 1000; 
static const int CLEAR_CODE = 256;

static bool writeAll(int fd, const void* buffer, size_t count)
{
    const unsigned char* ptr = static_cast<const unsigned char*>(buffer);
    size_t writtenSoFar = 0;
    while (writtenSoFar < count) {
        ssize_t w = write(fd, ptr + writtenSoFar, count - writtenSoFar);
        if (w < 0) {
            std::cerr << "[writeAll] Error en write().\n";
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

static bool isBinaryFile(int fd)
{
    unsigned char buffer[SAMPLE_SIZE];
    ssize_t bytesRead = read(fd, buffer, SAMPLE_SIZE);
    if (bytesRead <= 0) {
        lseek(fd, 0, SEEK_SET);
        return false;
    }

    int nonPrintableCount = 0;
    for (int i = 0; i < bytesRead; ++i) {
        unsigned char c = buffer[i];
        if ((c < 32 || c > 126) && c != '\n' && c != '\r' && c != '\t') {
            nonPrintableCount++;
        }
    }

    lseek(fd, 0, SEEK_SET);

    double ratio = (double)nonPrintableCount / (double)bytesRead;
    return (ratio > 0.30);
}

static unsigned int bitBuffer = 0;
static int bitCount = 0;

static bool flushOutputBits(int fd)
{
    while (bitCount >= 8) {
        unsigned char outByte = bitBuffer & 0xFF;
        if (!writeAll(fd, &outByte, 1)) {
            return false;
        }
        bitBuffer >>= 8;
        bitCount -= 8;
    }
    return true;
}

static bool writeCode12(int fd, int code)
{
    code &= 0xFFF;
    bitBuffer |= (code << bitCount);
    bitCount += 12;
    if (!flushOutputBits(fd)) {
        return false;
    }
    return true;
}

static bool flushRemainingBits(int fd)
{
    if (bitCount > 0) {
        unsigned char outByte = bitBuffer & ((1 << bitCount) - 1);
        if (!writeAll(fd, &outByte, 1)) {
            return false;
        }
    }
    bitBuffer = 0;
    bitCount  = 0;
    return true;
}

static int resetDictionary(std::unordered_map<std::string,int> &dictionary)
{
    dictionary.clear();
    dictionary.reserve(MAX_DICT_SIZE);

    for (int i = 0; i < 256; ++i) {
        std::string ch(1, static_cast<char>(i));
        dictionary[ch] = i;
    }
    return 257;
}

void compressLZW(const std::string &inputFile, const std::string &outputFile)
{
    int fdIn = open(inputFile.c_str(), O_RDONLY);
    if (fdIn < 0) {
        std::cerr << "[compressLZW] Error: no se pudo abrir el archivo de entrada: "
                  << inputFile << "\n";
        return;
    }

    int fdOut = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fdOut < 0) {
        std::cerr << "[compressLZW] Error: no se pudo crear/abrir el archivo de salida: "
                  << outputFile << "\n";
        close(fdIn);
        return;
    }

    bool isBin = isBinaryFile(fdIn);
    if (isBin) {
        std::cout << "[Info] Archivo detectado como BINARIO.\n";
    } else {
        std::cout << "[Info] Archivo detectado como TEXTO (o mayormente texto).\n";
    }

    std::unordered_map<std::string, int> dictionary;
    int nextCode = resetDictionary(dictionary);

    unsigned char buffer;
    ssize_t bytesRead = read(fdIn, &buffer, 1);
    if (bytesRead < 0) {
        std::cerr << "[compressLZW] Error de lectura en el archivo: " << inputFile << "\n";
        close(fdIn);
        close(fdOut);
        return;
    }
    if (bytesRead == 0) {
        std::cerr << "[compressLZW] El archivo está vacío, no se genera salida.\n";
        close(fdIn);
        close(fdOut);
        return;
    }

    std::string W(1, static_cast<char>(buffer));

    while (true) {
        bytesRead = read(fdIn, &buffer, 1);
        if (bytesRead < 0) {
            std::cerr << "[compressLZW] Error de lectura.\n";
            close(fdIn);
            close(fdOut);
            return;
        }
        if (bytesRead == 0) {
            int code = dictionary[W];
            if (!writeCode12(fdOut, code)) {
                std::cerr << "[compressLZW] Error escribiendo el código final.\n";
            }
            break;
        }

        std::string WK = W + static_cast<char>(buffer);

        auto it = dictionary.find(WK);
        if (it != dictionary.end()) {
            W = WK;
        } else {
            int code = dictionary[W];
            if (!writeCode12(fdOut, code)) {
                std::cerr << "[compressLZW] Error escribiendo código.\n";
            }

            if (nextCode < MAX_DICT_SIZE) {
                dictionary[WK] = nextCode++;
            } else {
                if (!writeCode12(fdOut, CLEAR_CODE)) {
                std::cerr << "[compressLZW] Error escribiendo CLEAR_CODE.\n";
                }
                nextCode = resetDictionary(dictionary);
            }

            W = std::string(1, static_cast<char>(buffer));
        }
    }

    if (!flushRemainingBits(fdOut)) {
        std::cerr << "[compressLZW] Error al volcar bits restantes.\n";
    }

    close(fdIn);
    close(fdOut);

    std::cout << "[compressLZW] Archivo comprimido (12 bits) exitosamente: " 
              << outputFile << "\n";
}
