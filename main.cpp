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

    char* buffer = new char[ fileSize + 1];
    if(buffer == nullptr){
        std::cerr << "Not enough memory to read \"" << path << "\".\n";
        delete[] buffer;
        std::exit(74);
    }

    file.read(buffer, fileSize);
    if(static_cast<std::size_t>(file.gcount()) < fileSize){
        std::cerr << "Could not read file \"" << path << "\".\n";
        delete[] buffer;
        std::exit(74);
    }
    buffer[fileSize] = '\0';

    return buffer;


    // std::FILE* file = std::fopen(path, "rb");
    // std::fseek(file, 0L, SEEK_END);
    // std::size_t filesize = std::ftell(file);
    // rewind(file);

    // char* buffer = (char*)std::malloc(filesize + 1);
    // std::size_t bytesRead = std::fread(buffer, sizeof(char), filesize, file);
    // buffer[bytesRead] = '\0';

    // std::fclose(file);
    // return buffer;   
}

static void runFile(const char* path){
    char* source = readFile(path);
    interpretResult result = interpret(source);
    delete[] source;

    if(result == INTERPRET_COMPILE_ERROR) exit(65);
    if(result == INTERPRET_RUNTIME_ERROR) exit(70);
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

    // Chunk chunk;
    // initChunk(&chunk);

    // writeConstant(&chunk, 1.5, 122);
    // writeConstant(&chunk, 2.5, 122);
    // writeChunk(&chunk, OP_NEGATE, 122);
    // writeChunk(&chunk, OP_ADD, 122);
    // writeConstant(&chunk, 3.0, 122);
    // writeChunk(&chunk, OP_MULTIPLY, 122);

    // for(int i = 0; i < 300; i++){
    //     if(i < 255){
    //         writeConstant(&chunk, (double)i, 127);
    //     } else {
    //         writeConstant(&chunk, (double)i, 126);
    //     }
    // }

    // writeConstant(&chunk, 1.2, 128);
    // writeChunk(&chunk, OP_ADD, 129);

    // writeChunk(&chunk, OP_RETURN, 128);

    // disassembleChunk(&chunk, "test chunk");
    // interpret(&chunk);
}
