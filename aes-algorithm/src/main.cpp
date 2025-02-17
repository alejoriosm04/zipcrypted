#include <iostream>
#include <string>
#include "ManejadorArchivo.h"

void printHelp(const std::string &progName) {
    std::cout << "Uso: " << progName << " [opciones] <archivo_entrada> [clave] [archivo_salida]\n";
    std::cout << "Opciones:\n";
    std::cout << "  -h, --help             Muestra este mensaje de ayuda.\n";
    std::cout << "  -v, --version          Muestra la versión del programa.\n";
    std::cout << "  -e, --encrypt          Encripta el archivo indicado.\n";
    std::cout << "  -d, --decrypt          Desencripta el archivo indicado.\n";
}

void printVersion() {
    std::cout << "AES Encryptor/Decryptor Version 1.0\n";
}

int main(int argc, char* argv[]) {
    // Si no se pasan argumentos, se muestra la ayuda.
    if (argc < 2) {
        printHelp(argv[0]);
        return 1;
    }
    
    std::string option = argv[1];
    
    // Verifica si se pide ayuda
    if (option == "-h" || option == "--help") {
        printHelp(argv[0]);
        return 0;
    }
    
    // Verifica si se pide la versión
    if (option == "-v" || option == "--version") {
        printVersion();
        return 0;
    }
    
    // Para encriptación o desencriptación se requiere al menos 3 argumentos
    if (argc < 3) {
        std::cerr << "Error: Falta el nombre del archivo de entrada.\n";
        printHelp(argv[0]);
        return 1;
    }
    
    std::string archivoEntrada = argv[2];
    // Clave por defecto es "aesEncryptionKey"
    std::string clave = (argc >= 4) ? argv[3] : "aesEncryptionKey";
    // Archivo de salida por defecto:
    std::string archivoSalida = (argc >= 5) ? argv[4] : ((option == "-e" || option == "--encrypt") ? "encriptado_hex.txt" : "desencriptado.txt");
    
    ManejadorArchivo manejador;
    if (!manejador.leerArchivo(archivoEntrada)) {
        std::cerr << "Error al leer el archivo: " << archivoEntrada << std::endl;
        return 1;
    }
    
    if (option == "-e" || option == "--encrypt") {
        std::string cipherHex = manejador.encriptarArchivo(clave);
        if (!manejador.escribirEncriptacion(archivoSalida, cipherHex)) {
            std::cerr << "Error al escribir el archivo encriptado." << std::endl;
            return 1;
        }
        std::cout << "Archivo encriptado correctamente en: " << archivoSalida << std::endl;
    }
    else if (option == "-d" || option == "--decrypt") {
        std::string plainText = manejador.desencriptarArchivo(clave);
        if (!manejador.escribirDesencriptacion(archivoSalida, plainText)) {
            std::cerr << "Error al escribir el archivo desencriptado." << std::endl;
            return 1;
        }
        std::cout << "Archivo desencriptado correctamente en: " << archivoSalida << std::endl;
    }
    else {
        std::cerr << "Opción no reconocida. Usa -h para ayuda." << std::endl;
        return 1;
    }
    
    return 0;
}
