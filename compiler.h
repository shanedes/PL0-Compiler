#define MAX_STACK_HEIGHT 23
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3
#define MAX_NUM_TOKENS 5000
#define MAX_IDENT_LEN 11
#define MAX_NUM_LEN 5
#define MAX_SYMBOL_TABLE_SIZE 5000

typedef struct instruction
{
	int op; // opcode
	int r;  // R
	int l;  // L
	int m;  // M
} instruction;

typedef struct token
{
	int type;
	char name[MAX_IDENT_LEN+1];
	int val;
} token;

typedef enum
{
	nulsym = 1, identsym, numbersym, plussym, minussym, multsym,
	slashsym, oddsym, eqsym, neqsym, lessym, leqsym, gtrsym, geqsym,
	lparentsym, rparentsym, commasym, semicolonsym, periodsym,
	becomessym, beginsym, endsym, ifsym, elsesym, thensym, whilesym, dosym,
	callsym, constsym, varsym, procsym, writesym, readsym
} token_type;

typedef enum
{
	fileerr = 0, identerr, longnum, longident, invaliderr,commenterr
} error_code;

extern int lines;
extern token *tokens[MAX_NUM_TOKENS];
extern instruction code[MAX_CODE_LENGTH];

int vm();
void error(int code);
void inputForScanner();
void print_source();
void print_lexeme_table();
void print_lexeme_list();
void parse();
