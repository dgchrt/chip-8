#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct timespec cycle_delay = {0, 0};
uint8_t delay_timer = 0;
uint32_t frequency = 0;
uint8_t graphics[GRAPHICS_SIZE];
uint16_t index_register;
uint16_t instruction;
uint8_t keys[KEYS_SIZE];
uint8_t memory[MEMORY_SIZE];
uint16_t program_counter = PROGRAM_START;
uint8_t registers[REGISTER_SIZE];
uint8_t running = 1;
uint8_t should_draw = 0;
uint8_t sound_timer = 0;
uint16_t stack[STACK_SIZE];
uint16_t stack_pointer = 0;

void check_keys(void) {
	if (kbhit()) {
		uint8_t key = get_key();
		if (key < KEYS_SIZE) {
			keys[key] = 1;
		}
	}
}

void cycle(void) {
	while(running) {
		if (!hal_handle_events()) {
			running = 0;
			break;
		}

		fetch();
		decode();
		update();
	}
}

void update(void) {
	check_keys();

	if (delay_timer > 0) {
		--delay_timer;
	}

	if (sound_timer > 0) {
		if (--sound_timer == 0) {
			hal_log("BEEP!");
		}
	}

	if (should_draw) {
		draw();
	}
}

void decode() {
	switch (instruction & 0xf000) {
		case 0x0000:
			switch (operand_nn()) {
				case 0x00e0:
					instruction_00e0();
				break;

				case 0x00ee:
					instruction_00ee();
				break;

				default:
					instruction_0nnn();
				break;
			}
		break;

		case 0x1000:
			instruction_1nnn();
		break;

		case 0x2000:
			instruction_2nnn();
		break;

		case 0x3000:
			instruction_3xnn();
		break;

		case 0x4000:
			instruction_4xnn();
		break;

		case 0x5000:
			switch (instruction & 0x000f) {
				case 0x0000:
					instruction_5xy0();
				break;

				default:
					not_implemented();
				break;
			}
		break;

		case 0x6000:
			instruction_6xnn();
		break;

		case 0x7000:
			instruction_7xnn();
		break;

		case 0x8000:
			switch (instruction & 0x000f) {
				case 0x0000:
					instruction_8xy0();
				break;

				case 0x0001:
					instruction_8xy1();
				break;

				case 0x0002:
					instruction_8xy2();
				break;

				case 0x0003:
					instruction_8xy3();
				break;

				case 0x0004:
					instruction_8xy4();
				break;

				case 0x0005:
					instruction_8xy5();
				break;

				case 0x0006:
					instruction_8xy6();
				break;

				case 0x0007:
					instruction_8xy7();
				break;

				case 0x000e:
					instruction_8xye();
				break;

				default:
					not_implemented();
				break;
			}
		break;

		case 0x9000:
			switch (instruction & 0x000f) {
				case 0x0000:
					instruction_9xy0();
				break;

				default:
					not_implemented();
				break;
			}
		break;

		case 0xa000:
			instruction_annn();
		break;

		case 0xb000:
			instruction_bnnn();
		break;

		case 0xc000:
			instruction_cxnn();
		break;

		case 0xd000:
			instruction_dxyn();
		break;

		case 0xe000:
			switch (operand_nn()) {
				case 0x009e:
					instruction_ex9e();
				break;

				case 0x00a1:
					instruction_exa1();
				break;

				default:
					not_implemented();
				break;
			}
		break;

		case 0xf000:
			switch (operand_nn()) {
				case 0x0007:
					instruction_fx07();
				break;

				case 0x000a:
					instruction_fx0a();
				break;

				case 0x0015:
					instruction_fx15();
				break;

				case 0x0018:
					instruction_fx18();
				break;

				case 0x001e:
					instruction_fx1e();
				break;

				case 0x0029:
					instruction_fx29();
				break;

				case 0x0033:
					instruction_fx33();
				break;

				case 0x0055:
					instruction_fx55();
				break;

				case 0x0065:
					instruction_fx65();
				break;

				default:
					not_implemented();
				break;
			}
		break;

		default:
			not_implemented();
		break;
	}
}

