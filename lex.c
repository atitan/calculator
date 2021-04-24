#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lex.h"
static TokenSet getToken(void);
static TokenSet lookahead = UNKNOWN;
static TokenSet lastToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    int i;
    char c;
    TokenSet currentToken = UNKNOWN;

    while ( (c = fgetc(stdin)) == ' ' || c== '\t' );  // ©¿²¤ªÅ¥Õ¦r¤¸

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i<MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        currentToken = INT;
    } else if (c == '+' || c == '-') {
        lexeme[0] = c;
        c = fgetc(stdin);
        if (lexeme[0] == c) {
		lexeme[1] = c;
		lexeme[2] = '\0';
		currentToken = INCDEC;
	} else {
		if (
			(lastToken == UNKNOWN || lastToken == ADDSUB || lastToken == MULDIV || lastToken == AND || lastToken == XOR || lastToken == OR || lastToken == ASSIGN || lastToken == LPAREN)
			&& (isalpha(c) || isdigit(c) || c == '_')
		) {
	        	currentToken = POSNEG;
		} else {
		        currentToken = ADDSUB;
		}
		ungetc(c, stdin);
	        lexeme[1] = '\0';
	}
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        currentToken = MULDIV;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        currentToken = END;
    } else if (c == '&') {
        strcpy(lexeme, "&");
        currentToken = AND;
    } else if (c == '^') {
        strcpy(lexeme, "^");
        currentToken = XOR;
    } else if (c == '|') {
        strcpy(lexeme, "|");
        currentToken = OR;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        currentToken = ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        currentToken = LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        currentToken = RPAREN;
    } else if (isalpha(c) || c == '_') {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isalpha(c) || isdigit(c) || c == '_') {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        currentToken = ID;
    }

if (currentToken == UNKNOWN)
	printf("unknown\n");
else if(currentToken == INT)
	printf("int\n");
else if(currentToken == INCDEC)
	printf("incdec\n");
else if(currentToken == ADDSUB)
	printf("addsub\n");
else if(currentToken == POSNEG)
	printf("posneg\n");
else if(currentToken == MULDIV)
	printf("muldiv\n");
else if(currentToken == OR)
	printf("or\n");
else if(currentToken == XOR)
	printf("xor\n");
else if(currentToken == AND)
	printf("and\n");
else if(currentToken == END)
	printf("end\n");
else if(currentToken == ASSIGN)
	printf("assign\n");
else if(currentToken == ID)
	printf("id\n");
else if(currentToken == LPAREN)
	printf("lparen\n");
else if(currentToken == RPAREN)
	printf("rparen\n");

    lastToken = currentToken;
    return currentToken;
}

void advance(void)
{
    lookahead = getToken();
}

int match(TokenSet token)
{
    if (lookahead == UNKNOWN) advance();
    return token == lookahead;
}

char* getLexeme(void)
{
    return lexeme;
}

