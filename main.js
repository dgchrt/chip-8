"use strict";

var BYTE_SIZE = 8;
var FONT_ADDRESS = 0x050;
var FONT_HEIGHT = 0x5;
var FREQUENCY = 60;
var INSTRUCTION_BYTES = 2;
var MAX_CYCLES = 32;
var MILLISECONDS = 1000;
var PROGRAM_COUNTER_START = 0x200;
var SCREEN_HEIGHT = 32;
var SCREEN_WIDTH = 64;
var SPRITE_WIDTH = 8;

var beepSound;
var canvas;
var delayTimer;
var frameTime;
var graphics;
var indexRegister;
var isRunning;
var keys;
var lastRender;
var memory;
var operationCode;
var programCounter;
var registers;
var screenSize;
var shouldDraw;
var soundTimer;
var stack;
var stackPointer;

function byteSize(value) {
	return value & 0xff;
}

function cycle() {
	fetch();
	decode();
	update();
}

function decode() {
	switch (operationCode & 0xf000) {
	case 0x0000:
		switch (getNN()) {
		case 0x00e0:
			operation00E0();
			break;

		case 0x00ee:
			operation00EE();
			break;

		default:
			operation0NNN();
			break;
		}

		break;

	case 0x1000:
		operation1NNN();
		break;

	case 0x2000:
		operation2NNN();
		break;

	case 0x3000:
		operation3XNN();
		break;

	case 0x4000:
		operation4XNN();
		break;

	case 0x5000:
		switch (operationCode & 0x000f) {
		case 0x0000:
			operation5XY0();
			break;
		}

		break;

	case 0x6000:
		operation6XNN();
		break;

	case 0x7000:
		operation7XNN();
		break;

	case 0x8000:
		switch (operationCode & 0x000f) {
		case 0x0000:
			operation8XY0();
			break;

		case 0x0001:
			operation8XY1();
			break;

		case 0x0002:
			operation8XY2();
			break;

		case 0x0003:
			operation8XY3();
			break;

		case 0x0004:
			operation8XY4();
			break;

		case 0x0005:
			operation8XY5();
			break;

		case 0x0006:
			operation8XY6();
			break;

		case 0x0007:
			operation8XY7();
			break;

		case 0x000e:
			operation8XYE();
			break;
		}

		break;

	case 0x9000:
		switch (operationCode & 0x000f) {
		case 0x0000:
			operation9XY0();
			break;
		}

		break;

	case 0xa000:
		operationANNN();
		break;

	case 0xb000:
		operationBNNN();
		break;

	case 0xc000:
		operationCXNN();
		break;

	case 0xd000:
		operationDXYN();
		break;

	case 0xe000:
		switch (getNN()) {
		case 0x009e:
			operationEX9E();
			break;

		case 0x00a1:
			operationEXA1();
			break;
		}

		break;

	case 0xf000:
		switch (getNN()) {
		case 0x0007:
			operationFX07();
			break;

		case 0x000a:
			operationFX0A();
			break;

		case 0x0015:
			operationFX15();
			break;

		case 0x0018:
			operationFX18();
			break;

		case 0x001e:
			operationFX1E();
			break;

		case 0x0029:
			operationFX29();
			break;

		case 0x0033:
			operationFX33();
			break;

		case 0x0055:
			operationFX55();
			break;

		case 0x0065:
			operationFX65();
			break;
		}

		break;

	default:
		notImplemented();
		break;
	}
}

function fetch() {
	// instructions are two-byte long
	var byte1 = memory[programCounter];
	var byte2 = memory[programCounter + 1];
	operationCode = byte1 << BYTE_SIZE | byte2;
}

function getNN() {
	return (operationCode & 0x00ff);
}

function getNNN() {
	return (operationCode & 0x0fff);
}

function getX() {
	return (operationCode & 0x0f00) >> 8;
}

function getY() {
	return (operationCode & 0x00f0) >> 4;
}

function iterateProgramCounter() {
	programCounter += INSTRUCTION_BYTES;
}

function loadFont() {
	var FONT_DATA = [
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80 // F
	];

	for (var i = 0; i < FONT_DATA.length; ++i) {
		memory[i + FONT_ADDRESS] = FONT_DATA[i];
	}
}

function loadProgram(buffer) {
	reset();

	for (var i = 0; i < buffer.length; ++i) {
		memory[i + PROGRAM_COUNTER_START] = buffer[i];
	}

	isRunning = true;
}

function loop() {
	if (isRunning) {
		for (var i = 0; !shouldDraw && i < MAX_CYCLES; ++i) {
			cycle();
		}

		shouldDraw && requestAnimationFrame(render);
	}

	setTimeout(loop, frameTime + lastRender - Date.now());
}

function notImplemented() {
	isRunning = false;
	console.log("Operation code 0x" + operationCode.toString(16) + " not implemented.");
}

function random(max) {
	return Math.floor(Math.random() * max + 1);
}

