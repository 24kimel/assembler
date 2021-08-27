#include <math.h>
enum ARG_LIMITS{
    MIN_ARGUMENTS =  2,
    MAX_ARGUMENTS  = 4
};

#define printerr(MSG) fprintf(stderr,"%s %s %d\n",MSG,__FILE__,__LINE__)

/*boolean enum (FALSE = 0, TRUE = 1):*/
typedef enum {
    FALSE,
    TRUE
} boolean;


/*in this enum I keep all the limit to strings, such sa the line read from the assembly file*/
enum CHAR_LIMITS{
    MAX_LINE = 80,
    MAX_FILE_NAME  = 64,
    MAX_LABEL = 31
};

/*integer limits using two's complement:*/
#define IMMED_MAX 32767
#define IMMED_MIN -32768
#define in_lim(X,N)  ((X)<(pow(2,(N)-1)-1) && (X)>-pow(2,(N)-1))? 1:0
#define memory_lim(X)   ((X)>=0 && (X)<=MEMORY_MAX)? 1:0

#define MEMORY_MAX  pow(2,25)-1
/*line analysis:*/
#define endline(C)  ((C)=='\n' || (C)==EOF)? TRUE:FALSE
#define NUM_ORDERS  27

int read_line(FILE *fp, char* line);
int length_check(char *line);
int empty(char *line);
int meaningless(char *line);
int spaceln(char c);
int register_num(char *line, int err);
int is_label(char *line, int err);
int start_label(char *line);
void scan_label (char *line, char* label);
char* scan_op(char *line, char *op);
int next_op(char *line, int comma);
int ent_ext(char *line);
int order_structure(char *line);
int order_index(char *line);
int is_data(char *line);
int asciz_len(char *line);
int compatible_args(char *line);
int get_num_args(char *line);

/*the two assembler passes:*/
int pass_one(char *filename);
void pass_two(char *filename);

/*add the name of the current file to the error print:*/
#define add_err_detail(NAME,LINE)   fprintf(stderr,"[file: %s. line: %lu]",NAME,LINE)

void build_tables();


/*opcode table:*/
enum COMMAND_TYPES {
    R_CMD = 1,
    I_CMD = 2,
    J_CMD = 3
};

enum DATA_DIRECTIVES{
    DB = 1,
    DH = 2,
    ASCIZ = 3,
    DW = 4
};

enum LABEL_DIRECTIVES{
    ENTRY = 1,
    EXTERN = 2
};

/*symbol table:*/
typedef struct symbol{
    struct symbol *next;
    long unsigned address;
    int attribute;
    char *symbol;
    boolean is_entry;
}symbol_node;

enum ATTRIBUTES{
    CODE = 1,
    DATA = 2,
    EXTERNAL = 3
};

int add_symbol(unsigned address, char *symbol, int attribute, int is_entry);
void update_symbol_table(unsigned ICF);
int add_ent(char *symbol);

char* filename(char* name);
void output(char *filename);
int num_files (int argc);

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


/*unions, structs and functions related to the binary image of the source file:*/

int complete_missing_info(char *label, char order_type, unsigned IC);
unsigned get_opcode(char *line);
void cmd_to_info(char *line, unsigned IC);

void data_to_info(char *line);
void update_data_img(unsigned ICF);


typedef struct order_info {
    char name[5];
    unsigned opcode:6;
    unsigned funct:5; /*only relevant for orders of the type R*/
} cmd_info;

typedef struct external_label_list {
    unsigned address:25;
    char *label;
    struct external_label_list *next;
}ext_node;

/*external label list:*/

void add_to_ext_list(unsigned address, char *label);
/*dynamic binding*/
void alloc_check(void* x);
void mem_allocate();
void mem_deallocate();

void initialize_tables();
