#include <iostream>
#include <string>
#include "ManejadorArchivo.h"

int main(int argc, char* argv[]) {
    // Validar argumentos
    if (argc < 3) {
        std::cerr << "Uso:\n"
                  << "  " << argv[0] << " -e|--encrypt <archivo_entrada> [clave] [archivo_salida]\n"
                  << "  " << argv[0] << " -d|--decrypt <archivo_entrada> [clave] [archivo_salida]\n";
        return 1;
    }

    // Opción (-e o -d)
    std::string option = argv[1];
    std::string archivoEntrada = argv[2];
    // Clave (por defecto "aesEncryptionKey")
    std::string clave = (argc >= 4) ? argv[3] : "aesEncryptionKey";
    // Archivo de salida
    // Por defecto, "encriptado_hex.txt" si encriptas, o "desencriptado.txt" si desencriptas
    std::string archivoSalida;
    if (argc >= 5) {
        archivoSalida = argv[4];
    } else {
        archivoSalida = (option == "-e" || option == "--encrypt") 
                        ? "encriptado_hex.txt" 
                        : "desencriptado.txt";
    }

    // Crear objeto para manejar archivo
    ManejadorArchivo manejador;
    // Leer archivo de entrada
    if (!manejador.leerArchivo(archivoEntrada)) {
        std::cerr << "Error al leer el archivo: " << archivoEntrada << std::endl;
        return 1;
    }

    // Lógica de encriptación o desencriptación
    if (option == "-e" || option == "--encrypt") {
        // Encriptar
        std::string cipherHex = manejador.encriptarArchivo(clave);
        if (!manejador.escribirEncriptacion(archivoSalida, cipherHex)) {
            std::cerr << "Error al escribir el archivo encriptado." << std::endl;
            return 1;
        }
        std::cout << "Archivo encriptado correctamente en: " << archivoSalida << std::endl;
    } 
    else if (option == "-d" || option == "--decrypt") {
        // Desencriptar
        std::string plainText = manejador.desencriptarArchivo(clave);
        if (!manejador.escribirDesencriptacion(archivoSalida, plainText)) {
            std::cerr << "Error al escribir el archivo desencriptado." << std::endl;
            return 1;
        }
        std::cout << "Archivo desencriptado correctamente en: " << archivoSalida << std::endl;
    } 
    else {
        std::cerr << "Opción no reconocida. Usa -e para encriptar o -d para desencriptar." << std::endl;
        return 1;
    }

    return 0;
}
