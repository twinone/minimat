#include "minimat.h"

size_t tokenize(char *input, char *delim, char **array) {      
	size_t i = 0;      
	char *ptr = strtok(input, delim);
	while(ptr) {
		array[i] = malloc(strlen(ptr) + 1); // Null byte at end
		if (array[i]) 
			strcpy(array[i], ptr);
		i++;
		ptr = strtok(NULL, delim);       
	}
	// add nullpointer to detect end of tokens
	array[i]=NULL;
	return i;
}
void freeTokens(char **array) {
	if (array) {
		size_t i;
		for(i = 0;array[i];i++) {
			free(array[i]);
			array[i]=NULL;
		}
	}
}
char *getTypeString(int type) {
	switch (type){
		case TYPE_DOUBLE:
		return "number";
		case TYPE_MATRIX:
		return "matrix";
		default:
		return "undefined";
	}
}

int getTokType(char *tok) {
	if (!tok) return TYPE_UNKNOWN;
	if (strcmp(tok, CMD_TRANSPOSE) == 0 ||
		strcmp(tok, CMD_INVERSE) == 0 ||
		strcmp(tok, CMD_COFACTOR) == 0 ||
		strcmp(tok, CMD_ADJOINT) == 0 ||
		strcmp(tok, CMD_READ) == 0 ||
		strcmp(tok, "{") == 0)
	{
		return TYPE_MATRIX;
	} else if (
		strcmp(tok, CMD_DET) == 0 ||
		strcmp(tok, CMD_SQRT) == 0) 
	{
		return TYPE_DOUBLE;
	} 
	else if (strlen(tok)==1) 
	{
		switch (tok[0]){
			case 'W': case 'X': case 'Y': case 'Z':
			return TYPE_MATRIX;
			case 'a': case 'b': case 'c': case 'd':
			return TYPE_DOUBLE;
			case '+': case '-': case '*': case '/': case '%': case '&':
			return TYPE_OP;
		}
	} 
	
	return validateDouble(tok) ? TYPE_DOUBLE : TYPE_UNKNOWN;
}

int getReturnType(int v1, char op, int v2) {
	if (v1==TYPE_UNKNOWN||v2==TYPE_UNKNOWN) 
		return TYPE_UNKNOWN;
	if (op == '+' || op == '-') {
		// Sum and substract if types are equal
		return (v1==v2?v1:TYPE_UNKNOWN);
	} else if (op == '*') {
		if(v1==TYPE_MATRIX||v2==TYPE_MATRIX) 
			return TYPE_MATRIX;
		else 
			return TYPE_DOUBLE;
	} else if (op == '/') {
		if (v2==TYPE_MATRIX)
			return TYPE_UNKNOWN;
		else 
			return v1;
	} else if (op == '%') {
		if (v1 != TYPE_DOUBLE || v2 != TYPE_DOUBLE) {
			return TYPE_UNKNOWN;
		} else {
			return TYPE_DOUBLE;
		}
	} else {
		return TYPE_UNKNOWN;
	}
}