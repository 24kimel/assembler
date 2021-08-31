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

every line in the assembly language is 1 of 4 types:

1. blank line - this is a line which only contains white spaces
2. comment line - this is a line that its first character that is not white is ';'
3. a directive line - (I will refer to it later)
4. an order line - (I will refer to it later)

# directives
a directive line may start with a label(optional)
after the label the comes name of the directive.
after the name of the directive comes the parameters
there are 2 main types of directives:
1. a data storage directive:
a data storage directive (.db,.dh,.dw,.asciz).
.db,.dh,.dw directives tell the computer to store 1,2, or 4 bytes respectively (1 for .db, 2 for.dh, 4 for .dw)
the parameters of these directives are whole number, with a comma separating between each two
examples: .db 6,-9,-57,0
          .dh 27056
          .dw 31,-12,4,31,1111
note that the number of parametes is limited only to the size of the memory of the imaginary computer.
.asciz directive tells the computer to store a string of characters.
the string will be contained in double qoutation marks.
the string will be terminated by the null character ('\0')
example: .asciz "HELLO WORLD"
note that the number of characters is limited onlyto the size of the memory of the imaginary computer.
2. non-storage directives:
these directives have only one parameter that is a valid label.
they tell the computer something about a label:
.entry makes its parameter an entry point so other files can use it.
.extern imports its parameter from another file, so the program can use that label, even if it's not declared in this file

note that .extern can only be used on a label that is an entry point in another file 
(but this assembler does not check that. it works on each input file individually)

# orders
an order line

# project structure
typical project tree with the following directories:
--> top level: this directory, README
	--> bin			executable and obj files will be placed here after build
	--> include		h file(s)
	--> smoke_test		script for basic test of the assembler
		--> gold	input and expected output files for the test script
	--> src			c source files, makefile
to build the assembler go to /src and run 'make'
to build the assembler and run basic test go to /src and run 'make test'
to run basic test go to /smoke_test and run 'bash smoke_test.sh [f1] [f2] [f3]', read the script documentation for more information
