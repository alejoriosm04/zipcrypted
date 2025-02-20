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

static bool writeAll(int fd, const void* buffer, size_t count) {
    const unsigned char* ptr = static_cast<const unsigned char*>(buffer);
    size_t written = 0;
    while (written < count) {
        ssize_t w = write(fd, ptr + written, count - written);
        if (w <= 0) {
            std::cerr << "[writeAll] Error en write().\n";
            return false;
        }
        written += w;
    }
    return true;
}

static bool isBinaryFile(int fd) {
    unsigned char buf[SAMPLE_SIZE];
    ssize_t n = read(fd, buf, SAMPLE_SIZE);
    if(n <= 0) { lseek(fd, 0, SEEK_SET); return false; }
    int nonPrintable = 0;
    for(int i = 0; i < n; ++i)
        if((buf[i] < 32 || buf[i] > 126) && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\t')
            nonPrintable++;
    lseek(fd, 0, SEEK_SET);
    return (double(nonPrintable)/n > 0.30);
}

static unsigned int bitBuffer = 0;
static int bitCount = 0;

static bool flushOutputBits(int fd) {
    while(bitCount >= 8) {
        int shift = bitCount - 8;
        unsigned char byteOut = (bitBuffer >> shift) & 0xFF;
        if (!writeAll(fd, &byteOut, 1)) return false;
        bitCount -= 8;
        bitBuffer &= ((1 << bitCount) - 1);
    }
    return true;
}

static bool writeCode12(int fd, int code) {
    code &= 0xFFF;
    bitBuffer = (bitBuffer << 12) | code;
    bitCount += 12;
    return flushOutputBits(fd);
}

static bool flushRemainingBits(int fd) {
    if(bitCount > 0) {
        unsigned char outByte = (bitBuffer << (8 - bitCount)) & 0xFF;
        if (!writeAll(fd, &outByte, 1)) return false;
    }
    bitBuffer = 0;
    bitCount = 0;
    return true;
}

static int resetDictionary(std::unordered_map<std::string,int> &dict) {
    dict.clear();
    dict.reserve(MAX_DICT_SIZE);
    for(int i = 0; i < 256; ++i)
        dict[std::string(1, static_cast<char>(i))] = i;
    return 257;
}

void compressLZW(const std::string &inputFile, const std::string &outputFile) {
    int fdIn = open(inputFile.c_str(), O_RDONLY);
    if(fdIn < 0) {
        std::cerr << "[compressLZW] Error al abrir entrada: " << inputFile << "\n";
        return;
    }
    int fdOut = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fdOut < 0) {
        std::cerr << "[compressLZW] Error al abrir salida: " << outputFile << "\n";
        close(fdIn);
        return;
    }

    if(isBinaryFile(fdIn))
        std::cout << "[Info] Archivo detectado como BINARIO.\n";
    else
        std::cout << "[Info] Archivo detectado como TEXTO (o mayormente texto).\n";

    std::unordered_map<std::string,int> dict;
    int nextCode = resetDictionary(dict);

    unsigned char buffer;
    ssize_t n = read(fdIn, &buffer, 1);
    if(n <= 0) {
        std::cerr << "[compressLZW] Archivo vacío o error de lectura.\n";
        close(fdIn); close(fdOut);
        return;
    }
    std::string W(1, static_cast<char>(buffer));

    while(true) {
        n = read(fdIn, &buffer, 1);
        if(n < 0) {
            std::cerr << "[compressLZW] Error de lectura.\n";
            break;
        }
        if(n == 0) {
            int code = dict[W];
            if(!writeCode12(fdOut, code))
                std::cerr << "[compressLZW] Error escribiendo código final.\n";
            break;
        }
        std::string WK = W + static_cast<char>(buffer);
        if(dict.find(WK) != dict.end()) {
            W = WK;
        } else {
            int code = dict[W];
            if(!writeCode12(fdOut, code)) {
                std::cerr << "[compressLZW] Error escribiendo código.\n";
                break;
            }
            if(nextCode < MAX_DICT_SIZE)
                dict[WK] = nextCode++;
            else {
                if(!writeCode12(fdOut, CLEAR_CODE))
                    std::cerr << "[compressLZW] Error escribiendo CLEAR_CODE.\n";
                nextCode = resetDictionary(dict);
            }
            W = std::string(1, static_cast<char>(buffer));
        }
    }

    if(!flushRemainingBits(fdOut))
        std::cerr << "[compressLZW] Error al vaciar bits restantes.\n";

    close(fdIn);
    close(fdOut);
    std::cout << "[compressLZW] Compresión exitosa: " << outputFile << "\n";
}