#include <inttypes.h>

typedef int16_t DATA;				/* Signed 16-bit int is the only hack datatype */


struct m_segment {
	DATA memory[64];
	// const int16_t addr;    /* address in RAM, i.e. 0 - 15 */
	// const int16_t base;		/* Base address of segment and address of first value  */
	// const int16_t upper;		/*  Upper limit of possible values in stack */
	int16_t ith;              /* Ith element of value in stack */
};

enum RAM_SEG {
	SP,
	LCL,
	ARG,
	THIS,
	THAT,
} RAM;

struct m_segment ram[sizeof(RAM)] = {
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
};

/**
 * TODO map memory segment string to symbol string to enum
 * in a struct. I just realized no arithmetic actually has to be done.
 * writing the correct asm is the next step.
 **/

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
	char *vm_code;	// plain text command as realized while parsing
	enum OPS op;		// enum version to be used in switch
	int has_arg2;		// Does a second argument exist
};

struct op_code op_codes[] = {
	"push", C_PUSH, 1,
	"pop", C_POP, 1,
	"add", C_ARITHMETIC, 0,
	"sub", C_ARITHMETIC, 0,
};

/* how many op codes */
#define HM_OP_CODES (sizeof op_codes/sizeof(struct op_code))

const int operation(char *op);								/* finds type of current commnad */
// const char command_type(const char* arg);		 [> returns constant representing the type of command <]
// struct m_segment *addr_to_mem(int addr);			 [> translates memory address to segment name <]
int parse(char *vm_nstr, FILE *outfile);				/* Entrypoint for instruction translation */
int tokenize(FILE *outfile, char *vm_nstr);					/* seperates string into tokens to call operations */
int writepp(FILE *outfile, char *arg1, char *arg2); /* write push pop commands to output */

