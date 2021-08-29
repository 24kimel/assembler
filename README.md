# assembler
C project for the open university. 

an assembler is basically a "translator" from a programming language called assembly
to machine code - a sequence of zeros and one which the computer can understand.
the assembler reads an assembly file and tries to translate it into machine code.
if there is an error, the assembler will report it, and not finish its process,
at the end of which, 1-3 output files will be made for the assembly file used as an input.

# the imaginary computer:
this project is built for an imaginary computer, which contains a CPU, registers and memory(RAM).

the imaginary computer has 32 registers that are numbered $0,$1,$2,...,$31. every register's size is 32 bit.

the memory of the imaginary computer contains 2^25 cells - each cell is sized at 1 byte (8 bits)

the imaginary computer works only with integers, and the arithmetic is done using the 2's complement method.

the imaginary computer can also work with characters that can be represented ny an ascii code.

# the structure of an assembly program
the assembly language I use for this project may be similar to real assembly,
but it is not assembly code that a computer can understand.
It was crafted by the instructors specifically for this project.

# for more information check the guide
