#include "inc/mpc.h"

#include <stdio.h>
#include <editline/readline.h>
#include <editline/history.h>

#define STYLE_BOLD         "\033[1m"
#define STYLE_UNDERLINE    "\033[4m"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int tape[1024];
int* ip;
char inp;
int index;

void eval_symbol(mpc_ast_t* t) {
	char* a = t->contents;
	if(strcmp(a, "+") == 0) { ++*ip; }
	if(strcmp(a, "-") == 0) { --*ip; }
	if(strcmp(a, ">") == 0) { ip++; }
	if(strcmp(a, "<") == 0) { ip--; }
	if(strcmp(a, ".") == 0) { printf("%c", *ip); }
	if(strcmp(a, ",") == 0) { 
		scanf("%c", &inp);
		*ip = inp; 
		if (*ip == 0 || *ip == 10) {
			scanf("%c", &inp);
			*ip = inp;
		}
	}
}

void eval_loop(mpc_ast_t* t);
void eval_loop(mpc_ast_t* t) {
	for (int i = 0;; ++i) {
		if (strstr(t->children[i]->tag, "loop")) {
			eval_loop(t->children[i]);
		}
		else if (strstr(t->children[i]->tag, "expr")) {
			for (int j = 0; j < t->children[i]->children_num; ++j) {
				eval_symbol(t->children[i]->children[j]);
			}
		}
		else if (strcmp(t->children[i]->contents, "[") == 0) { 
			if (*ip != 0)
				continue; 
			else 
				break;
		}
		else if (strcmp(t->children[i]->contents, "]") == 0) { 
			i = -1; 
		}
	}
}

void read_bf(mpc_ast_t* t) {
	if (strstr(t->tag, "symbol")) { 
		return eval_symbol(t); 
	}
	if (strstr(t->tag, "loop")) {
		return eval_loop(t);
	}
	
	for (int i = 0; i < t->children_num; ++i) {
		if (strcmp(t->children[i]->contents, "[") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "]") == 0) { continue; }
		if(strcmp(t->children[i]->tag, "regex") == 0) { continue; }

		read_bf(t->children[i]);

	}
}

void print_tape() {
	index = ip - tape;
	int start = (index/10) * 10;
	int end = start + (10 - 1);
	int _index = index % 10;

	printf("\n----------------------------------------------------\n");

	printf("\n" STYLE_BOLD "Start" ANSI_COLOR_RESET " = %d\n", start);
	printf(STYLE_BOLD "End" ANSI_COLOR_RESET "   = %d\n", end);
	printf(STYLE_BOLD "Index" ANSI_COLOR_RESET " = %d\n", index);

	printf("\n+");
	for (int i = start; i <= end; ++i) {
		printf("-----+");
	}
	printf("\n|");
	for (int i = start; i <= end; ++i) {
		if ((i-start) == _index)
			printf(STYLE_BOLD ANSI_COLOR_GREEN " %03d" ANSI_COLOR_RESET " |", tape[i]);
		else
			printf(" %03d |", tape[i]);
	}
	printf("\n+");
	for (int i = start; i <= end; ++i) {
		printf("-----+");
	}
	printf("\n");
	for (int i = 0; i < _index; ++i) {
		printf("      ");
	}
	printf(STYLE_BOLD ANSI_COLOR_GREEN "   ^" ANSI_COLOR_RESET);
	printf("\n");
	for (int i = 0; i < _index; ++i) {
		printf("      ");
	}
	printf(STYLE_BOLD ANSI_COLOR_GREEN " %3d\n\n" ANSI_COLOR_RESET, index);
}

int main(int argc, char** argv) {

    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Loop = mpc_new("loop");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Bf = mpc_new("bf");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                 								\
        symbol  :  '+' | '-' | '.' | ',' | '<' | '>' | '+' ;       	\
        loop  :  '[' <expr>* ']' ;   								\
        expr  :  <loop> | <symbol>+ ; 		 						\
        bf  : /^/ <expr>* /$/ ;       								\
    ",
    Symbol, Loop, Expr, Bf);

    puts(STYLE_BOLD ANSI_COLOR_BLUE "Fck It! Version 0.0.0.1.0" ANSI_COLOR_RESET);
    puts("Press " STYLE_BOLD ANSI_COLOR_RED "Ctrl+c" ANSI_COLOR_RESET " to Exit\n");

    /*lval_eval(e, "def {fun} (\\ {args body} {def (head args) (\\ (tail args) body)})");*/

    ip = tape;

    while (1) {

        char* input = readline("fck> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Bf, &r)) {
            // mpc_ast_print(r.output);
            read_bf(r.output);
            putchar('\n');
            // index = ip - tape;
            // printf("%d\n", index);
            print_tape();
            mpc_ast_delete(r.output);
        } else {    
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);

    }
    

    mpc_cleanup(4, Symbol, Loop, Expr, Bf);

    return 0;
}