#pragma once

#include "vm.h"
#include "object.h"
#include "chunk.h"

bool compile(const char* source, Chunk* chunk);