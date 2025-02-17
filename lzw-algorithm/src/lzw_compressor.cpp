#include "lzw_compressor.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <fcntl.h>       // open
#include <unistd.h>     
#include <sys/types.h>
#include <sys/stat.h>

// ---------------------------
// Constantes y auxiliares
// ---------------------------
static const int MAX_DICT_SIZE = 4096;   // 12 bits -> 4096 entradas
static const size_t SAMPLE_SIZE = 1000;    // para detección de binario
static const int CLEAR_CODE = 256;         // Código de reinicio del diccionario

// Función para escribir todos los bytes solicitados (manejo de escrituras parciales)
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

// Detecta si el archivo es mayormente binario (informativo)
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

// ---------------------------
// Empaquetado de 12 bits
// ---------------------------
static unsigned int bitBuffer = 0; // Buffer para acumular bits
static int bitCount = 0;           // Número de bits almacenados en bitBuffer

// Envía a disco mientras hayan 8 bits completos
static bool flushOutputBits(int fd) {
    while(bitCount >= 8) {
        unsigned char byteOut = bitBuffer & 0xFF;
        if (!writeAll(fd, &byteOut, 1)) return false;
        bitBuffer >>= 8;
        bitCount -= 8;
    }
    return true;
}

// Escribe un código de 12 bits en el flujo
static bool writeCode12(int fd, int code) {
    code &= 0xFFF; // Asegurar 12 bits
    bitBuffer |= (code << bitCount);
    bitCount += 12;
    return flushOutputBits(fd);
}

// Vacía los bits restantes (rellena a la izquierda para completar el byte)
static bool flushRemainingBits(int fd) {
    if(bitCount > 0) {
        unsigned char outByte = (bitBuffer & ((1 << bitCount)-1)) << (8 - bitCount);
        if (!writeAll(fd, &outByte, 1)) return false;
    }
    bitBuffer = 0;
    bitCount = 0;
    return true;
}

// ---------------------------
// Resetear el diccionario
// ---------------------------
static int resetDictionary(std::unordered_map<std::string,int> &dict) {
    dict.clear();
    dict.reserve(MAX_DICT_SIZE);
    for(int i = 0; i < 256; ++i)
        dict[std::string(1, static_cast<char>(i))] = i;
    return 257;  // Después de 0..255, reservamos CLEAR_CODE en 256
}

// ---------------------------
// Compresión LZW principal
// ---------------------------
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

    // Informe sobre tipo de archivo
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
        if(n == 0) { // Fin de archivo
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
            // Agregar WK al diccionario o emitir CLEAR_CODE si está lleno
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