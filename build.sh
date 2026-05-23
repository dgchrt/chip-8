#!/bin/sh

# Simple build script for the CHIP-8 emulator
# Requires: gcc, make

echo "Building CHIP-8 Emulator..."
make

if [ $? -eq 0 ]; then
    echo "Build successful! Executable is at dist/chip8"
else
    echo "Build failed."
    exit 1
fi
