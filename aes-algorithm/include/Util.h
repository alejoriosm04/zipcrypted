#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdio>    // popen, pclose, fgets
#include "aes_constants.h"  

extern bool g_enableLogging;

inline void printMatrix(const std::string &label, aes_byte state[4][4], const std::string &roundInfo) {
    if (!g_enableLogging)
        return;
    std::cout << "=== " << label << " - " << roundInfo << " ===\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(state[i][j]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "----------------------\n";
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

#endif 
