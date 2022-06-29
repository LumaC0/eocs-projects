#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

/** void print_file(FILE *, FILE *); */

typedef struct {
	char *name;
  char *value;
} SYMBOLS;

/** {{{ jmp struct */
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
/** }}} */

/** {{{ comp struct */
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
/** }}} */

/** {{{ dest struct */
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
/* }}} */

/** {{{ builtins struct */
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
/** }}} */

/** void  _iter_(SYMBOLS *arr) { */
/**   size_t s = sizeof(arr); */
/**   printf("the array has %i values\n", s); */
	/** while (size--) { */
	/**   printf("name is %s and val is %s\n", arr->name, arr->value); */
	/**   arr++; */
	/** } */
/** } */


/** TODO incorporate jmp lookup code */
/** TODO incorporate symbol table */

#define MAX_LINE 256

#define BIT_RANGE 16

#define MAX_ADDR (int) pow(2, BIT_RANGE-1)

char *c_comp_dest(char *line, char *eq_pt) {

	int comp_start = labs(line - eq_pt); 
	char *comp_sym = line + comp_start + 1;

	char *nstr = (char *) malloc(BIT_RANGE);

	strcpy(nstr, "111");
	/** finding comp code */
	for (int i = 0; i < comp_size; i++) {

		if (strcmp((comp+i)->name, comp_sym) == -13) {

			strcat(nstr, (strchr(comp_sym, 'M') != NULL) ? "1": "0");
			strcat(nstr, (comp+i)->value);
			
		}
	}	
	/** finding dest code */
	*(line+comp_start) = '\0';
	for (int i = 0; i < dest_size; i++) {
		if (strcmp((dest+i)->name, line) == 0) {
			strcat(nstr, (dest+i)->value);
		}
	}
	/** add no jump */
	strcat(nstr, "000\n\0");

	return nstr;	
}
// }}} 

char *c_jump(char *line) {
	;
	/** char *comp_delim = strchr( */
}
	

int c_instruction(char *line, FILE *out) {
	char *nstr, *c;

	if ((c = strchr(line, '=')) != NULL) {
		nstr = c_comp_dest(line, c);

	} 
	/** else { */
	/**   nstr = c_jump(line); */

		/** coded_nstr = dst_comp(line); */
	/** while ((c = *(_iter_nstr++)) != '\n') { */
	/**   if */
	/** } */

	if (fputs(nstr, out) == EOF) {
		printf("Error writing symbols to out file");
		return -1;
	}
	/** printf("%s\n", nstr); */

	return 0;
}


int symbol(char *line, FILE *out) {

	char *tmp = line, nstr[BIT_RANGE];

	if (isdigit(*++line)) {
		int num;
		num = atoi(line);

		int i = 0;
		while (i < BIT_RANGE) {
			/** adding '0' + int for type cast to char  */
			nstr[i++]  =  (!!((num << i) & MAX_ADDR)) + '0';
			
		}
		nstr[i++] = '\n';
		nstr[i] = '\0';
	}

	/** printf("%s\n", nstr); */
	
	if (fputs(nstr, out) == EOF) {
		printf("Error writing symbols to out file");
		return -1;
	}

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

		} else if (_type == C_NSTR) {

			c_instruction(line, out);

		} else 

			continue;
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
	/** printf("%s %s\n", infile, outfile); */
	

	if (parser(argv[1], outfile) == -1) {
		printf("%s", "Error parsing");
		return -1;
		}

	/** _iter_(comp); */
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
