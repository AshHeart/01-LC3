#include <cstdint>
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

#include "AS/BitUtils.h"

#define MEMORY_MAX (1 << 16) // 65,536

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

uint16_t memory[MEMORY_MAX];

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
    R_PC,
    R_COND,
    R_COUNT
};

uint16_t reg[R_COUNT];

enum
{
    MR_KBSR = 0xFE00,
    MR_KBDR = 0xFE02
};

enum
{
    FL_POS = 1 << 0,
    FL_ZRO = 1 << 1,
    FL_NEG = 1 << 2
};

enum
{
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_RES,
    OP_LEA,
    OP_TRAP
};

struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

void execute_add(uint16_t);
void execute_loadi(uint16_t);
void update_flags(uint16_t);
void __print_decimal(const char *);
void __print_16bits(uint16_t);

uint16_t mem_read(uint16_t addr);

int main(int argc, char **argv) {

    // if (argc < 2) {

    //     printf("lc3 [image-file1] ... \n");
    //     exit(2);
    // }

    // for (int j = 1; j < argc; ++j) {
    //     if (!read_image(argv[j])) {

    //         printf("failed to load image: %s\n", argv[j]);
    //         exit(1);
    //     }
    // }

       // A workaround to the cannot convert string literal to char* warning by clang
      // FIXME: There should be a better approach, see why std::string is not found
     //          void foo (std::string theParam) { std::cout << theParam; }
    //           foo("bar");
    const char* param = "0x1f";
    __print_decimal(param);

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    reg[R_COND] = FL_ZRO;

    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 0;
    while (running) {
        // uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t instr = 4162; // => 0b 00010000 01000010 => ADD R0 R1 000 R2
        uint16_t op = instr >> 12;

        __print_16bits(instr);

        switch (op)
        {
            case OP_ADD:
                execute_add(instr);
                break;
            case OP_LD:
                execute_loadi(instr);
                break;
        }
    }

    restore_input_buffering();
}

void execute_add(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;       // & 0x7 ensures we read the value of last 3 bits
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1; // & 0x1 ensured we read a single bit at the end

    if (imm_flag)
    {
        uint16_t imm5 = AS::sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] + imm5;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }

    update_flags(r0);
}

void execute_loadi(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;

    uint16_t pc_offset = AS::sign_extend(instr & 0x1FF, 9);

    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));

    update_flags(r0);
}

void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key()) {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }

    return memory[address];
}

void __print_decimal(const char *hexstr)
{
    printf("%d\n", (int)strtol(hexstr, NULL, 0));
}

void __print_16bits(uint16_t byte)
{
    printf("byte: " BYTE_TO_BINARY_PATTERN" " BYTE_TO_BINARY_PATTERN"\n",
    BYTE_TO_BINARY(byte >> 8), BYTE_TO_BINARY(byte));
}
