#pragma once

#include <string>

/**
 * @brief Comprime un archivo usando LZW con empaquetado de 12 bits, 
 *        reseteo del diccionario, y manejo robusto de E/S.
 *
 * @param inputFile  Ruta del archivo original a comprimir.
 * @param outputFile Ruta del archivo comprimido de salida.
 *
 * Requisitos cumplidos:
 *  - Uso de llamadas al sistema (open, read, write, close).
 *  - No uso de librerías externas de compresión.
 *  - Manejo de archivo binario o de texto (8 bits).
 *  - Empaquetado de bits para códigos de 12 bits.
 *  - Diccionario se resetea al llenarse (4096 entradas).
 *  - Manejo más robusto de errores en lectura/escritura.
 */
void compressLZW(const std::string &inputFile, const std::string &outputFile);