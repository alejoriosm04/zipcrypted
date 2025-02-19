#ifndef MANEJADOR_ARCHIVO_H
#define MANEJADOR_ARCHIVO_H

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include "aes_constants.h" 

class ManejadorArchivo {
private:
    std::vector<uint8_t> datos;  

public:
    // Lee un archivo (modo binario) y almacena su contenido en 'datos'
    bool leerArchivo(const std::string &nombreArchivo);

    // Retorna los datos leídos para procesamiento
    const std::vector<uint8_t>& obtenerDatos() const { return datos; }

    // Procesa la encriptación del contenido leído utilizando la clave proporcionada y retorna el resultado en hexadecimal.
    std::string encriptarArchivo(const std::string &clave);

    std::string desencriptarArchivo(const std::string &clave);  
    bool escribirDesencriptacion(const std::string &nombreArchivo, const std::string &contenido);

    // Escribe la cadena de encriptación (en hexadecimal) en el archivo indicado.
    bool escribirEncriptacion(const std::string &nombreArchivo, const std::string &contenidoHex);
};

#endif 
