#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "lex.h"
/*
Something like Python
>> y = 2
>> z = 2
>> x = 3*y + 4/(2*z)

*/

/*
the only type: integer
everything is an expression
  statement   := END | expr END
  expr        := term expr_tail
  expr_tail   := ADDSUB term expr_tail | NIL
  term        := factor term_tail
  term_tail := MULDIV factor term_tail | NIL
  factor      := INT | ADDSUB INT | ADDSUB ID | ID ASSIGN expr | ID | LPAREN expr RPAREN
*/

#define TBLSIZE 65535
typedef struct {
	char name[MAXLEN];
	int val;
} Symbol;
Symbol table[TBLSIZE];
int sbcount = 0;

typedef struct _Node {
	char lexeme[MAXLEN];
	TokenSet token;
	int val;
	struct _Node *left, *right;
} BTNode;

void statement(void);
BTNode* assign_expr(void);
BTNode* or_expr(void);
BTNode* xor_expr(void);
BTNode* and_expr(void);
BTNode* addsub_expr(void);
BTNode* muldiv_expr(void);
BTNode* factor(void);
int getval(void);
int setval(char*, int);

typedef enum {MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NAN} ErrorType;
void error(ErrorType errorNum);

/* create a node without any child */
BTNode* makeNode(TokenSet tok, const char *lexe)
{
	BTNode *node = (BTNode*) malloc(sizeof(BTNode));
	strcpy(node->lexeme, lexe);
	node->token= tok;
	node->val = 0;
	node->left = NULL;
	node->right = NULL;
	return node;
}

/* clean a tree */
void freeTree(BTNode *root)
{
	if (root!=NULL) {
		freeTree(root->left);
		freeTree(root->right);
		free(root);
	}
}

/* print a tree by pre-order */
void printPrefix(BTNode *root)
{
	if (root != NULL) {
		printf("%s ", root->lexeme);
		printPrefix(root->left);
		printPrefix(root->right);
	}
}

/* traverse the syntax tree by pre-order
   and evaluate the underlying expression */
int evaluateTree(BTNode *root)
{
	int retval = 0, lv, rv;
printf("tree lex: %s\n", root->lexeme);
	if (root != NULL) {
		switch (root->token) {
		case ID:
		case INT:
			retval = root->val;
			break;
		case ASSIGN:
		case ADDSUB:
		case MULDIV:
		case AND:
		case OR:
		case XOR:
		case INCDEC:
		case POSNEG:
			lv = evaluateTree(root->left);
			rv = evaluateTree(root->right);
			if (strcmp(root->lexeme, "+") == 0) {
				if (root->token == POSNEG)
					retval = lv * 1;
				else
					retval = lv + rv;
			} else if (strcmp(root->lexeme, "-") == 0) {
				if (root->token == POSNEG)
					retval = lv * -1;
				else
					retval = lv - rv;
			} else if (strcmp(root->lexeme, "++") == 0) {
				retval = lv + 1;
				setval(root->left->lexeme, retval);
			} else if (strcmp(root->lexeme, "--") == 0) {
				retval = lv - 1;
				setval(root->left->lexeme, retval);
			} else if (strcmp(root->lexeme, "*") == 0)
				retval = lv * rv;
			else if (strcmp(root->lexeme, "/") == 0) {
				if (rv==0)
					error(NAN);
				else
					retval = lv / rv;
			} else if (strcmp(root->lexeme, "&") == 0) {
				retval = lv & rv;
			} else if (strcmp(root->lexeme, "^") == 0) {
				retval = lv ^ rv;
			} else if (strcmp(root->lexeme, "|") == 0) {
				retval = lv | rv;
			} else if (strcmp(root->lexeme, "=") == 0)
				retval = setval(root->left->lexeme, rv);
			break;
		default:
			retval = 0;
		}
	}
	return retval;
}

int getval(void)
{
	int i, retval, found;

	if (match(INT)) {
		retval = atoi(getLexeme());
	} else if (match(ID)) {
		i = 0;
		found = 0;
		retval = 0;
		while (i<sbcount && !found) {
			if (strcmp(getLexeme(), table[i].name)==0) {
				retval = table[i].val;
				found = 1;
				break;
			} else {
				i++;
			}
		}
		if (!found) {
			if (sbcount < TBLSIZE) {
				strcpy(table[sbcount].name, getLexeme());
				table[sbcount].val = 0;
				sbcount++;
			} else {
				error(RUNOUT);
			}
		}
	}
	return retval;
}

int setval(char *str, int val)
{
	int i, retval = 0;
	i = 0;
	while (i<sbcount) {
		if (strcmp(str, table[i].name)==0) {
			table[i].val = val;
			retval = val;
			break;
		} else {
			i++;
		}
	}
	return retval;
}

