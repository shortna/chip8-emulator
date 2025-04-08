#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
typedef uint16_t instruction_t;
#define INSTRUCTION_SIZE sizeof(instruction_t)
extern int execute(instruction_t i);

#endif
