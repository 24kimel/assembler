/****************************************************************************
* Title                 :   Header File for the whole Assembler
* Filename              :   adc_app.h
* Author                :   Itai Kimelman
* Version               :   1.5.0
*****************************************************************************/
/** \file assembler.h
 *  \brief This module contains all the function, macros, and constants that the assembler uses
 *
 *  This is the header file for the assembler
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <math.h>
/******************************************************************************
* Constants
*******************************************************************************/
/*argument amount limits*/
enum ARG_LIMITS {
    MIN_ARGUMENTS =  2,
    MAX_ARGUMENTS  = 4
};

/*boolean enum (FALSE = 0, TRUE = 1):*/
typedef enum {
    FALSE,
    TRUE
} boolean;

/*opcode table:*/
enum COMMAND_TYPES {
    R_CMD = 1,
    I_CMD = 2,
    J_CMD = 3
};

/*data directive return values for is_data*/
enum DATA_DIRECTIVES {
    DB = 1,
    DH = 2,
    ASCIZ = 3,
    DW = 4
};

/*label directive return values for ent_ext*/
enum LABEL_DIRECTIVES {
    ENTRY = 1,
    EXTERN = 2
};

/*attributes for symbols (see symbol_node)*/
enum ATTRIBUTES {
    CODE = 1,
    DATA = 2,
    EXTERNAL = 3
};

/*in this enum I keep all the limit to strings, such sa the line read from the assembly file*/
enum CHAR_LIMITS{
    MAX_LINE = 80,
    MAX_FILE_NAME  = 64,
    MAX_LABEL = 31
};

/*for output() function*/
enum BYTES_TO_PRINT {
    ONE_BYTE = 1,
    HALF_WORD = 2,
    WORD = 4
};

#define NOT_REG -1
#define REG_MIN 0
#define REG_MAX 31
#define NON_VALID_OPERAND -1
#define NUM_ORDERS  27
#define NON_REAL_OPCODE 64
/******************************************************************************
* Macros
*******************************************************************************/
#define endline(C)  ((C)=='\n' || (C)==EOF)? TRUE:FALSE
/*checks for bit range (2's complement)*/
#define in_lim(X,N)  ((X)<(pow(2,(N)-1)-1) && (X)>-pow(2,(N)-1))? 1:0
/******************************************************************************
* Typedefs
*******************************************************************************/
/******************************************************************************
* Typedefs for Opcode Table
*******************************************************************************/
/*order info struct(for opcode table)*/
typedef struct order_info {
    char name[5];
    unsigned opcode:6;
    unsigned funct:5; /*only relevant for orders of the type R*/
} cmd_info;

/******************************************************************************
* Typedefs for the Symbol Table
*******************************************************************************/
/*symbol node for the symbol table:*/
typedef struct symbol{
    struct symbol *next;
    long unsigned address;
    int attribute;
    char *symbol;
    boolean is_entry;
}symbol_node;

/******************************************************************************
* Typedefs for the Orders
*******************************************************************************/
/*unions, structs and functions related to the binary image of the source file:*/
typedef struct line_in_binary{
    unsigned b1:8;
    unsigned b2:8;
    unsigned b3:8;
    unsigned b4:8;
}word;

typedef struct type_r_cmd{
    unsigned zeros:6;
    unsigned funct:5;
    unsigned rd:5;
    unsigned rt:5;
    unsigned rs:5;
    unsigned opcode:6;
}r_command;

typedef struct type_i_cmd{
    int immed:16;
    unsigned rt:5;
    unsigned rs:5;
    unsigned opcode:6;
}i_command;

typedef struct type_j_cmd{
    unsigned address:25;
    unsigned reg:1;
    unsigned opcode:6;
}j_command;

typedef union cmd_in_binary{
    r_command r_cmd;
    i_command i_cmd;
    j_command j_cmd;
    word w;
}cmd_in_binary;

typedef struct cmd_img{
    cmd_in_binary machine_code;
    unsigned address:25;
}command_image;

/******************************************************************************
* Typedefs for Data Directives
*******************************************************************************/
typedef struct two_bytes{
    unsigned b1:8;
    unsigned b2:8;
}half_word;

typedef union dh_in_binary{
    int img:16;
    half_word h;
}dh_img;

typedef union dw_in_binary{
    int img:32;
    word w;
}dw_img;

/*this union unites all the options for printable, dividable data*/
typedef union data_in_binary{
    unsigned b:8;
    dh_img dh;
    dw_img dw;
}data_in_binary;

typedef struct data_img {
    data_in_binary machine_code; /*all the info about the data*/
    unsigned address:25; /*directive address*/
    unsigned bytes_taken:3; /*1,2 or 4*/
} data_image;

/******************************************************************************
* Typedefs for The External Label List
*******************************************************************************/
/*external label node (for the external label list)*/
typedef struct external_label_list {
    unsigned address:25;
    char *label;
    struct external_label_list *next;
}ext_node;

/******************************************************************************
* Function Prototypes
*******************************************************************************/
/******************************************************************************
* Line Analysis Function Prototypes
*******************************************************************************/
int read_line(FILE *fp, char* line);
int length_check(char *line);
int empty(char *line);
int meaningless(char *line);
int spaceln(char c);
int register_num(char *line, int err);
int is_label(char *line, int err);
int start_label(char *line);
void scan_label (char *line, char* label);
void scan_op(char *line, char *op);
int next_op(char *line, int comma);
int ent_ext(char *line);
int order_structure(char *line);
int order_index(char *line);
int is_data(char *line);
int asciz_len(char *line);
int compatible_args(char *line);
int get_num_args(char *line);

/******************************************************************************
* Function Prototypes for Tables
*******************************************************************************/
void initialize_tables();

/******************************************************************************
* Function Prototypes for Order Lines
*******************************************************************************/
int num_ops_expected(unsigned opcode);
int complete_missing_info(char *label, char order_type, unsigned long IC);
unsigned get_opcode(char *line);
void cmd_to_info(char *line, unsigned IC);

/******************************************************************************
* Function Prototypes for Data Directive Lines
*******************************************************************************/
void data_to_info(char *line);
void update_data_img(unsigned ICF);

/******************************************************************************
* Function Prototypes for the Symbol Table
*******************************************************************************/
int add_symbol(unsigned address, char *symbol, int attribute, int is_entry);
void update_symbol_table(unsigned long ICF);
int add_ent(char *symbol);

/******************************************************************************
* Function Prototypes for Files
*******************************************************************************/
char* filename(char* name);
int output(char *file_name);
int num_files (int argc);

/******************************************************************************
* Function Prototypes for the External Label List
*******************************************************************************/
void add_to_ext_list(unsigned address, char *label);

/******************************************************************************
* Function Prototypes for Memory Management
*******************************************************************************/
void alloc_check(void* x);
void mem_allocate();
void mem_deallocate();

/******************************************************************************
* The Two Assembler Passes Function Prototypes
*******************************************************************************/
int pass_one(char *filename);
int pass_two(char *filename);

/*** End of File **************************************************************/