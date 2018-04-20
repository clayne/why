#ifndef __WVM_H__
#define __WVM_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef int64_t word;
typedef uint64_t uword;
typedef uint8_t byte;

bool wvm_init(word length);
void wvm_free();
int wvm_load(char *filename);
void wvm_init_vm();
word wvm_get_word(word addr);
void wvm_set_word(word addr, word value);
byte wvm_get_byte(word addr);
void wvm_set_byte(word addr, byte value);
void wvm_jump(word addr);
void wvm_link();
void wvm_increment_pc();
bool wvm_tick();
void wvm_print_memory();

byte *memory;
size_t memsize;
word pc; // in bytes, not words.
word offset_handlers;
word offset_data;
word offset_code;
word offset_end;
word registers[128];
word membytes;
int cycles;
bool alive;

#endif
