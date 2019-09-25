// shane desilva
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"

char *special_symbols[] = {"1", "2","3","+","-","*","/","8","=","<>","<","<=",">",">=","(",")",",",";",".",":="};
token *tokens[MAX_NUM_TOKENS];
int num_tokens;

void reserved(token *t);

void error(int code)
{
	switch (code)
	{
		case 0:
		printf( "ERROR #%d: File not found.\n", code);
		break;

		case 1:
		printf( "ERROR #%d: Use = instead of :=.\n", code);
		break;

		case 2:
		printf( "ERROR #%d: = must be followed by a number.\n", code);
		break;

		case 3:
		printf( "ERROR #%d: Identifier must be followed by =.\n", code);
		break;

		case 4:
		printf( "ERROR #%d: const, var, read, write must be followed by identifier.\n", code);
		break;

		case 5:
		printf( "ERROR #%d: Semicolon or comma missing.\n", code);
		break;

		case 6:
		printf( "ERROR #%d: Incorrect symbol after procedure declaration.\n", code);
		break;

		case 7:
		printf( "ERROR #%d: Statement expected.\n", code);
		break;

		case 8:
		printf( "ERROR #%d: Incorrect symbol after statement part in block.\n", code);
		break;

		case 9:
		printf( "ERROR #%d: Period expected.\n", code);
		break;

		case 10:
		printf( "ERROR #%d: Semicolon between statements missing.\n", code);
		break;

		case 11:
		printf( "ERROR #%d: Undeclared identifier.\n", code);
		break;

		case 12:
		printf( "ERROR #%d: Assignment to constant is not allowed.\n", code);
		break;

		case 13:
		printf( "ERROR #%d: Assignment operator expected.\n", code);
		break;

		case 14:
		printf( "ERROR #%d: call must be followed by an identifier.\n", code);
		break;

		case 15:
		printf( "ERROR #%d: Call of a constant or variable is meaningless.\n", code);
		break;

		case 16:
		printf( "ERROR #%d: then expected.\n", code);
		break;

		case 17:
		printf( "ERROR #%d: Semicolon or end expected.\n", code);
		break;

		case 18:
		printf( "ERROR #%d: do expected.\n", code);
		break;

		case 19:
		printf( "ERROR #%d: Incorrect symbol following statement.\n", code);
		break;

		case 20:
		printf( "ERROR #%d: Relational operator expected.\n", code);
		break;

		case 21:
		printf( "ERROR #%d: Expression must not contain a procedure identifier.\n", code);
		break;

		case 22:
		printf( "ERROR #%d: Right parenthesis missing.\n", code);
		break;

		case 23:
		printf( "ERROR #%d: The preceding factor cannot begin with this symbol.\n", code);
		break;

		case 24:
		printf( "ERROR #%d: An expression cannot begin with this symbol.\n", code);
		break;

		case 25:
		printf( "ERROR #%d: This number is too large.\n", code);
		break;

		case 26:
		printf( "ERROR #%d: This ident is too long.\n", code);
		break;

		case 27:
		printf( "ERROR #%d: variable doesn't start with letter.\n", code);
		break;

		case 28:
		printf( "ERROR #%d: invalid symbol.\n", code);
		break;

		case 29:
		printf( "ERROR #%d: Comment left unclosed.\n", code);
		break;

		case 30:
		printf( "ERROR #%d: No more than %d lines of code permitted.\n", code, MAX_CODE_LENGTH);
		break;

	}
	exit(EXIT_FAILURE);
}

