## AES-128 (ECB)

Este proyecto implementa la encriptación y desencriptación utilizando **AES-128 en modo ECB** con padding **PKCS#7**. La aplicación lee un archivo de entrada, encripta su contenido y muestra el resultado en formato hexadecimal tanto en la consola como en un archivo de salida. Así mismo, recibe un archivo encriptado en hexadecimal y lo desencripta.

> **¿Qué es padding?**  
> Padding es una técnica utilizada en los cifrados por bloques como AES para garantizar que la longitud de los datos de entrada sea un múltiplo del tamaño de bloque requerido. Por ejemplo, AES funciona con bloques de 16 bytes. Si los datos no son un múltiplo exacto de 16 bytes, se añaden bytes adicionales según un esquema definido. En  PKCS#7, por ejemplo, si los datos requieren 6 bytes extra, cada uno de estos bytes tendrá el valor 6 (0x06).

### Estructura del Proyecto

La organización del proyecto es la siguiente:

```
aes_algorithm/
 ├── include
 │     ├── aes_constants.h      # Declaraciones de las constantes del algoritmo AES-128
 │     ├── aes_decryptor.h      # Declaraciones de las funciones inversas de desencriptación
 │     ├── aes_encryptor.h      # Declaraciones de las funciones de encriptación
 │     └── ManejadorArchivo.h   # Declaración de la clase que maneja la lectura de archivos
 ├── src
 │     ├── aes_decryptor.cpp    # Implementación de las funciones de desencriptación AES-128 (modo ECB)
 │     ├── aes_encryptor.cpp    # Implementación de las funciones de encriptación AES-128 (modo ECB)
 │     ├── ManejadorArchivo.cpp # Implementación de la clase para manejo de archivos y procesamiento
 │     └── main.cpp             # Punto de entrada que gestiona los argumentos de línea de comandos
 └── Makefile                   # Script de compilación para generar el ejecutable 'aes'
```

### ¿Por qué AES-128?

Coming soon!

### Procesamiento de archivos

- Explicar si usamos texto o binario y porqué (justificación)
- Cómo almacenamos los datos en memoria
- Cómo recuparemos la información


### Cómo Probar la Implementación

1. **Compilar el proyecto**  
   Desde la raíz del directorio `aes_algorithm/`, ejecuta:
   ```
   make
   ```

2. **Ejecutar la encriptación**  
   La aplicación se invoca desde la línea de comandos con la siguiente sintaxis:
   ```
   ./aes_ [opciones] <archivo_entrada> [archivo_salida] [clave]
   ```
   - `<archivo_entrada>`: Ruta del archivo que deseas encriptar o desencriptar.
   - `[archivo_salida]`: (Opcional) Nombre del archivo donde se almacenará el output respectivo. Por defecto se guarda en `encriptado_hex.txt` para encriptación y en`desencriptado.txt` para desencriptación.
   - `[clave]`: (Opcional) La clave a utilizar para la encriptación, la cual **debe tener longitud de 16 bytes**. Si no se proporciona, se usa la clave por defecto `"aesEncryptionKey"`.

   **Ejemplo:**
   ```
   ./aes -e archivo.txt encriptado_hex.aex aesEncryptionKey
   ./aes -d encriptado_hex.aex desencriptado.txt aesEncryptionKey
   ```

3. **Ver la salida**  
   Creará (o sobrescribirá) el archivo de salida con la cadena resultante.

### Configurar como un programa del sistema

Si deseas ejecutar el programa desde cualquier ubicación en tu sistema, puedes instalarlo en un directorio del PATH. Para ello, sigue estos pasos:

1. **Compilar el proyecto**  
   Desde la raíz del directorio `aes_algorithm/`, ejecuta:
   ```
   make
   ```

2. **Instalar el programa**
    ```
    sudo make install
    ```
    Esto copiará el ejecutable `aes` al directorio `/usr/local/bin/`. De pronto es necesario reiniciar la terminal para que los cambios surtan efecto.

3. **Ejecutar el programa**  
   Ahora puedes invocar el programa desde cualquier ubicación en tu sistema:
   ```
   aes [opciones] <archivo_entrada> [archivo_salida] [clave]
   ```

   **Ejemplo:**
   ```
   aes -e archivo.txt encriptado_hex.aex aesEncryptionKey
   aes -d encriptado_hex.aex desencriptado.txt aesEncryptionKey
   ```

### Cómo puedo comprobar si funciona correctamente

1. Accede a [AES encryption / decryption tool](https://the-x.cn/en-us/cryptography/Aes.aspx), selecciona el algoritmo AES.

2. En la configuraciones, selecciona el modo **ECB**, el padding **PKCS7** y la longitud de la clave **128 bits**.

3. Introduce la clave `"aesEncryptionKey"` y el contenido de que desees encriptar o desencriptar.

4. Haz clic en **Encrypt/Dencrypt** y deberías obtener el mismo output que tuviste en nuestro programa.
