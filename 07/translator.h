#include <inttypes.h>

typedef int16_t DATA;				/* Signed 16-bit int is the only hack datatype */

struct m_segment {
	DATA **memory;            /* Pointer to array of pointers representing value stack */
	// const int16_t addr;       [> address in RAM, i.e. 0 - 15 <]
	const int16_t base;				/* Base address of segment and address of first value  */
	// const int16_t upper;      [> Upper limit of possible values in stack <]
	int16_t ith;              /* Ith element of value in stack */
};

struct m_segment SP, LCL, ARG, THIS, THAT;

#define MAX_LINE_LENGTH 100		/* Max length of read for fgets */
#define ERROR -1              /* Return value when error is encountered */

#define PUSH(data, segment) (segment->(memory+segment->ith)) = data
#define POP(data, segment) (segment->(memory+segment->ith))

/* #define C_ARITHMETIC "C_ARITHMETIC" // */
/* #define C_PUSH "C_PUSH"						  // */
/* #define C_POP "C_POP"							  // */
/* #define C_LABEL "C_LABEL"					  // */
/* #define C_GOTO "C_GOTO"						  // bytecode command options */
/* #define C_IF "C_IF"								  // */
/* #define C_FUNCTION "C_FUNCTION"		  // */
/* #define C_RETURN "C_RETURN"				  // */
/* #define C_CALL "C_CALL"						  // */

enum OPS {
	C_ARITHMETIC,
	C_PUSH,
	C_POP,
};

struct op_code {
	char *vm_code;
	enum OPS op;
};
struct op_code op_codes[] = {
	"push", C_PUSH,
	"pop", C_POP,
	"add", C_ARITHMETIC,
	"sub", C_ARITHMETIC
};

/* how many op codes */
#define HM_OP_CODES (sizeof op_codes/sizeof(struct op_code))

const char *operation(char *op);								/* finds type of current commnad */
// const char command_type(const char* arg);		 [> returns constant representing the type of command <]
// struct m_segment *addr_to_mem(int addr);			 [> translates memory address to segment name <]
int parse(char *vm_nstr, FILE *outfile);				/* Entrypoint for instruction translation */
int split_string(char **str_lst, char *vm_nstr);/* utility to split string into list of strings */