void draw(void) {
	uint32_t display_buffer[GRAPHICS_SIZE];
	int i;

	should_draw = 0;

	for (i = 0; i < GRAPHICS_SIZE; ++i) {
		display_buffer[i] = graphics[i] ? 0xFFFFFFFF : 0xFF000000;
	}

	hal_present(display_buffer, GRAPHICS_WIDTH, GRAPHICS_HEIGHT);

	if (frequency > 0) {
		nanosleep(&cycle_delay, NULL);
	}
}

void fetch(void) {
	uint8_t byte1 = memory[program_counter];
	uint8_t byte2 = memory[program_counter + 1];
	instruction = byte1 << 8 | byte2;
}

uint8_t get_key(void) {
	int ch = getch();
	switch(ch) {
		case '1': return 0x1;
		case '2': return 0x2;
		case '3': return 0x3;
		case '4': return 0xc;
		case 'q': return 0x4;
		case 'w': return 0x5;
		case 'e': return 0x6;
		case 'r': return 0xd;
		case 'a': return 0x7;
		case 's': return 0x8;
		case 'd': return 0x9;
		case 'f': return 0xe;
		case 'z': return 0xa;
		case 'x': return 0x0;
		case 'c': return 0xb;
		case 'v': return 0xf;

		case 27: // ESC
			running = 0;
			return 0xFF;
		
		default:
			return 0xFF;
	}
}

void iterate_program_counter(void) {
	program_counter += INSTRUCTION_BYTES;
}

void instruction_00e0(void) {
	/* 00E0 - Clears the screen. */
	uint16_t i;

	for (i = 0; i < GRAPHICS_SIZE; ++i) {
		graphics[i] = 0x0;
	}

	should_draw = 1;
	iterate_program_counter();
}

void instruction_00ee(void) {
	/* 00EE - Returns from a subroutine. */
	program_counter = stack_pull();
	iterate_program_counter();
}

void instruction_0nnn(void) {
	/* 0NNN - Calls RCA 1802 program at address NNN. Not necessary for most ROMs. */
	iterate_program_counter();
}

void instruction_1nnn(void) {
	/* 1NNN - Jumps to address NNN. */
	program_counter = operand_nnn();
}

void instruction_2nnn(void) {
	/* 2NNN - Calls subroutine at NNN. */
	stack_push(program_counter);
	program_counter = operand_nnn();
}

