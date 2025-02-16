#include <iostream>
#include "ManejadorArchivo.h"

int main(int argc, char* argv[]) {
    // Uso: aes_encrypt -e|--encrypt <archivo_entrada> [clave] [archivo_salida]
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " -e|--encrypt <archivo_entrada> [clave] [archivo_salida]" << std::endl;
        return 1;
    }
    
    std::string archivoEntrada = argv[2];
    std::string clave = (argc >= 4) ? argv[3] : "aesEncryptionKey";
    std::string archivoSalida = (argc >= 5) ? argv[4] : "encriptado_hex.txt";
    
    ManejadorArchivo manejador;
    if (!manejador.leerArchivo(archivoEntrada)) {
        std::cerr << "Error al leer el archivo: " << archivoEntrada << std::endl;
        return 1;
    }
    
    // Procesar la encriptación y obtener el resultado en hexadecimal
    std::string cipherHex = manejador.encriptarArchivo(clave);
    
    // Escribir la encriptación en un archivo nuevo
    if (!manejador.escribirEncriptacion(archivoSalida, cipherHex)) {
        std::cerr << "Error al escribir el archivo de encriptación." << std::endl;
        return 1;
    }
    
    return 0;
}
