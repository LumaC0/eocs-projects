#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

void print_file(FILE *, FILE *);

typedef struct {
	char *name;
  char *value;
} SYMBOLS;

size_t jmp_size = 8;
SYMBOLS jmp[] = {
	"null", "000",
	"JGT", "001",
	"JEQ", "010",
	"JGE", "011",
	"JLT", "100",
	"JNE", "101",
	"JLE", "110",
	"JMP", "111"
};

size_t comp_size = 28;
SYMBOLS comp[] = {
	"0",   "101010",
	"1",   "111111",
	"-1",  "001100",
	"D",   "001100",
	"A",   "110000",
	"M",   "110000",
	"!D",  "001101",
	"!A",  "110001",
	"!M",  "110001",
	"-D",  "001111",
	"-A",  "110011",
	"-M",  "110011",
	"D+1", "011111",
	"A+1", "110111",
	"M+1", "110111",
	"D-1", "001110",
	"A-1", "110010",
	"M-1", "110010",
	"D+A", "000010",
	"D+M", "000010",
	"D-A", "010011",
	"D-M", "010011",
	"A-D", "000111",
	"M-D", "000111",
	"D&A", "000000",
	"D&M", "000000",
	"D|A", "010101",
	"D|M", "010101",
};


size_t dest_size = 8;	
SYMBOLS dest[] = {
	"null", "000",
	"M",    "001",
	"D",    "010",
	"DM",   "011",
	"A",    "100",
	"AM",   "101",
	"AD",   "110",
	"ADM",  "111",
};

size_t builtins_size = 22;	
SYMBOLS builtins[] = {
		"R0", "0",
		"R1", "1",
		"R2", "2",
		"R3", "3",
		"R4", "4",
		"R5", "5",
		"R6", "6",
		"R7", "7",
		"R8", "8",
		"R9", "9",
		"R10", "10",
		"R11", "11",
		"R12", "12",
		"R13", "13",
		"R14", "14",
		"R15", "15",
		"SP", "0",
		"LCL", "1",
		"ARG", "2",
		"THIS", "3",
		"THAT", "4",
		"SCREEN", "16384",
		"KBD", "24576"
};


void  _iter_(SYMBOLS *arr, size_t size) {
	while (size--) {
		printf("name is %s and val is %s\n", arr->name, arr->value);
		arr++;
	}
}


#define MAX_LINE 256

#define BIT_RANGE 16

#define MAX_ADDR (int) pow(2, BIT_RANGE-1)

int symbol(char *line, FILE *out) {
	line = ++line;

	char *tmp = line;
	char nstr[BIT_RANGE];

	int val = atoi(line);
	
	int i = 0;
	while (i < BIT_RANGE) {
		/** adding '0' + int for type cast to char  */
		nstr[i++]  =  (!!((val << i) & MAX_ADDR)) + '0';
		
	}
	nstr[i++] = '\n';
	nstr[i] = '\0';

	/** printf("%s", nstr); */
	/** printf("%c", '\n'); */
	
	if (fputs(nstr, out) == EOF) {
		printf("Error writing symbols to out file");
		return -1;
	}

	return 0;
}
int c_instruction(char *, FILE *) {
	return 0;
}

typedef enum {
	A_NSTR,
	C_NSTR,
	L_NSTR
		
} INST_TYPE;

#define NSTR_TYPE(c) ((c == '@') || (c == '(') \
		? (c == '@') ? (A_NSTR) : (L_NSTR) \
		: (C_NSTR))


int parser(char *infile, char *outfile) {
	char c;
	FILE *in, *out;
	char line[MAX_LINE];

	in = fopen(infile, "r");
	out = fopen(outfile, "w");
	
	while (fgets(line, MAX_LINE, in) != NULL) {

		c = *line;

		if (c  == '/' || c == ' ' || c == '\0' || c == '\n') {
			continue;
		}

		INST_TYPE _type = NSTR_TYPE(c);

		if (_type == A_NSTR || _type ==  L_NSTR) {
			symbol(line, out);
		} else {
			c_instruction(line, out);
		}
	}

	fclose(in);
	fclose(out);
	return 0;
}
	
int main(int argc, char *argv[]) {

	char *infile, *outfile;

	if (argc == 2) {
			infile = argv[1];
			outfile = "Prog.hack";
		} 
	else if (argc == 3) {
			infile = argv[1];
			outfile = argv[2];
		}
	else {
		printf("wrong number of arguments supplied.");
		return -1;
	}
	printf("%s %s\n", infile, outfile);
	

	if (parser(argv[1], outfile) == -1) {
		printf("%s", "Error parsing");
		return -1;
		}

	return 0;
}


/** void print_file(FILE *ifp, FILE *ofp) { */
/**   char *c; */
/**   while ((c = fgetc(ifp)) != EOF) { */
/**  */
/**     if (c == '/') { */
/**       while (c = fgetc(ifp) != '\n')  */
/**         continue; */
/**       continue; */
/**     } */
/**  */
/**     putc(c, ofp); */
/**   } */
/** } */
