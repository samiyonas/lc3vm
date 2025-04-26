#include "lc3.h"

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x = x | (0xFFFF << bit_count);
    }

    return x;
}

void update_flag(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* 1(true) if the number in the regsiter is negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

/* add two numbers */
void add(uint16_t instr)
{
    /* the destination regsiter */
    uint16_t r0 = (instr >> 9) & 0x7;

    /* the first operand register */
    uint16_t r1 = (instr >> 6) & 0x7;

    /* the mode bit
     * it will help us find out if the second operand is immediate or from a register
     */
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        reg[r0] = reg[r1] + sign_extend(instr & 0x1F, 5);
    }
    else
    {
        /* the second operand */
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }

    update_flag(r0);
}

/* load indirectly from memory to register */
void ldi(uint16_t instr)
{
    /* load it on this register */
    uint16_t r0 = (instr >> 9) & 7;

    uint16_t pc_offset = reg[R_PC] + sign_extend(instr & 0x1FF, 9);
    reg[r0] = mem_read(mem_read(pc_offset));

    update_flag(r0);
}

void And(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;

    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        reg[r0] = reg[r1] & sign_extend(instr & 0x1F, 5);
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] & reg[r2];
    }

    update_flag(r0);
}

void Not(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;

    reg[r0] = ~reg[r1];

    update_flag(r0);
}

/* branch to another address if the condition is met */
void branch(uint16_t instr)
{
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;

    if (reg[R_COND] & cond_flag)
    {
        reg[R_PC] = reg[R_PC] + pc_offset;
    }
}

void jump(uint16_t instr)
{
    uint16_t r0 = (instr >> 6) & 0x7;
    reg[R_PC] = reg[r0];
}

void jump_register(uint16_t instr)
{
    reg[R_R7] = reg[R_PC];

    uint16_t mode = (instr >> 11) & 0x1;
    if (mode)
    {
        reg[R_PC] += sign_extend(instr & 0x7FF, 11);
    }
    else
    {
        uint16_t r0 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r0];
    }
}

void load(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    reg[r0] = mem_read(reg[R_PC] + sign_extend(instr & 0x1FF, 9));

    update_flag(r0);
}

void load_register(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;

    reg[r0] = mem_read(reg[r1] + sign_extend(instr & 0x3F, 6));
    
    update_flag(r0);
}

void load_effective_address(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    reg[r0] = reg[R_PC] + sign_extend(instr & 0x1FF, 9);

    update_flag(r0);
}

void store(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

    mem_write(reg[R_PC] + pc_offset, reg[r0]);
}

void store_indirect(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
}

void store_register(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;

    uint16_t pc_offset = sign_extend(instr & 0x3F, 6);

    mem_write(reg[r1] + pc_offset, reg[r0]);
}

void read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);
    
    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

uint16_t swap16(uint16_t x)
{
    return (x << 8 ) | (x >> 8);
}

int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
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
