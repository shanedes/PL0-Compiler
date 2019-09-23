// shane desilva
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"


typedef struct
{
	int kind; 		// const = 1, var = 2, proc = 3
	char name[10];	// name up to 11 chars
	int val; 		// number (ASCII value)
	int level; 		// L level
	int addr; 		// M address
	int mark;		// to indicate that code has been generated already for a block.
} symbol;

symbol *symbol_table[MAX_SYMBOL_TABLE_SIZE];

token *toke;
int token_index = 0, tablesize = 1;
int currReg = 0;
int ad = 4;
int cx = 0;
int lex = 0;
int factors = 0;

void program();
void block();
void condition();
void factor();
void constDec();
void varDec();
void procDec();
void statement();
void expression();
void term();

void insertSymbol(char *name, int kind, int l, int add, int v)
{
	symbol s;
	strcpy(s.name, name);
	s.kind = kind;
	s.addr = add;
	s.mark = 0;
	s.level = l;
	s.val = v;
	symbol_table[tablesize] = malloc(sizeof(symbol));
	*symbol_table[tablesize] = s;
	tablesize++;
}

int findSymbol(char *name)
{
	int i;
	for (i = tablesize-1; i > 0; i--)
	{
		if (!symbol_table[i]->mark && (strcmp(name, symbol_table[i]->name) == 0))
			break;
	}
	return i;
}

void markAll(int l)
{
	int i;
	for (i = tablesize-1; i > 0; i--)
	{
		if (symbol_table[i]->level == l)
			symbol_table[i]->mark = 1;
	}
}

void emit(int op, int r, int l, int m)
{
	if (cx > MAX_CODE_LENGTH)
		error(30);
	else
	{
		code[cx].op = op;
		code[cx].r = r;
		code[cx].l = l;
		code[cx].m = m;
		cx++;
	}
}


void getToken()
{
		toke = tokens[token_index++];
}

void parse(char *filename)
{
	inputForScanner(filename);

	program();
	printf("No errors, program is syntactically correct\n\n");
	lines = cx;
}

void program()
{
	getToken();
	block();

	if (toke->type != periodsym) error(9);

	emit(11, 0, 0, 3);
}

void block()
{
	currReg = 0;
	ad = 4;
	int jmpI = cx;
	emit(7, 0, 0, jmpI+1);
	if (toke->type == constsym)
		constDec();

	if (toke->type == varsym)
		varDec();

	while (toke->type == procsym)
	{
		procDec(jmpI);
		code[jmpI].m = cx;
	}
	emit(6, 0, 0, ad);
	statement();
}

void constDec()
{
	char *s;
	do
	{
		getToken();
		if (toke->type != identsym) error(4);
		s = toke->name;
		getToken();
		if(toke->type == becomessym) error(1);
		if (toke->type != eqsym) error(3);
		getToken();
		if (toke->type != numbersym) error(2);
		insertSymbol(s, 1, lex, 0, toke->val);
		getToken();
	} while (toke->type == commasym);

	if (toke->type != semicolonsym)
		error(5);

	getToken();
}

void varDec()
{
	do
	{
		getToken();
		if (toke->type != identsym) error(4);
		insertSymbol(toke->name, 2, lex, ad, 0);
		ad++;
		getToken();
	} while (toke->type == commasym);

	if (toke->type != semicolonsym)
		error(5);

	getToken();
}

void procDec(int jmpI)
{
	getToken();
	if (toke->type != identsym) error(4);
	insertSymbol(toke->name, 3, lex, jmpI + 1, -1);
	lex++;
	getToken();
	if (toke->type != semicolonsym) error(6);
	getToken();
	block();
	if (toke->type != semicolonsym) error(6);
	emit(2, 0, 0 , 0);
	markAll(lex);
	lex--;
	getToken();
}

