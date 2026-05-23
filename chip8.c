#include "chip8.h"

void check_keys() {
	if (kbhit()) {
		keys[get_key()] = 1;
	}
}

void cycle() {
	while(running) {
		fetch();
		decode();
		update();
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

void draw() {
	unsigned short i;
	unsigned char symbol;
	unsigned char pixel1;
	unsigned char pixel2;
	unsigned short x;
	unsigned short y;

	should_draw = 0;

	for (y = 0; y < GRAPHICS_HEIGHT; y += 2) {
		gotoxy(MARGIN_LEFT + 1, MARGIN_TOP + 1 + (y / 2));

		for (x = 0; x < GRAPHICS_WIDTH; ++x) {
			i = (y * GRAPHICS_WIDTH) + x;
			pixel1 = graphics[i];
			pixel2 = graphics[i + GRAPHICS_WIDTH];

			if (pixel1 > 0 && pixel2 > 0) {
				symbol = CHAR_WHOLE;
			} else if (pixel1 > 0) {
				symbol = CHAR_TOP;
			} else if (pixel2 > 0) {
				symbol = CHAR_BOTTOM;
			} else {
				symbol = CHAR_SPACE;
			}

			putch(symbol);
		}
	}

	if (frequency) {
		delay(MILLISECONDS / frequency);
	}
}

void fetch() {
	unsigned char byte1 = memory[program_counter];
	unsigned char byte2 = memory[program_counter + 1];
	instruction = byte1 << 8 | byte2;
}

unsigned char get_key() {
	switch(getch()) {
		case KEY_1: return 0x1;
		case KEY_2: return 0x2;
		case KEY_3: return 0x3;
		case KEY_4: return 0xc;
		case KEY_Q: return 0x4;
		case KEY_W: return 0x5;
		case KEY_E: return 0x6;
		case KEY_R: return 0xd;
		case KEY_A: return 0x7;
		case KEY_S: return 0x8;
		case KEY_D: return 0x9;
		case KEY_F: return 0xe;
		case KEY_Z: return 0xa;
		case KEY_X: return 0x0;
		case KEY_C: return 0xb;
		case KEY_V: return 0xf;

		case KEY_ESC:
			running = 0;
		break;
	}
}

void iterate_program_counter() {
	program_counter += INSTRUCTION_BYTES;
}

void instruction_00e0() {
	/* 00E0 - Clears the screen. */
	unsigned short i;

	for (i = 0; i < GRAPHICS_SIZE; ++i) {
		graphics[i] = 0x0;
	}

	should_draw = 1;
	iterate_program_counter();
}

void instruction_00ee() {
	/* 00EE - Returns from a subroutine. */
	program_counter = stack_pull();
	iterate_program_counter();
}

void instruction_0nnn() {
	/* 0NNN - Calls RCA 1802 program at address NNN. Not necessary for most ROMs. */
	iterate_program_counter();
}

void instruction_1nnn() {
	/* 1NNN - Jumps to address NNN. */
	program_counter = operand_nnn();
}

void instruction_2nnn() {
	/* 2NNN - Calls subroutine at NNN. */
	stack_push(program_counter);
	program_counter = operand_nnn();
}

void instruction_3xnn() {
	/* 3XNN - Skips the next instruction if VX equals NN. */
	if (registers[operand_x()] == operand_nn()) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_4xnn() {
	/* 4XNN - Skips the next instruction if VX doesn't equal NN. */
	if (registers[operand_x()] != operand_nn()) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_5xy0() {
	/* 5XY0 - Skips the next instruction if VX equals VY. */
	if (registers[operand_x()] == registers[operand_y()]) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_6xnn() {
	/* 6XNN - Sets VX to NN. */
	registers[operand_x()] = operand_nn();
	iterate_program_counter();
}

void instruction_7xnn() {
	/* 7XNN - Adds NN to VX. */
	registers[operand_x()] += operand_nn();
	iterate_program_counter();
}

void instruction_8xy0() {
	/* 8XY0 - Sets VX to the value of VY. */
	registers[operand_x()] = registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy1() {
	/* 8XY1 - Sets VX to VX or VY. */
	registers[operand_x()] |= registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy2() {
	/* 8XY2 - Sets VX to VX and VY. */
	registers[operand_x()] &= registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy3() {
	/* 8XY3 - Sets VX to VX xor VY. */
	registers[operand_x()] ^= registers[operand_y()];
	iterate_program_counter();
}

void instruction_8xy4() {
	/* 8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. */
	unsigned short x = operand_x();
	unsigned short y = operand_y();
	registers[0xf] = 0xff - registers[x] < registers[y] ? 1 : 0;
	registers[x] += registers[y];
	iterate_program_counter();
}

void instruction_8xy5() {
	/* 8XY5 - VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
	unsigned short x = operand_x();
	unsigned short y = operand_y();
	registers[0xf] = registers[x] < registers[y] ? 0 : 1;
	registers[x] -= registers[y];
	iterate_program_counter();
}

void instruction_8xy6() {
	/* 8XY6 - Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift. */
	unsigned short x = operand_x();
	registers[0xf] = registers[x] & 0x1;
	registers[x] >>= 1;
	iterate_program_counter();
}

void instruction_8xy7() {
	/* 8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
	unsigned short x = operand_x();
	unsigned short y = operand_y();
	registers[0xf] = registers[x] > registers[y] ? 0 : 1;
	registers[x] = registers[y] - registers[x];
	iterate_program_counter();
}

void instruction_8xye() {
	/* 8XYE - Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift. */
	unsigned short x = operand_x();
	registers[0xf] = registers[x] >> 7;
	registers[x] <<= 1;
	iterate_program_counter();
}

void instruction_9xy0() {
	/* 9XY0 - Skips the next instruction if VX doesn't equal VY. */
	if (registers[operand_x()] != registers[operand_y()]) {
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_annn() {
	/* ANNN - Sets I to the address NNN. */
	index_register = operand_nnn();
	iterate_program_counter();
}

void instruction_bnnn() {
	/* BNNN - Jumps to the address NNN plus V0. */
	program_counter = operand_nnn() + registers[0x0];
}

void instruction_cxnn() {
	/* CXNN - Sets VX to the result of a bitwise and operation on a random number and NN. */
	registers[operand_x()] = operand_nn() & rand();
	iterate_program_counter();
}

void instruction_dxyn() {
	/* DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen. */
	unsigned char x = registers[operand_x()];
	unsigned char y = registers[operand_y()];
	unsigned char height = operand_n();
	unsigned short i;
	registers[0xf] = 0x0;

	for (i = 0; i < height; ++i) {
		unsigned char data = memory[index_register + i];
		unsigned short j;

		for (j = 0; j < SPRITE_WIDTH; ++j) {
			if (data & (0x80 >> j)) {
				unsigned short position = (x + j + ((y + i) * GRAPHICS_WIDTH));

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

void instruction_ex9e() {
	/* EX9E - Skips the next instruction if the key stored in VX is pressed. */
	if (keys[registers[operand_x()]]) {
		keys[registers[operand_x()]] = 0;
		iterate_program_counter();
	}

	iterate_program_counter();
}

void instruction_exa1() {
	/* EXA1 - Skips the next instruction if the key stored in VX isn't pressed. */
	if (!keys[registers[operand_x()]]) {
		iterate_program_counter();
	}

	keys[registers[operand_x()]] = 0;
	iterate_program_counter();
}

void instruction_fx07() {
	/* FX07 - Sets VX to the value of the delay timer. */
	registers[operand_x()] = delay_timer;
	iterate_program_counter();
}

void instruction_fx0a() {
	/* FX0A - A key press is awaited, and then stored in VX. */
	registers[operand_x()] = get_key();
	iterate_program_counter();
}

void instruction_fx15() {
	/* FX15 - Sets the delay timer to VX. */
	delay_timer = registers[operand_x()];
	iterate_program_counter();
}

void instruction_fx18() {
	/* FX18 - Sets the sound timer to VX. */
	sound_timer = registers[operand_x()];
	iterate_program_counter();
}

void instruction_fx1e() {
	/* FX1E - Adds VX to I. */
	index_register += registers[operand_x()];
	iterate_program_counter();
}

void instruction_fx29() {
	/* FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. */
	index_register = registers[operand_x()] * FONT_HEIGHT;
	iterate_program_counter();
}

void instruction_fx33() {
	/* FX33 - Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.) */
	unsigned short x = operand_x();
	memory[index_register] = registers[x] / 100;
	memory[index_register + 1] = (registers[x] / 10) % 10;
	memory[index_register + 2] = (registers[x] % 100) % 10;
	iterate_program_counter();
}

void instruction_fx55() {
	/* FX55 - Stores V0 to VX (including VX) in memory starting at address I. */
	unsigned char x = operand_x();
	unsigned short i;

	for (i = 0; i <= x; ++i) {
		memory[index_register + i] = registers[i];
	}

	iterate_program_counter();
}

void instruction_fx65() {
	/* FX65 - Fills V0 to VX (including VX) with values from memory starting at address I. */
	unsigned char x = operand_x();
	unsigned short i;

	for (i = 0; i <= x; ++i) {
		registers[i] = memory[index_register + i];
	}

	iterate_program_counter();
}

void load_font() {
	unsigned short i;

	unsigned char data[80] = { \
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

void load_rom(char path[]) {
	FILE *file;
	unsigned char byte;
	unsigned short i;

	file = fopen(path, "rb");

	if (!file) {
		printf("Can't open %s!\n", path);
		exit(1);
	}

	printf("Loading %s...\n", path);
	i = 0;

	while (!feof(file)) {
		fread(&byte, 1, 1, file);
		memory[PROGRAM_START + i++] = byte;
	}

	fclose(file);
	clrscr();
	cycle();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s file.rom [frequency]\n", argv[0]);
		exit(1);
	} else {
		if (argc > 2) {
			frequency = atoi(argv[2]);
		}

		load_font();
		load_rom(argv[1]);
		clrscr();
	}

	return 0;
}

void not_implemented() {
	running = 0;
	printf("Instruction %x not implemented.\n", instruction);
	exit(1);
}

unsigned char operand_n() {
	return instruction & OPERAND_N;
}

unsigned char operand_nn() {
	return instruction & OPERAND_NN;
}

unsigned short operand_nnn() {
	return instruction & OPERAND_NNN;
}

unsigned char operand_x() {
	return (instruction & OPERAND_X) >> BYTE_BITS;
}

unsigned char operand_y() {
	return (instruction & OPERAND_Y) >> HALF_BYTE_BITS;
}

unsigned short stack_pull() {
	return stack[--stack_pointer];
}

void stack_push(unsigned short data) {
	stack[stack_pointer++] = data;
}

void update() {
	check_keys();

	if (delay_timer > 0) {
		--delay_timer;
	}

	if (sound_timer > 0) {
		if (--sound_timer == 0) {
			putch(BELL);
		}
	}

	if (should_draw) {
		draw();
	}
}
