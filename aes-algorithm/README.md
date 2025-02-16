## AES (ECB) Encrypt

Este proyecto implementa la encriptación utilizando **AES-128 en modo ECB** con padding **PKCS#7**. La aplicación lee un archivo de entrada, encripta su contenido y muestra el resultado en formato hexadecimal tanto en la consola como en un archivo de salida.

> **What is padding?: Padding**  
> Padding is a technique used in block ciphers like AES to ensure that the input data's length is a multiple of the required block size. For instance, AES works with 16-byte blocks. If the data isn't an exact multiple of 16 bytes, extra bytes are added according to a defined scheme.  
> In PKCS#7 padding, for example, if the data requires 6 extra bytes, each of these bytes will have the value 6 (0x06). This allows the decryption process to identify and remove the padding correctly, restoring the original data length.

### Estructura del Proyecto

La organización del proyecto es la siguiente:

```
aes_algorithm/
 ├── include
 │     ├── AES.h                # Declaraciones de las funciones de encriptación (KeyExpansion, AES_Encrypt, conversiones y padding PKCS#7)
 │     └── ManejadorArchivo.h   # Declaración de la clase que maneja la lectura de archivos y el proceso de encriptación, 
 │                                # incluyendo la escritura del resultado en un archivo en hexadecimal.
 ├── src
 │     ├── AES.cpp              # Implementación de las funciones de encriptación AES-128 (modo ECB)
 │     ├── ManejadorArchivo.cpp # Implementación de la clase para manejo de archivos y procesamiento de encriptación
 │     └── main.cpp             # Punto de entrada que gestiona los argumentos de línea de comandos y llama a la función de encriptación
 └── Makefile                   # Script de compilación para generar el ejecutable 'aes_encrypt'
```

### Descripción de Archivos

- **include/aes_encryptor.h**  
  Define el tipo de dato `aes_byte` y las constantes necesarias para AES-128, así como la interfaz de las funciones:
  - `KeyExpansion`: Expande la clave de 16 bytes a 176 bytes.
  - `AES_Encrypt`: Cifra un bloque de 16 bytes (representado como una matriz 4x4).
  - `arrayToState` y `stateToArray`: Convierten entre un arreglo lineal de 16 bytes y la representación en matriz 4x4.
  - `applyPKCS7Padding`: Aplica el padding PKCS#7 a un string para que su longitud sea múltiplo de 16.

- **src/aes_encryptor.cpp**  
  Contiene la implementación de las funciones de encriptación. Se utiliza la S-Box y Rcon para la expansión de la clave y la encriptación de cada bloque.

- **include/ManejadorArchivo.h**  
  Declara la clase `ManejadorArchivo`, la cual se encarga de:
  - Leer el archivo de entrada (en modo binario) y almacenar sus datos.
  - Procesar la encriptación del contenido leído utilizando una clave.
  - Convertir el resultado de la encriptación a una cadena hexadecimal.
  - Escribir el resultado en un nuevo archivo.

- **src/ManejadorArchivo.cpp**  
  Implementa los métodos de `ManejadorArchivo`, en particular:
  - `leerArchivo`: Lee el archivo y almacena su contenido en un vector.
  - `encriptarArchivo`: Aplica padding, cifra cada bloque utilizando las funciones de AES y retorna el ciphertext en formato hexadecimal.
  - `escribirEncriptacion`: Escribe la cadena hexadecimal resultante en un archivo de salida.

- **src/main.cpp**  
  Es el entry point de la aplicación. Se encarga de:
  - Leer los argumentos de la línea de comandos.
  - Validar el uso correcto del programa (por ejemplo, la opción `-e` o `--encrypt`).
  - Invocar los métodos de `ManejadorArchivo` para leer el archivo de entrada, encriptarlo y guardar el resultado en un archivo nuevo.

- **Makefile**  
  Permite compilar todo el proyecto y generar el ejecutable llamado `aes`.  
  - Para compilar: `make`
  - Para limpiar la compilación: `make clean`

### Cómo Probar la Implementación

1. **Compilar el proyecto**  
   Desde la raíz del directorio `aes_algorithm/`, ejecuta:
   ```
   make
   ```

2. **Ejecutar la encriptación**  
   La aplicación se invoca desde la línea de comandos con la siguiente sintaxis:
   ```
   ./aes_ -e <archivo_entrada> [clave] [archivo_salida]
   ```
   - `<archivo_entrada>`: Ruta del archivo que deseas encriptar.
   - `[clave]`: (Opcional) La clave a utilizar para la encriptación. Si no se proporciona, se usa la clave por defecto `"aesEncryptionKey"`.
   - `[archivo_salida]`: (Opcional) Nombre del archivo donde se almacenará el ciphertext en formato hexadecimal. Por defecto se guarda en `encriptado_hex.txt`.

   **Ejemplo:**
   ```
   ./aes -e archivo.txt aesEncryptionKey encriptado_hex.txt
   ```

3. **Ver la salida**  
   Creará (o sobrescribirá) el archivo de salida con la cadena hexadecimal resultante.

### Cómo puedo comprobar si funciona correctamente

1. Accede a [AES encryption / decryption tool](https://the-x.cn/en-us/cryptography/Aes.aspx), selecciona el algoritmo AES.

2. En la configuraciones, selecciona el modo **ECB**, el padding **PKCS7** y la longitud de la clave **128 bits**.

3. Introduce la clave `"aesEncryptionKey"` y el contenido del archivo de encriptación en `encriptado_hex.txt`.

4. Haz clic en **Dencrypt** y deberías obtener el mismo mensaje que encriptaste.