void statement()
{
	int cx1, cx2, pos;
	//currReg = 0;
	if (toke->type == identsym)
	{
		pos = findSymbol(toke->name);
		if (pos == 0) error(11); // undeclared var
		if (symbol_table[pos]->kind != 2) error(12);
		getToken();
		if (toke->type != becomessym) error(13);
		getToken();
		expression();
		emit(4, currReg, lex-symbol_table[pos]->level, symbol_table[pos]->addr);
		currReg--;
	}
	else if (toke->type == callsym)
	{
		getToken();
		if (toke->type != identsym) error(14);
		pos = findSymbol(toke->name);
		if (pos == 0) error(11);
		if (symbol_table[pos]->kind != 3) error(15);
		emit(5, 0, lex-symbol_table[pos]->level, symbol_table[pos]->addr);
		getToken();
	}
	else if (toke->type ==  beginsym)
	{
		getToken();
		statement();
		while (toke->type == semicolonsym)
		{
			getToken();
			statement();
		}
		if (toke->type != endsym) error(17);
		getToken();
	}
	else if (toke->type == ifsym)
	{
		getToken();
		condition();
		if (toke->type != thensym) error(16);
		int cx1 = cx;
		emit(8, currReg, 0, 0);
		getToken();
		statement();
		token* t;
		if (toke->type == semicolonsym)
		{
			t = toke;
			getToken();
		}
		if (toke->type != elsesym)
		{
			token_index--;
			toke = t;
			code[cx1].m = cx;
		}
		else
		{
			cx2 = cx;
			emit(7, 0, 0, 0);
			code[cx1].m = cx;
			getToken();
			statement();
			code[cx2].m = cx;
		}
	}
	else if (toke->type == whilesym)
	{
		cx1 = cx;
		getToken();
		condition();
		emit(8, currReg, 0, 0);
		cx2 = cx;
		if (toke->type != dosym) error(18);
		getToken();
		statement();
		emit(7, 0, 0, cx1);
		code[cx2-1].m = cx;
	}
	else if (toke->type == readsym)
	{
		getToken();
		if (toke->type != identsym) error(4);
		pos = findSymbol(toke->name);
		if (pos == 0) error(11);
		if (symbol_table[pos]->kind != 2) error(12);
		emit(10, currReg, 0, 0);
		emit(4, currReg, lex-symbol_table[pos]->level, symbol_table[pos]->addr);
		getToken();
	}
	else if (toke->type == writesym)
	{
		getToken();
		if (toke->type != identsym) error(4);
		pos = findSymbol(toke->name);
		if (pos == 0) error(11);
		if (symbol_table[pos]->kind == 2)
		{
			emit(3, currReg, lex-symbol_table[pos]->level, symbol_table[pos]->addr);
			emit(9, currReg, 0, 0);
		}
		else if (symbol_table[pos]->kind == 1)
		{
			emit(1, currReg, 0, symbol_table[pos]->val);
			emit(10, currReg, 0, 0);
		}
		getToken();
	}
}

void condition()
{
	int t;
	if (toke->type == oddsym)
	{
		getToken();
		expression();
		emit(17, 0, 0, 0);
	}
	else
	{
		expression();
		t = toke->type;
		if ( t < eqsym || t > geqsym) error(20);
		getToken();
		expression();
		emit(t-eqsym+19, currReg, currReg-1, currReg);
	}
}

void expression()
{
	int addop;
	if (toke->type == plussym || toke->type == minussym)
	{
		addop = toke->type;
		getToken();
		term();
		if (addop == minussym)
			emit(12, currReg, currReg, 0);
	}
	else
		term();
	while (toke->type == plussym || toke->type == minussym)
	{
		addop = toke->type;
		getToken();
		term();
		currReg--;
		if (addop == plussym)
			emit(13, currReg, currReg, currReg + 1);
		else
			emit(14, currReg, currReg, currReg + 1);
	}
}

void term()
{
	int mulop;
	factor();
	while (toke->type == multsym || toke->type == slashsym)
	{
		mulop = toke->type;
		getToken();
		factor();
		currReg--;
		if (mulop == multsym)
			emit(15, currReg, currReg, currReg + 1);
		else
			emit(16, currReg, currReg, currReg + 1);
	}
}

void factor()
{
	int pos;
	if (toke->type == identsym)
	{
		pos = findSymbol(toke->name);
		if (pos == 0) error(11);

			currReg++;

		if (symbol_table[pos]->kind == 1)
			emit(1, currReg, 0, symbol_table[pos]->val);
		else if (symbol_table[pos]->kind == 2)
			emit(3, currReg, lex-symbol_table[pos]->level, symbol_table[pos]->addr);
		getToken();
	}
	else if (toke->type == numbersym)
	{
			currReg++;

		emit(1, currReg, 0, toke->val);
		getToken();
	}
	else if (toke->type == lparentsym)
	{

		getToken();
		expression();
		if (toke->type != rparentsym ) error(22);
		getToken();
	}
	else error(24);
}
