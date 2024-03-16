# Zune - a simple compiler

Compiles a source file written in a very simple custom programming language
to x86 64-bit assembly, and optionally builds an executable binary.
Grammar is defined in `grammar.txt`. Example source code: `test.zu`.

## Limitations
- No floating point operations - floating point assembly is a pain. Use even numbers when testing, to be safe.
- Linux only (uses Unix syscalls). Does not work on Mac out of the box. Developed on Lubuntu.

## Prerequisites
- `CMAKE`:<br>
  `$ sudo apt install binutils`<br>
  `$ sudo apt install cmake`<br>
  `$ sudo apt install build-essential`
- `c++` (`gcc` does not work for compiling, but does work for the final binary linking).
- If you want to build an executable binary, `NASM` (`sudo apt install nasm`) to compile assembly and `gcc` to link the output file into a binary.

## Building
- `$ cmake -S . -B build/` to prepare CMAKE files.
- To build, run `cmake --build build/`.
- Once built, to compile a source file into assembly, run `build/zune test.zu`. Add command-line arguments to compile to binary, e.g.:<br>
`$ build/zune test.zu -asm=./test.asm -bin=./test`<br>
Run `build/zune` to see all params.
- To manually build a binary from the resulting assembly file:

  `$ nasm -felf64 ./test.asm -o ./test.o`<br>
  `$ gcc -lc -z execstack ./test.o -o ./test`