function render() {
	shouldDraw = false;
	var context = canvas.getContext("2d");

	for (var i = 0; i < screenSize; ++i) {
		var x = i % SCREEN_WIDTH;
		var y = Math.floor(i / SCREEN_WIDTH);
		context.fillStyle = graphics[i] ? "black" : "white";
		context.fillRect(x, y, 1, 1);
	}

	lastRender = Date.now();
}

function reset() {
	delayTimer = 0;
	graphics = [];
	indexRegister = 0;
	memory = [];
	operationCode = 0x0;
	programCounter = PROGRAM_COUNTER_START;
	registers = [];
	shouldDraw = true;
	soundTimer = 0;
	stack = [];
	stackPointer = 0;
	loadFont();
}

function setupFileInput() {
	var fileInput = document.getElementById("fileInput");
	fileInput.addEventListener("change", onFileInputChange);

	function onFileInputChange() {
		var fileReader = new FileReader();
		fileReader.addEventListener("load", onFileReaderLoad);
		fileReader.readAsBinaryString(fileInput.files[0]);

		function onFileReaderLoad() {
			console.log("Read " + fileReader.result.length + " bytes from file.");
			var buffer = [];

			for (var i = 0; i < fileReader.result.length; ++i) {
				buffer[i] = fileReader.result.charCodeAt(i);
			}

			loadProgram(buffer);
		}
	}
}

function setupKeyboard() {
	var KEY_MAP = {
		"49" : 0x1, // 1
		"50" : 0x2, // 2
		"51" : 0x3, // 3
		"52" : 0xc, // 4
		"65" : 0x7, // A
		"67" : 0xb, // C
		"68" : 0x9, // D
		"69" : 0x6, // E
		"70" : 0xe, // F
		"81" : 0x4, // Q
		"82" : 0xd, // R
		"83" : 0x8, // S
		"86" : 0xf, // V
		"87" : 0x5, // W
		"88" : 0x0, // X
		"90" : 0xa // Z
	};

	addEventListener("keydown", onKeyDown, false);
	addEventListener("keyup", onKeyUp, false);

	function onKeyDown(event) {
		var key = KEY_MAP[event.keyCode];
		key && (keys[key] = 1) && event.preventDefault();
	}

	function onKeyUp(event) {
		var key = KEY_MAP[event.keyCode];
		key && (keys[key] = 0);
	}
}

function update() {
	delayTimer > 0 && --delayTimer;
	soundTimer > 0 && --soundTimer == 0 && beepSound.play();
}

// operations
function operation00E0() {
	// 00E0 - Clears the screen.
	graphics = [];
	shouldDraw = true;
	iterateProgramCounter();
}

function operation00EE() {
	// 00EE - Returns from a subroutine.
	programCounter = stack[--stackPointer];
	iterateProgramCounter();
}

function operation0NNN() {
	// 0NNN - Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
	iterateProgramCounter();
}

function operation1NNN() {
	// 1NNN - Jumps to address NNN.
	programCounter = getNNN();
}

function operation2NNN() {
	// 2NNN - Calls subroutine at NNN.
	stack[stackPointer++] = programCounter;
	programCounter = getNNN();
}

function operation3XNN() {
	// 3XNN - Skips the next instruction if VX equals NN.
	var x = getX();
	var nn = getNN();
	registers[x] == nn && iterateProgramCounter();
	iterateProgramCounter();
}

function operation4XNN() {
	// 4XNN - Skips the next instruction if VX doesn't equal NN.
	var x = getX();
	var nn = getNN();
	registers[x] != nn && iterateProgramCounter();
	iterateProgramCounter();
}

function operation5XY0() {
	// 5XY0 - Skips the next instruction if VX equals VY.
	var x = getX();
	var y = getY();
	registers[x] == registers[y] && iterateProgramCounter();
	iterateProgramCounter();
}

function operation6XNN() {
	// 6XNN - Sets VX to NN.
	var x = getX();
	var nn = getNN();
	registers[x] = nn;
	iterateProgramCounter();
}

function operation7XNN() {
	// 7XNN - Adds NN to VX.
	var x = getX();
	var nn = getNN();
	registers[x] = byteSize(registers[x] + nn);
	iterateProgramCounter();
}

function operation8XY0() {
	// 8XY0 - Sets VX to the value of VY.
	var x = getX();
	var y = getY();
	registers[x] = registers[y];
	iterateProgramCounter();
}

function operation8XY1() {
	// 8XY1 - Sets VX to VX or VY.
	var x = getX();
	var y = getY();
	registers[x] = registers[x] | registers[y];
	iterateProgramCounter();
}

function operation8XY2() {
	// 8XY2 - Sets VX to VX and VY.
	var x = getX();
	var y = getY();
	registers[x] = registers[x] & registers[y];
	iterateProgramCounter();
}

function operation8XY3() {
	// 8XY3 - Sets VX to VX xor VY.
	var x = getX();
	var y = getY();
	registers[x] = registers[x] ^ registers[y];
	iterateProgramCounter();
}