void inputForScanner(char* filename)
{
	FILE *ifp = fopen(filename, "r");
	char cur, next;
	int index = 0, token_len;
	token *t;

	while ((cur = fgetc(ifp)) != EOF)
	{
		tokens[index] = malloc(sizeof(token));
		switch(cur)
		{
			case '\0':
			tokens[index]->type = nulsym;
			break;

			case '+':
			tokens[index]->type = plussym;
			break;

			case '-':
			tokens[index]->type = minussym;
			break;

			case '*':
			tokens[index]->type = multsym;
			break;

			case '/':
			next = fgetc(ifp);
			if (next == '*')
			{
				while ((cur = fgetc(ifp)) != EOF)
					if (cur == '*')
						if ((cur = fgetc(ifp)) == '/')
							break;

				if (cur == EOF)
					error(29);

				index--;
				break;
			}
			ungetc(next, ifp);
			tokens[index]->type = slashsym;
			break;

			case '=':
			tokens[index]->type = eqsym;
			break;

			case '<':
			next = fgetc(ifp);

			if (next == '>')
				tokens[index]->type = neqsym;

			else if (next == '=')
				tokens[index]->type = leqsym;

			else
			{
				tokens[index]->type = lessym;
				ungetc(next, ifp);
			}
			break;

			case '>':
			next = fgetc(ifp);

			if (next == '=')
				tokens[index]->type = geqsym;

			else
			{
				tokens[index]->type = gtrsym;
				ungetc(next, ifp);
			}
			break;

			case '(':
			tokens[index]->type = lparentsym;
			break;

			case ')':
			tokens[index]->type = rparentsym;
			break;

			case ',':
			tokens[index]->type = commasym;
			break;

			case ';':
			tokens[index]->type = semicolonsym;
			break;

			case '.':
			tokens[index]->type = periodsym;
			break;

			case ':':
			next = fgetc(ifp);
			if (next == '=')
				tokens[index]->type = becomessym;
			else
				error(28);

			break;

			default:
			if (isalpha(cur))
			{
				token_len = 1;
				tokens[index]->name[0] = cur;

				while ((cur = fgetc(ifp)) != EOF && (isalpha(cur) || isdigit(cur)))
				{
					if (++token_len > MAX_IDENT_LEN)
						error(26);

					tokens[index]->name[token_len-1] = cur;
				}

				tokens[index]->name[token_len] = '\0';
				reserved(tokens[index]);
				ungetc(cur, ifp);
			}

			else if (isdigit(cur))
			{
				token_len = 1;
				tokens[index]->val = cur - '0';
				tokens[index]->type = numbersym;
				while ((cur = fgetc(ifp)) != EOF && isdigit(cur))
				{
					if (++token_len > MAX_NUM_LEN)
						error(25);

					tokens[index]->val *= 10;
					tokens[index]->val += cur - '0';
				}

				if (isalpha(cur))
					error(27);

				ungetc(cur, ifp);
			}
			else if (isspace(cur))
				index--;

			else
				error(28);
		}
		index++;
	}
	num_tokens = index;
}

void reserved(token *t)
{
	if (strcmp(t->name, "const") == 0) t->type = constsym;
	else if (strcmp(t->name, "var") == 0) t->type = varsym;
	else if (strcmp(t->name, "begin") == 0) t->type = beginsym;
	else if (strcmp(t->name, "end") == 0) t->type = endsym;
	else if (strcmp(t->name, "if") == 0) t->type = ifsym;
	else if (strcmp(t->name, "then") == 0) t->type = thensym;
	else if (strcmp(t->name, "while") == 0) t->type = whilesym;
	else if (strcmp(t->name, "do") == 0) t->type = dosym;
	else if (strcmp(t->name, "read") == 0) t->type = readsym;
	else if (strcmp(t->name, "write") == 0) t->type = writesym;
	else if (strcmp(t->name, "odd") == 0) t->type = oddsym;
	else if (strcmp(t->name, "procedure") == 0) t->type = procsym;
	else if (strcmp(t->name, "call") == 0) t->type = callsym;
	else if (strcmp(t->name, "else") == 0) t->type = elsesym;
	else t->type = identsym;
}

void print_source(char *filename)
{
	FILE *ifp = fopen(filename, "r");
	char c;

	if (ifp == NULL)
		error(0);

	while ((c = fgetc(ifp)) != EOF)
		printf("%c", c);
}

void print_lexeme_table()
{
	int i;
	token *t;
	printf("lexeme     token type\n");

	for (i = 0; i < num_tokens; i++)
	{
		t = tokens[i];
		if (t->type == identsym || (t->type >= beginsym && t->type <= readsym))
			printf("%-11s", t->name);

		else if (t->type == numbersym)
			printf("%-11d", t->val);

		else
			printf("%-11s", special_symbols[t->type-1]);

		printf("%d\n", t->type);
	}
}

void print_lexeme_list()
{
	int i;
	token *t;

	for (i = 0; i < num_tokens; i++)
	{
		t = tokens[i];
		printf("%d ", t->type);

		if (t->type == identsym)
			printf("%s ", t->name);

		else if (t->type == numbersym)
			printf("%d ", t->val);
	}
	printf("\n");
}
