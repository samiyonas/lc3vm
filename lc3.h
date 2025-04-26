#ifndef LC3_H
#define LC3_H
#define MEMORY_MAX (1 << 16)

/* Includes */
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
/* unix only */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

/* function prototypes */
uint16_t sign_extend(uint16_t x, int bit_count);
void update_flag(uint16_t r);
void add(uint16_t instr);
void ldi(uint16_t instr);
void And(uint16_t instr);
void Not(uint16_t instr);
void branch(uint16_t instr);
void jump(uint16_t instr);
void jump_register(uint16_t instr);
void load(uint16_t instr);
void load_register(uint16_t instr);
void load_effective_address(uint16_t instr);
void store(uint16_t instr);
void store_indirect(uint16_t instr);
void store_register(uint16_t instr);
void read_image_file(FILE* file);
uint16_t swap16(uint16_t x);
int read_image(const char* image_path);
void mem_write(uint16_t address, uint16_t val);
uint16_t mem_read(uint16_t address);
void disable_input_buffering();
void restore_input_buffering();
uint16_t check_key();

extern uint16_t memory[MEMORY_MAX]; /* 65,536 locations */

enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter register */
    R_COND, /* condition flag register */
    R_COUNT
};

extern uint16_t reg[R_COUNT]; /* 16 bits long registers */

enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

enum
{
    TRAP_GETC = 0x20, /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21, /* output a character */
    TRAP_PUTS = 0x22, /* output a word string */
    TRAP_IN = 0x23, /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25, /* halt the program */
};

enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02, /* keyboard data */
};

extern struct termios original_tio;

#endif
