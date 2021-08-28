CFLAGS=-ansi -Wall -pedantic -g
assembler: main.o pass_one.o pass_two.o line_analysis.o tables.o files.o memory_mgmt.o
	gcc $(CFLAGS) main.o pass_one.o pass_two.o line_analysis.o tables.o files.o memory_mgmt.o -o assembler

main_prog.o: prog.c assembler.h
	gcc -c $(CFLAGS) prog.c -o main.o

pass_one.o: pass_one.c assembler.h
	gcc -c $(CFLAGS) pass_one.c -o pass_one.o

pass_two.o: pass_two.c assembler.h
	gcc -c $(CFLAGS) pass_two.c -o pass_two.o

line_analysis.o: line_analysis.c assembler.h
	gcc -c $(CFLAGS) line_analysis.c -o line_analysis.o

tables.o: tables.c assembler.h
	gcc -c -ansi -Wall -pedantic tables.c -o tables.o

files.o: files.c assembler.h
	gcc -c $(CFLAGS) files.c -o files.o

memory_mgmt.o: memory_mgmt.c assembler.h
	gcc -c $(CFLAGS) memory_mgmt.c -o memory_mgmt.o

clean:
	rm -rf *.o assembler
