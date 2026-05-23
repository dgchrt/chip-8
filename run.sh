#!/bin/sh

# Simple run script for the CHIP-8 emulator

if [ ! -f "dist/chip8" ]; then
    echo "Executable dist/chip8 not found. Please build it first with ./build.sh"
    exit 1
fi

if [ $# -lt 1 ]; then
    echo "Usage: $0 file.rom"
    exit 1
fi

./dist/chip8 "$@"
