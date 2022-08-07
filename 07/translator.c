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
const char *command_type(char *op)
{
        const char *c_op;
        return "nothing";
}

/*{{{split_string(char **str_lst, char *vm_nstr)*/
int split_string(char **str_lst, char *vm_nstr)
{
        char *t_nstr = vm_nstr;
        char *tmp_str;
        char c;
        int l_inc = 0, t_inc = 0;

        while (1) {
                c = *(t_nstr+t_inc);
                if (c == 32) {
                        tmp_str = (char*)malloc(sizeof(char)*16);
                        strncpy(tmp_str, t_nstr, t_inc);
                        *(str_lst+l_inc++) = tmp_str;
                        t_nstr+=t_inc;
                        t_nstr++;
                        t_inc = 0;
               }
                if (c == 0) {
                        tmp_str = (char*)malloc(sizeof(char)*16);
                        strncpy(tmp_str, t_nstr, --t_inc);
                        *(str_lst+l_inc) = tmp_str;
                        break;
                }
                t_inc++;
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

        char **str_lst = malloc(sizeof(uintptr_t)*3);
        split_string(str_lst, vm_nstr);

        const char *command;
        command = command_type(*(str_lst++));
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
