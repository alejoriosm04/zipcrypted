# Makefile sencillo para compilar LZW (12 bits con reset)

CXX      = g++
CXXFLAGS = -Wall -O2 -std=c++17
INCLUDES = -I include

# Archivos fuente
SRC_COMPRESS    = src/lzw_compressor.cpp
SRC_DECOMPRESS  = src/lzw_decompressor.cpp  # <--- Nuevo para descompresión
SRC_MAIN        = src/main.cpp

# Generamos objetos a partir de .cpp
OBJ_COMPRESS    = $(SRC_COMPRESS:.cpp=.o)
OBJ_DECOMPRESS  = $(SRC_DECOMPRESS:.cpp=.o)  # <--- Objeto del descompresor
OBJ_MAIN        = $(SRC_MAIN:.cpp=.o)

# Regla principal: "all" para compilar y obtener ejecutable
all: lzw_compresor

# Ejecutable: incluye los objetos del compresor, descompresor y main
lzw_compresor: $(OBJ_COMPRESS) $(OBJ_DECOMPRESS) $(OBJ_MAIN)
	$(CXX) $(CXXFLAGS) -o lzw_compresor $(OBJ_COMPRESS) $(OBJ_DECOMPRESS) $(OBJ_MAIN)

# Regla genérica para compilar .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Comando de limpieza
clean:
	rm -f src/*.o lzw_compresor