#include "ManejadorArchivo.h"
#include "aes_encryptor.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

// Lee el archivo en modo binario y almacena su contenido en 'datos'
bool ManejadorArchivo::leerArchivo(const std::string &nombreArchivo) {
    int fd = open(nombreArchivo.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error: No se pudo abrir el archivo " << nombreArchivo << std::endl;
        return false;
    }
    datos.clear();
    uint8_t buffer[1024];
    ssize_t bytesLeidos;
    while ((bytesLeidos = read(fd, buffer, sizeof(buffer))) > 0) {
        datos.insert(datos.end(), buffer, buffer + bytesLeidos);
    }
    close(fd);
    return true;
}

// Encripta el contenido leído y retorna el ciphertext en hexadecimal
std::string ManejadorArchivo::encriptarArchivo(const std::string &clave) {
    // Convertir los datos leídos a string (se asume que es texto)
    std::string inputText(datos.begin(), datos.end());
    // Aplicar padding PKCS#7
    std::vector<aes_byte> paddedData = applyPKCS7Padding(inputText);
    size_t totalBlocks = paddedData.size() / AES_BLOCK_SIZE;

    // Preparar la clave: se toma la clave dada y se asegura que tenga 16 bytes
    aes_byte key[AES_KEY_SIZE] = {0};
    memcpy(key, clave.c_str(), std::min((size_t)AES_KEY_SIZE, clave.size()));

    // Generar la clave expandida
    aes_byte expandedKey[AES_EXPANDED_KEY_SIZE];
    KeyExpansion(key, expandedKey);

    // Encriptar cada bloque
    std::vector<aes_byte> cipherData(totalBlocks * AES_BLOCK_SIZE);
    for (size_t block = 0; block < totalBlocks; block++) {
        aes_byte blockIn[AES_BLOCK_SIZE], blockOut[AES_BLOCK_SIZE];
        aes_byte state[4][4];
        memcpy(blockIn, &paddedData[block * AES_BLOCK_SIZE], AES_BLOCK_SIZE);
        arrayToState(blockIn, state);
        AES_Encrypt(state, expandedKey);
        stateToArray(state, blockOut);
        memcpy(&cipherData[block * AES_BLOCK_SIZE], blockOut, AES_BLOCK_SIZE);
    }

    // Convertir el ciphertext a cadena hexadecimal
    std::string hexOutput;
    char bufferHex[3];
    for (aes_byte byte : cipherData) {
        sprintf(bufferHex, "%02x", byte);
        hexOutput += bufferHex;
    }
    return hexOutput;
}

bool ManejadorArchivo::escribirEncriptacion(const std::string &nombreArchivo, const std::string &contenidoHex) {
    int fd = open(nombreArchivo.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Error: No se pudo crear el archivo de encriptación " << nombreArchivo << std::endl;
        return false;
    }
    ssize_t bytesEscritos = write(fd, contenidoHex.c_str(), contenidoHex.size());
    if (bytesEscritos == -1) {
        std::cerr << "Error: No se pudo escribir en el archivo de encriptación " << nombreArchivo << std::endl;
        close(fd);
        return false;
    }
    close(fd);
    return true;
}
