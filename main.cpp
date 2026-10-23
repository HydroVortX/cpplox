#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "include/common.h"
#include "include/chunk.h"
#include "include/debug.h"
#include "include/vm.h"

static void repl(){
    char line[1024];
    for(;;){
        std::cout << "> ";
        if(!std::fgets(line, sizeof(line), stdin)){
            std::cout << "\n";
            break;
        }
        interpret(line);
    }
}

static char* readFile(const char* path){
    std::ifstream file(path, std::ios::binary);
    if(!file){
        std::cerr << "Could not open file \"" << path << "\".\n";
        std::exit(74);
    }

    file.seekg(0, file.end);
    std::size_t fileSize = file.tellg();
    file.seekg(0, file.beg);

    char* buffer = new char[fileSize + 1];

    file.read(buffer, fileSize);
    if(static_cast<std::size_t>(file.gcount()) < fileSize){
        std::cerr << "Could not read file \"" << path << "\".\n";
        delete[] buffer;
        std::exit(74);
    }
    buffer[fileSize] = '\0';

    return buffer;
}

static void runFile(const char* path){
    char* source = readFile(path);
    interpretResult result = interpret(source);
    delete[] source;

    if(result == INTERPRET_COMPILE_ERROR) std::exit(65);
    if(result == INTERPRET_RUNTIME_ERROR) std::exit(70);
}

int main(int argc, const char* argv[]) {
    initVM();

    if(argc == 1){
        repl();
    } else if(argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Usage: cpplox [path]" << std::endl;
        std::exit(64);
    }

    freeVM();
    return 0;
}
