#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "translator.h"

#ifndef INTPTR_MAX
#include <inttypes.h>
#endif

/** TODO match command type */
/** TODO determine arg1 and arg2 from command type */
const int operation(char *vm_op)
{
        const char *c_op;
        int i, j;
        for (i = 0; i < HM_OP_CODES; i++) {
                if (!strcmp((op_codes+i)->vm_code, vm_op)) {
                        j = (op_codes+i)->op;
                        break;
                }
                j = ERROR;
        }

        return j;
}

/*{{{tokenize(char **str_lst, char *vm_nstr)*/
int tokenize(char **str_lst, char *vm_nstr)
{
        char *t_nstr = vm_nstr;
        int l_inc = 0, t_inc = 0;
        /** = malloc(sizeof(uintptr_t)*3) */
        char *tmp;
        tmp = strtok(t_nstr, " ");
        const int command = operation(tmp);
        printf("%s ", command);

        while (tmp) {
                /** tmp = (char*)malloc(sizeof(char)*8); */
                printf("%s\n", tmp);
                tmp = strtok(NULL, " ");
        }

        return 0;
}
/*}}}*/

/*{{{parse(char *vm_nstr, FILE *outfile)*/
int parse(char *vm_nstr, FILE *outfile)
{
        int c;
        if ((c = *vm_nstr) == '/' || c == '\0' || c == '\n' || c == '\r')
                return 0;

        char **str_lst;
        tokenize(str_lst, vm_nstr);

        return 0;
}
/*}}}*/

/*{{{main*/
/** translates VM code into hack assembly code */
int main(int argc, char *argv[])
{
	char *in, *out;

        if (argc == 1) {
                printf("Must specify an input file name with .vm extention.\n");
                return ERROR;
        }
        if (strstr(*(argv+1), ".vm" ) == NULL) {
                printf("Must specify an input file name with .vm extention.\n");
                return ERROR;
        }

	in = *(argv+1);
        out = "out.asm";

	while (--argc)
		if (strcmp(*(argv+argc), "-o") == 0) {
                        if (strstr(*(argv+argc+1), ".asm" ) != NULL) {
                                out = *(argv+argc+1);
                        } else {
                                printf("output file must have .asm extention.\n");
                                return ERROR;
                        }
                }

        FILE *infile, *outfile;
        char vm_nstr[MAX_LINE_LENGTH];

        if ((infile = fopen(in, "r")) == NULL) {
                printf("Error occured reading input file");
                return ERROR;
        }

        if ((outfile = fopen(out, "w")) == NULL) {
                printf("Error occured opening output file");
                return ERROR;
        }

        while (fgets(vm_nstr, MAX_LINE_LENGTH, infile) != NULL)
                parse(vm_nstr, outfile);

        return 0;
}
/*}}}*/