// assign_expr             := ID ASSIGN assign_expr | or_expr
BTNode* assign_expr(void)
{
    	return or_expr();
}

// or_expr             := xor_expr or_expr_tail
// or_expr_tail        := OR xor_expr or_expr_tail | NiL
BTNode* or_expr(void)
{
        BTNode *retp, *left;
        retp = left = xor_expr();
        while (match(OR)) { // tail recursion => while
                retp = makeNode(OR, getLexeme());
                advance();
                retp->right = xor_expr();
                retp->left = left;
                left = retp;
        }
        return retp;
}

// xor_expr            := and_expr xor_expr_tail
// xor_expr_tail       := XOR and_expr xor_expr_tail | NiL
BTNode* xor_expr(void)
{
        BTNode *retp, *left;
        retp = left = and_expr();
        while (match(XOR)) { // tail recursion => while
                retp = makeNode(XOR, getLexeme());
                advance();
                retp->right = and_expr();
                retp->left = left;
                left = retp;
        }
        return retp;
}

// and_expr            := addsub_expr and_expr_tail
// and_expr_tail       := AND addsub_expr and_expr_tail | NiL
BTNode* and_expr(void)
{
        BTNode *retp, *left;
        retp = left = addsub_expr();
        while (match(AND)) { // tail recursion => while
                retp = makeNode(AND, getLexeme());
                advance();
                retp->right = addsub_expr();
                retp->left = left;
                left = retp;
        }
        return retp;
}

//  addsub_expr         := muldiv_expr addsub_expr_tail
//  addsub_expr_tail    := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode* addsub_expr(void)
{
	BTNode *retp, *left;
	retp = left = muldiv_expr();
	while (match(ADDSUB)) { // tail recursion => while
		retp = makeNode(ADDSUB, getLexeme());
		advance();
		retp->right = muldiv_expr();
		retp->left = left;
		left = retp;
	}
	return retp;
}

//  muldiv_expr         := unary_expr muldiv_expr_tail
//  muldiv_expr_tail    := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode* muldiv_expr(void)
{
	BTNode *retp, *left;
	retp = left = factor();
	while (match(MULDIV)) { // tail recursion => while
		retp = makeNode(MULDIV, getLexeme());
		advance();
		retp->right = factor();
		retp->left = left;
		left = retp;
	}
	return retp;
}

BTNode* factor(void)
{
        BTNode* retp = NULL;

	if (match(INT)) {
		retp =  makeNode(INT, getLexeme());
		retp->val = getval();
		advance();
	} else if (match(ID)) {
	        BTNode* left = makeNode(ID, getLexeme());
        	left->val = getval();
        	advance();
	        if (match(ASSIGN)) {
	            	retp = makeNode(ASSIGN, getLexeme());
		        advance();
        		retp->right = assign_expr();
		        retp->left = left;
        	} else 
            		retp = left;
	} else if (match(INCDEC)) {
		retp = makeNode(INCDEC, getLexeme());
		advance();
		if (match(ID)) {
			BTNode* left = makeNode(ID, getLexeme());
	                left->val = getval();
            		retp->left = left;
                	advance();
		} else {
			error(NOTNUMID);
		}
	} else if (match(POSNEG)) {
		retp =  makeNode(POSNEG, getLexeme());
                advance();
                if (match(ID)) {
                        BTNode* left = makeNode(ID, getLexeme());
                        left->val = getval();
                        retp->left = left;
                        advance();
		} else if (match(INT)) {
                        BTNode* left = makeNode(INT, getLexeme());
                        left->val = getval();
                        retp->left = left;
                        advance();
                } else {
                        error(NOTNUMID);
                }
	} else if (match(LPAREN)) {
		advance();
		retp = assign_expr();
		if (match(RPAREN)) {
			advance();
		} else {
			error(MISPAREN);
		}
	} else {
		error(NOTNUMID);
	}
	return retp;
}

void error(ErrorType errorNum)
{
	switch (errorNum) {
	case MISPAREN:
		fprintf(stderr, "Mismatched parenthesis\n");
		break;
	case NOTNUMID:
		fprintf(stderr, "Number or identifier expected\n");
		break;
	case NOTFOUND:
		fprintf(stderr, "%s not defined\n", getLexeme());
		break;
	case RUNOUT:
		fprintf(stderr, "Out of memory\n");
		break;
	case NAN:
		fprintf(stderr, "Not a number\n");
	}
	exit(0);
}

void statement(void)
{
	BTNode* retp;

	if (match(END)) {
		printf(">> ");
		advance();
	} else {
		retp = assign_expr();
		if (match(END)) {
			printf("%d\n", evaluateTree(retp));
			printPrefix(retp);
			printf("\n");
			freeTree(retp);

			printf(">> ");
			advance();
		}
	}
}

int main()
{
	printf(">> ");
	while (1) {
		statement();
	}
	return 0;
}
