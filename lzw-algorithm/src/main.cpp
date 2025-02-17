#include <iostream>
#include <string>
#include "lzw_compressor.h"
#include "lzw_decompressor.h"

int main(int argc, char* argv[])
{
    // Chequeo de argumentos mínimos
    if (argc < 2) {
        std::cout << "Uso: " << argv[0] << " -c <archivo> | -x <archivo> | -h | -v\n";
        return 1;
    }

    // Opción solicitada
    std::string option = argv[1];

    // Menú de ayuda
    if (option == "-h") {
        std::cout << "Opciones:\n"
                  << "  -c <archivo>  : Comprimir con LZW (12 bits)\n"
                  << "  -x <archivo>  : Descomprimir archivo .lzw (12 bits)\n"
                  << "  -v            : Versión\n"
                  << "  -h            : Ayuda\n";
        return 0;
    }
    else if (option == "-v") {
        std::cout << "LZW 12-bit Compressor/Decompressor v1.0\n";
        return 0;
    }
    else if (option == "-c") {
        if (argc < 3) {
            std::cerr << "Falta especificar el archivo a comprimir\n";
            return 1;
        }
        std::string inputFile = argv[2];
        std::string outputFile = inputFile + ".lzw";
        compressLZW(inputFile, outputFile);
    }
    else if (option == "-x") {
        if (argc < 3) {
            std::cerr << "Falta especificar el archivo a descomprimir\n";
            return 1;
        }
        std::string inputFile = argv[2];
        std::string outputFile = inputFile + ".out"; // Nombre sugerido
        decompressLZW(inputFile, outputFile);
    }
    else {
        std::cerr << "Opción no reconocida.\n";
        return 1;
    }

    return 0;
}