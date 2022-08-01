#include <inttypes.h>


struct m_segment {
	char **memory;            /* Pointer to array of pointers representing value stack */
	const int16_t addr;       /* address in RAM, i.e. 0 - 15 */
	const int16_t base;				/* Base address of segment and address of first value  */
	const int16_t upper;      /* Upper limit of possible values in stack */
	int16_t ith;              /* Ith element of value in stack */
};

#define C_PUSH(data, segment) (segment->(memory+segment->ith)) = data
#define C_POP(data, segment) (segment->(memory+segment->ith))

struct m_segment SP, LCL, ARG, THIS, THAT, TEMP5, TEMP6, TEMP7, TEMP8, TEMP9, TEMP10, TEMP11, TEMP12, R13, R14, R15;

const char *command_type(const char* arg);  /* returns constant representing the type of command */
struct m_segment *addr_to_mem(int addr);           /* translates memory address to segment name */

