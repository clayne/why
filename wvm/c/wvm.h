#ifndef __WVM_H__
#define __WVM_H__

#include <stdint.h>
typedef long long int lomg;

int wvm_init(lomg length);
void wvm_free();
int wvm_load(char *filename);
void wvm_init_pc();
lomg wvm_get_word(lomg n);
char wvm_get_byte(lomg byte_offset);
void wvm_print_memory();
uint16_t wvm_get_opcode(lomg instruction);

lomg *memory;
size_t memsize;
lomg pc; // in words, not bytes.
lomg offset_handlers;
lomg offset_data;
lomg offset_code;
lomg offset_end;

enum ins_type {J, I, R};

#endif