#ifndef CHIP8_H
#define CHIP8_H

#include "hal.h"
#include <stdint.h>

#define BYTE_BITS 8
#define FONT_ADDRESS 0x50
#define FONT_HEIGHT 0x5
#define GRAPHICS_HEIGHT 32
#define GRAPHICS_SIZE 0x800
#define GRAPHICS_WIDTH 64
#define HALF_BYTE_BITS 4
#define INSTRUCTION_BYTES 2
#define KEYS_SIZE 0x10
#define MEMORY_SIZE 0x1000
#define OPERAND_N 0x000f
#define OPERAND_NN 0x00ff
#define OPERAND_NNN 0x0fff
#define OPERAND_X 0x0f00
#define OPERAND_Y 0x00f0
#define PROGRAM_START 0x200
#define REGISTER_SIZE 0x10
#define SPRITE_WIDTH 8
#define STACK_SIZE 0x10

#define MS_PER_SECOND 1000

extern struct timespec cycle_delay;
extern uint8_t delay_timer;
extern uint32_t frequency;
extern uint8_t graphics[GRAPHICS_SIZE];
extern uint16_t index_register;
extern uint16_t instruction;
extern uint8_t keys[KEYS_SIZE];
extern uint8_t memory[MEMORY_SIZE];
extern uint16_t program_counter;
extern uint8_t registers[REGISTER_SIZE];
extern uint8_t running;
extern uint8_t should_draw;
extern uint8_t sound_timer;
extern uint16_t stack[STACK_SIZE];
extern uint16_t stack_pointer;

void check_keys(void);
void cycle(void);
void decode(void);
void draw(void);
void fetch(void);
uint8_t get_key(void);
void iterate_program_counter(void);
void load_font(void);
void load_rom(const char *path);
void not_implemented(void);
void instruction_00e0(void);
void instruction_00ee(void);
void instruction_0nnn(void);
void instruction_1nnn(void);
void instruction_2nnn(void);
void instruction_3xnn(void);
void instruction_4xnn(void);
void instruction_5xy0(void);
void instruction_6xnn(void);
void instruction_7xnn(void);
void instruction_8xy0(void);
void instruction_8xy1(void);
void instruction_8xy2(void);
void instruction_8xy3(void);
void instruction_8xy4(void);
void instruction_8xy5(void);
void instruction_8xy6(void);
void instruction_8xy7(void);
void instruction_8xye(void);
void instruction_9xy0(void);
void instruction_annn(void);
void instruction_bnnn(void);
void instruction_cxnn(void);
void instruction_dxyn(void);
void instruction_ex9e(void);
void instruction_exa1(void);
void instruction_fx07(void);
void instruction_fx0a(void);
void instruction_fx15(void);
void instruction_fx18(void);
void instruction_fx1e(void);
void instruction_fx29(void);
void instruction_fx33(void);
void instruction_fx55(void);
void instruction_fx65(void);
uint8_t operand_n(void);
uint8_t operand_nn(void);
uint16_t operand_nnn(void);
uint8_t operand_x(void);
uint8_t operand_y(void);
uint16_t stack_pull(void);
void stack_push(uint16_t data);
void update(void);

#endif
