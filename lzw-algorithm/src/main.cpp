#include <iostream>
#include <string>
#include <filesystem>
#include "lzw_compressor.h"
#include "lzw_decompressor.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Uso: " << argv[0] << " [opciones] <archivo>\n"
                  << " -c <archivo> (comprime)\n"
                  << " -x <archivo> (descomprime)\n"
                  << " -h           (ayuda)\n"
                  << " -v           (version)\n";
        return 1;
    }

    std::string option = argv[1];

    if (option == "-h" || option == "--help") {
        std::cout << "Opciones:\n"
                  << "  -c <archivo> : Comprimir archivo (lo busca en tests/input/), salida en tests/compressed/\n"
                  << "  -x <archivo> : Descomprimir archivo (lo busca en tests/compressed/), salida en tests/output/\n"
                  << "  -v           : Muestra la versión\n"
                  << "  -h           : Ayuda\n";
        return 0;
    }
    else if (option == "-v" || option == "--version") {
        std::cout << "LZW 12-bit Compressor/Decompressor v1.0\n";
        return 0;
    }
    else if (option == "-c") {
        if (argc < 3) {
            std::cerr << "Falta especificar el archivo a comprimir\n";
            return 1;
        }
        std::string inputFile = std::string("tests/input/") + argv[2];

        std::string baseName = fs::path(argv[2]).filename().string();

        std::string outputFile = std::string("tests/compressed/") + baseName + ".lzw";

        compressLZW(inputFile, outputFile);
    }
    else if (option == "-x") {
        if (argc < 3) {
            std::cerr << "Falta especificar el archivo a descomprimir\n";
            return 1;
        }
        std::string inputFile = std::string("tests/compressed/") + argv[2];

        std::string baseName = fs::path(argv[2]).stem().string();

        std::string outputFile = std::string("tests/output/") + baseName;

        decompressLZW(inputFile, outputFile);

        std::string originalFile = "tests/input/" + baseName;
        if (fs::exists(originalFile) && fs::exists(outputFile)) {
            std::cout << "[INFO] Comparando archivos: " << originalFile << " vs " << outputFile << "\n";
            std::string diffCommand = "diff " + originalFile + " " + outputFile;
            int result = std::system(diffCommand.c_str());

            if (result == 0) {
                std::cout << "[SUCCESS] Los archivos son idénticos después de la descompresión.\n";
            } else {
                std::cerr << "[ERROR] Los archivos son diferentes. Puede haber un problema en la compresión o descompresión.\n";
            }
        } else {
            std::cerr << "[WARNING] No se encontró el archivo original para comparar.\n";
        }

    }
    else {
        std::cerr << "Opción no reconocida.\n";
        return 1;
    }

    return 0;
}