function operation8XY4() {
	// 8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
	var x = getX();
	var y = getY();
	registers[0xf] = registers[y] > (0xff - registers[x]) ? 1 : 0;
	registers[x] = byteSize(registers[x] + registers[y]);
	iterateProgramCounter();
}

function operation8XY5() {
	// 8XY5 - VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
	var x = getX();
	var y = getY();
	registers[0xf] = registers[x] < registers[y] ? 0 : 1;
	registers[x] -= registers[y];
	iterateProgramCounter();
}

function operation8XY6() {
	// 8XY6 - Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
	var x = getX();
	registers[0xf] = registers[x] & 0x1;
	registers[x] >>= 1;
	iterateProgramCounter();
}

function operation8XY7() {
	// 8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
	var x = getX();
	var y = getY();
	registers[0xf] = registers[x] > registers[y] ? 0 : 1;
	registers[x] = registers[y] - registers[x];
	iterateProgramCounter();
}

function operation8XYE() {
	// 8XYE - Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
	var x = getX();
	registers[0xf] = byteSize(registers[x] >> 7);
	registers[x] = byteSize(registers[x] << 1);
	iterateProgramCounter();
}

function operation9XY0() {
	// 9XY0 - Skips the next instruction if VX doesn't equal VY.
	var x = getX();
	var y = getY();
	registers[x] != registers[y] && iterateProgramCounter();
	iterateProgramCounter();
}

function operationANNN() {
	// ANNN - Sets I to the address NNN.
	indexRegister = getNNN();
	iterateProgramCounter();
}

function operationBNNN() {
	// BNNN - Jumps to the address NNN plus V0.
	programCounter = getNNN() + registers[0x0];
}

function operationCXNN() {
	// CXNN - Sets VX to the result of a bitwise and operation on a random number and NN.
	var x = getX();
	var nn = getNN();
	registers[x] = nn & random(0xff);
	iterateProgramCounter();
}

function operationDXYN() {
	// DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen.
	var x = registers[getX()];
	var y = registers[getY()];
	var height = (operationCode & 0x000f);
	registers[0xf] = 0;

	for (var i = 0; i < height; ++i) {
		var pixel = memory[indexRegister + i];

		for (var j = 0; j < SPRITE_WIDTH; ++j) {
			if ((pixel & byteSize(0x80 >> j))) {
				var position = (x + j + ((y + i) * SCREEN_WIDTH));
				graphics[position] && (registers[0xf] = 1);
				graphics[position] ^= 1;
			}
		}
	}

	shouldDraw = true;
	iterateProgramCounter();
}

function operationEX9E() {
	// EX9E - Skips the next instruction if the key stored in VX is pressed.
	var x = getX();
	keys[registers[x]] && iterateProgramCounter();
	iterateProgramCounter();
}

function operationEXA1() {
	// EXA1 - Skips the next instruction if the key stored in VX isn't pressed.
	var x = getX();
	!keys[registers[x]] && iterateProgramCounter();
	iterateProgramCounter();
}

function operationFX07() {
	// FX07 - Sets VX to the value of the delay timer.
	var x = getX();
	registers[x] = delayTimer;
	iterateProgramCounter();
}

function operationFX0A() {
	// FX0A - A key press is awaited, and then stored in VX.
	for (var i in keys) {
		registers[getX()] = i;
		iterateProgramCounter();
	}
}

function operationFX15() {
	// FX15 - Sets the delay timer to VX.
	delayTimer = registers[getX()];
	iterateProgramCounter();
}

function operationFX18() {
	// FX18 - Sets the sound timer to VX.
	soundTimer = registers[getX()];
	iterateProgramCounter();
}

function operationFX1E() {
	// FX1E - Adds VX to I.
	var x = getX();
	indexRegister += registers[x];
	iterateProgramCounter();
}

function operationFX29() {
	// FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
	indexRegister = registers[getX()] * FONT_HEIGHT;
	iterateProgramCounter();
}

function operationFX33() {
	// FX33 - Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
	var x = getX();
	memory[indexRegister] = Math.floor(registers[x] / 100);
	memory[indexRegister + 1] = Math.floor(registers[x] / 10) % 10;
	memory[indexRegister + 2] = (registers[x] % 100) % 10;
	iterateProgramCounter();
}

function operationFX55() {
	// FX55 - Stores V0 to VX (including VX) in memory starting at address I.
	var x = getX();

	for (var i = 0; i <= x; ++i) {
		memory[indexRegister + i] = registers[i];
	}

	iterateProgramCounter();
}

function operationFX65() {
	// FX65 - Fills V0 to VX (including VX) with values from memory starting at address I.
	var x = getX();

	for (var i = 0; i <= x; ++i) {
		registers[i] = memory[indexRegister + i];
	}

	iterateProgramCounter();
}

beepSound = document.getElementById("beepSound");
canvas = document.getElementsByTagName("canvas")[0];
keys = {};
lastRender = Date.now();
screenSize = SCREEN_WIDTH * SCREEN_HEIGHT;
setupFileInput();
setupKeyboard();
frameTime = MILLISECONDS / FREQUENCY;
loop();
