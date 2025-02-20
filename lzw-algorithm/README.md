# LZW 12-bit Compressor/Decompressor

## Descripción del Proyecto

Este proyecto implementa un compresor y descompresor de archivos utilizando el algoritmo **Lempel-Ziv-Welch (LZW)** en C++. El algoritmo usa una codificación de **12 bits** para representar secuencias de bytes y permite hasta **4096 entradas** en el diccionario.

- Se emplea un mecanismo de **reinicio del diccionario** (CLEAR\_CODE = 256) cuando se llena.
- Todas las operaciones de **lectura y escritura** se realizan mediante llamadas al sistema como `open`, `read`, `write`, y `close`.
- No se utilizan librerías externas de compresión o encriptación.

## Características Principales

- **Compresión y descompresión sin pérdida (lossless).**
- **Uso de 12 bits internamente**, pero la entrada y salida final se manejan en **bloques de 8 bits**.
- **Reinicio automático** del diccionario cuando alcanza **4096 entradas**.
- **Soporte para archivos de 8 bits** (textos ASCII, imágenes BMP de 8 bits, archivos WAV sin comprimir, entre otros archivos binarios de 8 bits).
- **No compatible con archivos de codificación multibyte** (UTF-8 con caracteres especiales, UTF-16, etc.).

## Uso del Programa

El programa se comporta como una función del sistema operativo y acepta argumentos de línea de comandos:

```bash
./lzw_program [opción] <archivo>
```

### Opciones Disponibles

| Opción                                   | Descripción                                                                                                                                                                                                                   |
| ---------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-h`, `--help`                           | Muestra el mensaje de ayuda con las opciones de uso.                                                                                                                                                                          |
| `-v`, `--version`                        | Muestra la versión del programa.                                                                                                                                                                                              |
| `-c <archivo>`, `--compress <archivo>`   | Comprime el archivo indicado. Lo busca en `tests/input/` y guarda la salida en `tests/compressed/` con la extensión `.lzw`.                                                                                                   |
| `-x <archivo>`, `--decompress <archivo>` | Descomprime el archivo indicado, lo busca en `tests/compressed/` y lo guarda en `tests/output/`, recuperando el nombre original. También ejecuta `diff` automáticamente para comparar el archivo original y el descomprimido. |

### Ejemplos de Uso

#### Comprimir un archivo de texto:

```bash
./lzw_program -c test.txt
```

Salida: `tests/compressed/test.txt.lzw`

#### Descomprimir el archivo:

```bash
./lzw_program -x test.txt.lzw
```

Salida: `tests/output/test.txt` y comparación automática con `tests/input/test.txt`

#### Ver la ayuda:

```bash
./lzw_program -h
```

#### Ver la versión:

```bash
./lzw_program -v
```

## Estructura de Archivos y Carpetas

```
ProyectoLZW/
├── include/          # Archivos de cabecera (.h)
├── src/              # Código fuente (.cpp)
├── tests/
│   ├── input/        # Archivos originales a comprimir
│   ├── compressed/   # Archivos comprimidos (.lzw)
│   ├── output/       # Archivos descomprimidos
├── bin/              # Binarios compilados (opcional)
├── Makefile          # Archivo para compilación
├── README.md         # Documentación
```

## Justificación Técnica

### Funcionamiento del Algoritmo LZW

El algoritmo LZW funciona construyendo un **diccionario dinámico** de secuencias repetitivas en los datos:

1. **Inicialización:** Se llena el diccionario con los **256 caracteres ASCII**.
2. **Compresión:** Se van agregando nuevas secuencias encontradas en los datos de entrada.
3. **Cuando el diccionario alcanza su límite (4096 entradas):** Se emite un **CLEAR\_CODE (256)** y se reinicia el diccionario.
4. **Descompresión:** Se reconstruye el diccionario a medida que se leen los códigos comprimidos.

### Uso de 8 y 12 bits

- Los archivos de entrada son procesados en bloques de **8 bits**.
- Internamente, el algoritmo usa **12 bits** para representar las secuencias comprimidas.
- La salida final es escrita en bloques de **8 bits**, lo que garantiza compatibilidad con archivos binarios.


## Notas Adicionales

- Se recomienda probar con archivos de texto (`.txt`), imágenes BMP de 8 bits (`.bmp`), y audio sin comprimir (`.wav`).
- No es compatible con archivos UTF-8 que contengan caracteres especiales multibyte.
- Se puede mejorar agregando soporte para más formatos y optimizaciones en la memoria.

---

**Autor:** Proyecto desarrollado como parte del curso de **Sistemas Operativos**. Última actualización: 2025.