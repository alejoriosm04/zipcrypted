#include "ManejadorArchivo.h"
#include "aes_encryptor.h"
#include "aes_decryptor.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

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

// Función auxiliar para aplicar padding PKCS#7 a un vector de bytes (para archivos binarios)
static std::vector<aes_byte> applyPKCS7PaddingBinary(const std::vector<aes_byte>& data) {
    size_t blockSize = AES_BLOCK_SIZE;
    size_t inputLen = data.size();
    size_t padLen = blockSize - (inputLen % blockSize);
    if (padLen == 0)
        padLen = blockSize; // Siempre se agrega un bloque de padding si ya es múltiplo
    std::vector<aes_byte> padded = data;
    for (size_t i = 0; i < padLen; i++) {
        padded.push_back(static_cast<aes_byte>(padLen));
    }
    return padded;
}

// --- Modificación en la función encriptarArchivo ---
std::string ManejadorArchivo::encriptarArchivo(const std::string &clave) {
    // Ahora se utiliza directamente el vector de bytes 'datos'
    // Aplicar padding PKCS#7 a los datos binarios
    std::vector<aes_byte> paddedData = applyPKCS7PaddingBinary(datos);
    size_t totalBlocks = paddedData.size() / AES_BLOCK_SIZE;

    // Preparar la clave: se copia la clave dada y se asegura que tenga 16 bytes (rellenando con ceros si es necesario)
    aes_byte key[AES_KEY_SIZE] = {0};
    memcpy(key, clave.c_str(), std::min((size_t)AES_KEY_SIZE, clave.size()));

    std::cout << "Clave (hexadecimal): ";
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        printf("%02x", key[i]);
    }
    std::cout << std::endl;

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

    // Convertir el ciphertext a una cadena hexadecimal
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

bool ManejadorArchivo::escribirDesencriptacion(const std::string &nombreArchivo, const std::string &contenido) {
    // Abrir archivo en modo texto (o binario, pero aquí conviene texto si es un archivo textual)
    int fd = open(nombreArchivo.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Error: No se pudo crear el archivo " << nombreArchivo << std::endl;
        return false;
    }
    // Escribir el contenido desencriptado como texto
    ssize_t bytesEscritos = write(fd, contenido.c_str(), contenido.size());
    if (bytesEscritos == -1) {
        std::cerr << "Error: No se pudo escribir en el archivo " << nombreArchivo << std::endl;
        close(fd);
        return false;
    }
    close(fd);
    return true;
}


// Desencripta el archivo encriptado
std::string ManejadorArchivo::desencriptarArchivo(const std::string &clave) {
    // Convertir el contenido del archivo (hexadecimal) a bytes
    std::string encryptedHex(datos.begin(), datos.end());

    std::vector<aes_byte> encryptedBytes = hexStringToBytes(encryptedHex);
    if (encryptedBytes.empty()) {
        std::cerr << "Error: No se pudo convertir el hexadecimal en bytes" << std::endl;
        return "";
    }

    size_t totalBlocks = encryptedBytes.size() / AES_BLOCK_SIZE;

    // Preparar la clave de desencriptación
    aes_byte key[AES_KEY_SIZE] = {0};
    memcpy(key, clave.c_str(), std::min((size_t)AES_KEY_SIZE, clave.size()));

    // Expansión de la clave AES
    aes_byte expandedKey[AES_EXPANDED_KEY_SIZE];
    KeyExpansion(key, expandedKey);

    // Vector para almacenar la salida desencriptada
    std::vector<aes_byte> plainData(totalBlocks * AES_BLOCK_SIZE);

    // Desencriptar bloque por bloque
    for (size_t block = 0; block < totalBlocks; block++) {
        aes_byte blockIn[AES_BLOCK_SIZE], blockOut[AES_BLOCK_SIZE];
        aes_byte state[4][4];

        memcpy(blockIn, &encryptedBytes[block * AES_BLOCK_SIZE], AES_BLOCK_SIZE);
        arrayToState(blockIn, state);
        AES_Decrypt(state, expandedKey);
        stateToArray(state, blockOut);

        memcpy(&plainData[block * AES_BLOCK_SIZE], blockOut, AES_BLOCK_SIZE);
    }

    // Remover padding PKCS#7
    std::vector<aes_byte> plainBytes = removePKCS7Padding(plainData);
    if (plainBytes.empty()) {
        std::cerr << "Error: La salida desencriptada está vacía después de remover padding" << std::endl;
        return "";
    }

    // Convertir a string y devolver el resultado
    return std::string(plainBytes.begin(), plainBytes.end());
}