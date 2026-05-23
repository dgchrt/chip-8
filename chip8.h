#include <stdio.h>

#define BELL 7
#define BYTE_BITS 8
#define CHAR_BOTTOM 220
#define CHAR_SPACE 32
#define CHAR_TOP 223
#define CHAR_WHOLE 219
#define FONT_ADDRESS 0x50
#define FONT_HEIGHT 0x5
#define GRAPHICS_HEIGHT 32
#define GRAPHICS_SIZE 0x800
#define GRAPHICS_WIDTH 64
#define HALF_BYTE_BITS 4
#define INSTRUCTION_BYTES 2
#define KEY_ESC 27
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_4 52
#define KEY_A 97
#define KEY_C 99
#define KEY_D 100
#define KEY_E 101
#define KEY_F 102
#define KEY_Q 113
#define KEY_R 114
#define KEY_S 115
#define KEY_V 118
#define KEY_W 119
#define KEY_X 120
#define KEY_Z 122
#define KEYS_SIZE 0x10
#define MARGIN_LEFT 8
#define MARGIN_TOP 4
#define MEMORY_SIZE 0x1000
#define MILLISECONDS 1000
#define OPERAND_N 0x000f
#define OPERAND_NN 0x00ff
#define OPERAND_NNN 0x0fff
#define OPERAND_X 0x0f00
#define OPERAND_Y 0x00f0
#define PROGRAM_START 0x200
#define REGISTER_SIZE 0x10
#define SPRITE_WIDTH 8
#define STACK_SIZE 0x10

unsigned char delay_timer = 0;
unsigned char frequency = 0;
unsigned char graphics[GRAPHICS_SIZE];
unsigned short index_register;
unsigned short instruction;
unsigned char keys[KEYS_SIZE];
unsigned char memory[MEMORY_SIZE];
unsigned short program_counter = PROGRAM_START;
unsigned char registers[REGISTER_SIZE];
unsigned char running = 1;
unsigned char should_draw = 0;
unsigned char sound_timer = 0;
unsigned short stack[STACK_SIZE];
unsigned short stack_pointer = 0;

void check_keys();
void cycle();
void decode();
void draw();
void fetch();
unsigned char get_key();
void iterate_program_counter();
void load_font();
void load_rom(char path[]);
void not_implemented();
void instruction_00e0();
void instruction_00ee();
void instruction_0nnn();
void instruction_1nnn();
void instruction_2nnn();
void instruction_3xnn();
void instruction_4xnn();
void instruction_5xy0();
void instruction_6xnn();
void instruction_7xnn();
void instruction_8xy0();
void instruction_8xy1();
void instruction_8xy2();
void instruction_8xy3();
void instruction_8xy4();
void instruction_8xy5();
void instruction_8xy6();
void instruction_8xy7();
void instruction_8xye();
void instruction_9xy0();
void instruction_annn();
void instruction_bnnn();
void instruction_cxnn();
void instruction_dxyn();
void instruction_ex9e();
void instruction_exa1();
void instruction_fx07();
void instruction_fx0a();
void instruction_fx15();
void instruction_fx18();
void instruction_fx1e();
void instruction_fx29();
void instruction_fx33();
void instruction_fx55();
void instruction_fx65();
unsigned char operand_n();
unsigned char operand_nn();
unsigned short operand_nnn();
unsigned char operand_x();
unsigned char operand_y();
unsigned short stack_pull();
void stack_push(unsigned short data);
void update();
