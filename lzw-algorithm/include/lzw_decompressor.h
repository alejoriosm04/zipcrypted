#pragma once

#include <string>

/**
 * @brief Descomprime un archivo usando LZW (12 bits, reset) y llamadas al sistema.
 * 
 * @param inputFile  Ruta del archivo .lzw
 * @param outputFile Ruta del archivo donde se escribirá el contenido original.
 *
 * Requisitos:
 *  - Uso de llamadas al sistema (open, read, write, close).
 *  - No uso de librerías externas.
 *  - Manejo de 12 bits por código, con diccionario que se resetea (CLEAR_CODE=256).
 */
void decompressLZW(const std::string &inputFile, const std::string &outputFile);