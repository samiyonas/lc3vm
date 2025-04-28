#include "lc3.h"

uint16_t memory[MEMORY_MAX];
uint16_t reg[R_COUNT];
struct termios original_tio;

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }


    for (int j = 1; j < argc; j++)
    {
        if (!read_image(argv[j]))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
    /* since exactly one condition flag should be set at any given time, set the Z flag */
    reg[R_COND] = FL_ZRO;
    /* set the PC to starting position */
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 1;
    while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
            case OP_ADD:
                add(instr);
            case OP_AND:
                And(instr);
            case OP_NOT:
                Not(instr);
            case OP_BR:
                branch(instr);
            case OP_JMP:
                jump(instr);
            case OP_JSR:
                jump_register(instr);
            case OP_LD:
                load(instr);
            case OP_LDI:
                ldi(instr);
            case OP_LDR:
                load_register(instr);
            case OP_LEA:
                load_effective_address(instr);
            case OP_ST:
                store(instr);
            case OP_STI:
                store_indirect(instr);
            case OP_STR:
                store_register(instr);
            case OP_TRAP:
                /* we would have more cases here */
                reg[R_R7] = reg[R_PC];
                switch (instr & 0xFF)
                {
                    case TRAP_GETC:
                        reg[R_R0] = (uint16_t)(getchar());
                        update_flag(R_R0);
                        break;
                    case TRAP_OUT:
                        putc((char)(reg[R_R0]), stdout);
                        fflush(stdout);
                        break;
                    case TRAP_PUTS:
                        uint16_t *c = memory + reg[R_R0];
                        while (*c != '\0')
                        {
                            putc((char)(*c), stdout);
                            c++;
                        }
                        fflush(stdout);
                        break;
                    case TRAP_IN:
                        printf("Enter a character: ");
                        char l;
                        l = getchar();
                        putc(*c, stdout);
                        reg[R_R0] = (uint16_t)(*c);
                        update_flag(R_R0);
                        break;
                    case TRAP_PUTSP:
                        uint16_t *k = memory + reg[R_R0];
                        while (*k != '\0')
                        {
                            putc((char)((*k) & 0xFF), stdout);
                            char ch = (*k) >> 8;
                            if (ch)
                            {
                                putc((char)((*k) >> 8), stdout);
                            }
                            else
                            {
                                break;
                            }
                            k++;
                        }
                        fflush(stdout);
                        break;
                    case TRAP_HALT:
                        puts("HALT");
                        fflush(stdout);
                        running = 0;
                        break;
                }
                break;
            default:
                abort();
                break;
        }
    }
    restore_input_buffering();
}