void instruction_3xnn(void) {
	/* 3XNN - Skips the next instruction if VX equals NN. */
	if (registers[operand_x()] == operand_nn()) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_4xnn(void) {
	/* 4XNN - Skips the next instruction if VX doesn't equal NN. */
	if (registers[operand_x()] != operand_nn()) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_5xy0(void) {
	/* 5XY0 - Skips the next instruction if VX equals VY. */
	if (registers[operand_x()] == registers[operand_y()]) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_6xnn(void) {
	/* 6XNN - Sets VX to NN. */
	registers[operand_x()] = operand_nn();
	iterate_program_counter();
}

void instruction_7xnn(void) {
	/* 7XNN - Adds NN to VX. */
	registers[operand_x()] += operand_nn();
	iterate_program_counter();
}

void instruction_8xy0(void) {
	/* 8XY0 - Sets VX to the value of VY. */
	registers[operand_x()] = registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy1(void) {
	/* 8XY1 - Sets VX to VX or VY. */
	registers[operand_x()] |= registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy2(void) {
	/* 8XY2 - Sets VX to VX and VY. */
	registers[operand_x()] &= registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy3(void) {
	/* 8XY3 - Sets VX to VX xor VY. */
	registers[operand_x()] ^= registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy4(void) {
	/* 8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. */
	uint16_t x = operand_x();
	uint16_t y = operand_y();
	registers[0xf] = 0xff - registers[x] < registers[y] ? 1 : 0;
	registers[x] += registers[y];
	iterate_program_counter();
}

void instruction_8xy5(void) {
	/* 8XY5 - VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
	uint16_t x = operand_x();
	uint16_t y = operand_y();
	registers[0xf] = registers[x] < registers[y] ? 0 : 1;
	registers[x] -= registers[y];
	iterate_program_counter();
}

void instruction_8xy6(void) {
	/* 8XY6 - Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift. */
	uint16_t x = operand_x();
	registers[0xf] = registers[x] & 0x1;
	registers[x] >>= 1;
	iterate_program_counter();
}

void instruction_8xy7(void) {
	/* 8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
	uint16_t x = operand_x();
	uint16_t y = operand_y();
	registers[0xf] = registers[x] > registers[y] ? 0 : 1;
	registers[x] = registers[y] - registers[x];
	iterate_program_counter();
}

void instruction_8xye(void) {
	/* 8XYE - Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift. */
	uint16_t x = operand_x();
	registers[0xf] = registers[x] >> 7;
	registers[x] <<= 1;
	iterate_program_counter();
}

void instruction_9xy0(void) {
	/* 9XY0 - Skips the next instruction if VX doesn't equal VY. */
	if (registers[operand_x()] != registers[operand_y()]) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_annn(void) {
	/* ANNN - Sets I to the address NNN. */
	index_register = operand_nnn();
	iterate_program_counter();
}

void instruction_bnnn(void) {
	/* BNNN - Jumps to the address NNN plus V0. */
	program_counter = operand_nnn() + registers[0x0];
}

void instruction_cxnn(void) {
	/* CXNN - Sets VX to the result of a bitwise and operation on a random number and NN. */
	registers[operand_x()] = operand_nn() & (rand() % 256);
	iterate_program_counter();
}

void instruction_dxyn(void) {
	/* DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen. */
	uint8_t x = registers[operand_x()];
	uint8_t y = registers[operand_y()];
	uint8_t height = operand_n();
	uint16_t i;
	registers[0xf] = 0x0;

	for (i = 0; i < height; ++i) {
		uint8_t data = memory[index_register + i];
		uint16_t j;

		for (j = 0; j < SPRITE_WIDTH; ++j) {
			if (data & (0x80 >> j)) {
				uint16_t position = ((x + j) % GRAPHICS_WIDTH) + (((y + i) % GRAPHICS_HEIGHT) * GRAPHICS_WIDTH);

				if (graphics[position]) {
					registers[0xf] = 0x1;
				}

				graphics[position] ^= 1;
			}
		}
	}

	should_draw = 1;
	iterate_program_counter();
}

void instruction_ex9e(void) {
	/* EX9E - Skips the next instruction if the key stored in VX is pressed. */
	if (keys[registers[operand_x()]]) {
		keys[registers[operand_x()]] = 0;
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_exa1(void) {
	/* EXA1 - Skips the next instruction if the key stored in VX isn't pressed. */
	if (!keys[registers[operand_x()]]) {
		iterate_program_counter();
	}

	keys[registers[operand_x()]] = 0;
	iterate_program_counter();
}

void instruction_fx07(void) {
	/* FX07 - Sets VX to the value of the delay timer. */
	registers[operand_x()] = delay_timer;
	iterate_program_counter();
}

void instruction_fx0a(void) {
	/* FX0A - A key press is awaited, and then stored in VX. */
	uint8_t key = 0xFF;
	while (key == 0xFF) {
		key = get_key();
		if (!running) return;
	}
	registers[operand_x()] = key;
	iterate_program_counter();
}

void instruction_fx15(void) {
	/* FX15 - Sets the delay timer to VX. */
	delay_timer = registers[operand_x()];
	iterate_program_counter();
}

void instruction_fx18(void) {
	/* FX18 - Sets the sound timer to VX. */
	sound_timer = registers[operand_x()];
	iterate_program_counter();
}

void instruction_fx1e(void) {
	/* FX1E - Adds VX to I. */
	index_register += registers[operand_x()];
	iterate_program_counter();
}

void instruction_fx29(void) {
	/* FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. */
	index_register = FONT_ADDRESS + (registers[operand_x()] * FONT_HEIGHT);
	iterate_program_counter();
}

void instruction_fx33(void) {
	/* FX33 - Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.) */
	uint16_t x = operand_x();
	memory[index_register] = registers[x] / 100;
	memory[index_register + 1] = (registers[x] / 10) % 10;
	memory[index_register + 2] = registers[x] % 10;
	iterate_program_counter();
}

void instruction_fx55(void) {
	/* FX55 - Stores V0 to VX (including VX) in memory starting at address I. */
	uint8_t x = operand_x();
	uint16_t i;

	for (i = 0; i <= x; ++i) {
		memory[index_register + i] = registers[i];
	}

	iterate_program_counter();
}

void instruction_fx65(void) {
	/* FX65 - Fills V0 to VX (including VX) with values from memory starting at address I. */
	uint8_t x = operand_x();
	uint16_t i;

	for (i = 0; i <= x; ++i) {
		registers[i] = memory[index_register + i];
	}

	iterate_program_counter();
}

void load_font(void) {
	uint16_t i;

	uint8_t data[80] = { \
		0xF0, 0x90, 0x90, 0x90, 0xF0, \
		0x20, 0x60, 0x20, 0x20, 0x70, \
		0xF0, 0x10, 0xF0, 0x80, 0xF0, \
		0xF0, 0x10, 0xF0, 0x10, 0xF0, \
		0x90, 0x90, 0xF0, 0x10, 0x10, \
		0xF0, 0x80, 0xF0, 0x10, 0xF0, \
		0xF0, 0x80, 0xF0, 0x90, 0xF0, \
		0xF0, 0x10, 0x20, 0x40, 0x40, \
		0xF0, 0x90, 0xF0, 0x90, 0xF0, \
		0xF0, 0x90, 0xF0, 0x10, 0xF0, \
		0xF0, 0x90, 0xF0, 0x90, 0x90, \
		0xE0, 0x90, 0xE0, 0x90, 0xE0, \
		0xF0, 0x80, 0x80, 0x80, 0xF0, \
		0xE0, 0x90, 0x90, 0x90, 0xE0, \
		0xF0, 0x80, 0xF0, 0x80, 0xF0, \
		0xF0, 0x80, 0xF0, 0x80, 0x80};

	for (i = 0; i < 80; ++i) {
		memory[FONT_ADDRESS + i] = data[i];
	}
}

void load_rom(const char *path) {
	int read = hal_load(path, memory + PROGRAM_START, MEMORY_SIZE - PROGRAM_START);

	if (read < 0) {
		hal_log("Can't open ROM!");
		hal_exit(1);
	}

	char msg[256];
	snprintf(msg, sizeof(msg), "Loading %s... (%d bytes)", path, read);
	hal_log(msg);
	
	clrscr();
	cycle();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		hal_log("Usage: chip8 file.rom [frequency]");
		hal_exit(1);
	} else {
		if (argc > 2) {
			frequency = (uint32_t)atoi(argv[2]);
			if (frequency > 0) {
				const long ns_delay = (1000L * 1000L * 1000L) / frequency;
				cycle_delay.tv_sec = ns_delay / (1000L * 1000L * 1000L);
				cycle_delay.tv_nsec = ns_delay % (1000L * 1000L * 1000L);
			}
		}

		if (hal_init("CHIP-8", GRAPHICS_WIDTH, GRAPHICS_HEIGHT) < 0) {
			hal_log("HAL initialization failed!");
			hal_exit(1);
		}

		srand(time(NULL));
		load_font();
		load_rom(argv[1]);
		hal_cleanup();
	}

	return 0;
}

void not_implemented(void) {
	running = 0;
	char msg[256];
	snprintf(msg, sizeof(msg), "Instruction %x not implemented.", instruction);
	hal_log(msg);
	hal_exit(1);
}

uint8_t operand_n(void) {
	return instruction & OPERAND_N;
}

uint8_t operand_nn(void) {
	return (uint8_t)(instruction & OPERAND_NN);
}

uint16_t operand_nnn(void) {
	return instruction & OPERAND_NNN;
}

uint8_t operand_x(void) {
	return (uint8_t)((instruction & OPERAND_X) >> BYTE_BITS);
}

uint8_t operand_y(void) {
	return (uint8_t)((instruction & OPERAND_Y) >> HALF_BYTE_BITS);
}

uint16_t stack_pull(void) {
	return stack[--stack_pointer];
}

void stack_push(uint16_t data) {
	stack[stack_pointer++] = data;
}
