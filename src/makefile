INC_PATH=../include

CFLAGS=-ansi -Wall -pedantic -g -I$(INC_PATH)
GCC=gcc

_DEPS = assembler.h
DEPS = $(patsubst %,$(INC_PATH)/%,$(_DEPS))

BIN_PATH=../bin
_OBJ =  main.o pass_one.o pass_two.o line_analysis.o tables.o files.o memory_mgmt.o
OBJ =  $(patsubst %,$(BIN_PATH)/%,$(_OBJ))

$(BIN_PATH)/assembler: $(OBJ)
	$(GCC) $(CFLAGS) $(OBJ) -o $(BIN_PATH)/assembler

$(BIN_PATH)/main.o: main.c $(DEPS)
	$(GCC) -c $(CFLAGS) main.c -o $(BIN_PATH)/main.o

$(BIN_PATH)/pass_one.o: pass_one.c $(DEPS)
	$(GCC) -c $(CFLAGS) pass_one.c -o $(BIN_PATH)/pass_one.o

$(BIN_PATH)/pass_two.o: pass_two.c $(DEPS)
	$(GCC) -c $(CFLAGS) pass_two.c -o $(BIN_PATH)/pass_two.o

$(BIN_PATH)/line_analysis.o: line_analysis.c $(DEPS)
	$(GCC) -c $(CFLAGS) line_analysis.c -o $(BIN_PATH)/line_analysis.o

$(BIN_PATH)/tables.o: tables.c $(DEPS)
	$(GCC) -c $(CFLAGS) tables.c -o $(BIN_PATH)/tables.o

$(BIN_PATH)/files.o: files.c $(DEPS)
	$(GCC) -c $(CFLAGS) files.c -o $(BIN_PATH)/files.o

$(BIN_PATH)/memory_mgmt.o: memory_mgmt.c $(DEPS)
	$(GCC) -c $(CFLAGS) memory_mgmt.c -o $(BIN_PATH)/memory_mgmt.o

test: $(BIN_PATH)/assembler
	cd ../smoke_test/ && bash smoke_test.sh ps

clean:
	rm -rf $(BIN_PATH)/*.o $(BIN_PATH)/assembler
