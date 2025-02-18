#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "ManejadorArchivo.h"

bool g_enableLogging = false;

void printHelp(const std::string &progName) {
    std::cout << "Uso: " << progName << " [opciones] <archivo_entrada> [archivo_salida] [clave]\n";
    std::cout << "Opciones:\n";
    std::cout << "  -h, --help             Muestra este mensaje de ayuda.\n";
    std::cout << "  -v, --version          Muestra la versión del programa.\n";
    std::cout << "  -e, --encrypt          Encripta el archivo indicado.\n";
    std::cout << "  -d, --decrypt          Desencripta el archivo indicado.\n";
    std::cout << "  -l, --log              Activa el registro del proceso interno (logging).\n";
    std::cout << "  -t, --show-type        Muestra el tipo de archivo (usa el comando \"file\").\n";
    std::cout << "\nNota: La clave debe tener exactamente 16 caracteres de longitud.\n";
}

void printVersion() {
    std::cout << "AES Encryptor/Decryptor Version 1.0\n";
}

bool validarArgumentos(const std::vector<std::string> &args, std::string &option, 
                       std::string &archivoEntrada, std::string &archivoSalida, std::string &clave) {
    if (args.size() < 2) {
        std::cerr << "Error: Se requiere al menos la opción y el archivo de entrada.\n";
        return false;
    }
    
    option = args[0];
    if (option != "-e" && option != "--encrypt" &&
        option != "-d" && option != "--decrypt") {
        std::cerr << "Error: Opción '" << option << "' no reconocida.\n";
        return false;
    }
    
    archivoEntrada = args[1];
    if (archivoEntrada.empty()) {
        std::cerr << "Error: El nombre del archivo de entrada no puede estar vacío.\n";
        return false;
    }
    
    if (args.size() >= 3) {
        archivoSalida = args[2];
    } else {
        archivoSalida = (option == "-e" || option == "--encrypt") ? "encriptado_hex.aex" : "desencriptado.txt";
    }
    
    if (args.size() >= 4) {
        clave = args[3];
    } else {
        clave = "aesEncryptionKey";
    }
    
    if (clave.size() != 16) {
        std::cerr << "Error: La clave debe tener exactamente 16 caracteres, pero se proporcionó una de " 
                  << clave.size() << " caracteres.\n";
        return false;
    }
    
    if (args.size() > 4) {
        std::cerr << "Advertencia: Se han proporcionado argumentos adicionales que serán ignorados.\n";
    }
    
    return true;
}
inline std::string getFileType(const std::string &filename) {
    std::string command = "file " + filename;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "Error al ejecutar comando";
    }
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    bool showFileType = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "-l" || arg == "--log") {
            g_enableLogging = true;
        } else if (arg == "-t" || arg == "--show-type") {
            showFileType = true;
        } else if (arg != "-h" && arg != "--help" && arg != "-v" && arg != "--version") {
            args.push_back(arg);
        }
    }
    
    if (argc == 2) {
        std::string arg(argv[1]);
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        }
        if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        }
    }
    
    std::string option, archivoEntrada, archivoSalida, clave;
    if (!validarArgumentos(args, option, archivoEntrada, archivoSalida, clave)) {
        printHelp(argv[0]);
        return 1;
    }
    
    ManejadorArchivo manejador;
    if (!manejador.leerArchivo(archivoEntrada)) {
        std::cerr << "Error al leer el archivo: " << archivoEntrada << std::endl;
        return 1;
    }
    
    if (showFileType) {
        std::string fileInfo = getFileType(archivoEntrada);
        std::cout << "Tipo de archivo: " << fileInfo << std::endl;
    }
    
    if (option == "-e" || option == "--encrypt") {
        std::string cipherHex = manejador.encriptarArchivo(clave);
        if (!manejador.escribirEncriptacion(archivoSalida, cipherHex)) {
            std::cerr << "Error al escribir el archivo encriptado." << std::endl;
            return 1;
        }
        std::cout << "Archivo encriptado correctamente en: " << archivoSalida << std::endl;
    } else if (option == "-d" || option == "--decrypt") {
        std::string plainText = manejador.desencriptarArchivo(clave);
        if (!manejador.escribirDesencriptacion(archivoSalida, plainText)) {
            std::cerr << "Error al escribir el archivo desencriptado." << std::endl;
            return 1;
        }
        std::cout << "Archivo desencriptado correctamente en: " << archivoSalida << std::endl;
    } else {
        std::cerr << "Error: Opción no reconocida. Usa -h para ayuda.\n";
        return 1;
    }
    
    return 0;
}
