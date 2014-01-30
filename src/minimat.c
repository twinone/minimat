#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h> // size_t
#include "minimat.h"

static char input[MAX_CMD_LEN];
static char *cmd[MAX_TOKENS] = { NULL };

static double A = 0; static double *a = &A;
static double B = 0; static double *b = &B;
static double C = 0; static double *c = &C;
static double D = 0; static double *d = &D;

static TMatrix *W = NULL;
static TMatrix *X = NULL;
static TMatrix *Y = NULL;
static TMatrix *Z = NULL;

int main(int argc, char **argv) {
	printf("Author: Twinone (Luuk Willemsen)\n\n");

	// usage
	if (argc > 1) {
		printf("Welcome to minimat\n");
		printf("This is a university project, it's not maintained.\n");
		printf("You can use it, modify it, sell it, rape it, or do anything to it you want.\n\n");

		printf("Basic usage:\n");
		printf("(var =) value1 (op value2)\n");
		printf("Where var must be a variable and value1 and value2 can be either a variable or literal.");
		printf("Variables: a,b,c,d for numbers; W,X,Y,Z for matrices\n");
		printf("Numeric literals: 1, 2.3, -7.13\n");
		printf("Matrix literals: { [ 1 2 ] [ 1 1 ] }\n");
		printf("op can be either of '+', '-', '*', '/', '%%' or '&'\n");
		printf("Where +-*/ are basic operations, %% is modulus and & is concatenate\n");


		printf("Functions:\n");
		printf("%s ( M )\n\t Returns the inverse matrix of M\n", CMD_INVERSE);
		printf("%s ( M )\n\t Transposed matrix of M\n", CMD_TRANSPOSE);
		printf("%s ( M )\n\t Determinant of M\n", CMD_DET);
		printf("%s ( M )\n\t Returns the adjoint matrix of M\n", CMD_ADJOINT);
		printf("%s ( M )\n\t Returns the range of M\n", CMD_RANGE);
		printf("%s ( M )\n\t Returns the cofactor matrix of M\n", CMD_COFACTOR);
		printf("%s ( n|M )\n\t Returns the square root of n or the modulus of vector represented by M\n", CMD_SQRT);
		printf("%s ( f )\n\t Reads file f (must be a matrix)\n", CMD_READ);
		printf("%s ( M f )\n\t Writes matrix M to file f\n", CMD_WRITE);
		printf("%s\n\t Exits the program\n\n", CMD_EXIT);

		printf("Examples:\n");
		printf("a = 9 * 3.55\n");
		printf("W = { [ 2 3 ] [ 1 1 ] } * a\n");
		printf("b = det ( W ) * det ( W )\n");
		printf("Z = read ( m.txt ) * W\n");
		printf("c = 3 * det ( W )\n");
		printf("\n");
	}

	// main loop
	do {
		printf(prompt);
		fgets(input, sizeof(input), stdin);

		size_t numTok = tokenize(input, DELIM, cmd); 

		if (numTok == 0) {
			// ignore empty command
			continue;
		}

		// exit
		if (strcmp(cmd[0], CMD_EXIT) == 0) {
			// get out of loop and end program
			break;
		}
		// write to file
		else if (strcmp(cmd[0], CMD_WRITE) == 0) {
			size_t currentTokenPointer = 0;
			size_t *ctp = &currentTokenPointer;
			printMatrixToFile(cmd, ctp);
		}

		// assignment
		else  {
			int resultType;
			int isAssignment = numTok > 1 && strcmp(cmd[1], "=") == 0;
			size_t currentTokenPointer = isAssignment?2:0;
			size_t *ctp = &currentTokenPointer;

			if(isAssignment && numTok == 2) {
				printf("Error sintactico: asignacion vacia\n"); 
				continue; 
			}
			// calculate
			void *result = eval(cmd, ctp, &resultType);

			if (!result || !resultType) {
				continue;
			}
			if (isAssignment) {
				int lval = getTokType(cmd[0]);
				void *lvalPtr = getVarPointer(cmd[0]);
				if (!lvalPtr) { 
					printf("Error: %s is not a variable\n", cmd[0]);
					continue;
				}
				if (lval != resultType) {
					printf("Error: Can't assign %s to %s\n", getTypeString(resultType), getTypeString(lval));
					continue;
				} 
				if (resultType == TYPE_DOUBLE){
					*(double*)lvalPtr = *(double*)result;
				} else {
					*(TMatrix**)lvalPtr = result;
				}
			} else {
				// just print variable print
				if (resultType==TYPE_DOUBLE) {
					// Avoid ugly -0
					if (*(double*)result == -0) *(double*)result = 0;
					printf("%10.3f\n", *(double*)result);
				} else {
					printMatrix((TMatrix*)result);
				}
			}
		}
		freeAllMatrices();
		freeTokens(cmd);
	} while (1);

	return 0;
}


/* 
* Evaluates an expression in the form 'v1 (operator v2)
*/
void *eval(char **cmd, size_t *ctp, int *resultType) {
	if (cmd == NULL || cmd[*ctp]== NULL) return NULL;
	int t1 = TYPE_UNKNOWN;
	int t2 = TYPE_UNKNOWN;
	char op = TYPE_UNKNOWN;
	void *p1 = NULL; 
	void *p2 = NULL;

	t1 = getTokType(cmd[*ctp]);
	if (!t1 || t1 == TYPE_OP) {
		printf("%s isn't a number, matrix or variable\n", cmd[*ctp]);
		return NULL;
	}
	if (t1 == TYPE_DOUBLE) {
		p1 = (void*)readDouble(cmd,ctp);
	} else {
		p1 = (void*)readMatrix(cmd,ctp);
	}
	if (!p1) { return NULL; }

	op = tryReadOperator(cmd, ctp);
	if (op) {
		t2 = getTokType(cmd[*ctp]);
		if (!t2 || t2 == TYPE_OP) {
			if (cmd[*ctp]) {
				printf("%s isn't a number, matrix or variable\n", cmd[*ctp]);
			} else {
				printf("Expected value after %c\n", op);
			}
			return NULL;
		}
		if (t2 == TYPE_DOUBLE) {
			p2 = (void*)readDouble(cmd,ctp);
		} else {
			p2 = (void*)readMatrix(cmd,ctp);
		}
		if (!p2) { return NULL; }
		// At this point it's verified that both p1, p2 and op are not null
		// and t1 and t2 are not TYPE_UNKNOWN
		void *ret = NULL;
		switch (op) {
			case '+':
			*resultType = (t1 == t2 ? t1 : TYPE_UNKNOWN);
			ret = logicAdd(t1, t2, p1, p2);
			break;
			case '-':
			*resultType = (t1 == t2 ? t1 : TYPE_UNKNOWN);
			ret = logicSub(t1, t2, p1, p2);
			break;
			case '*':
			*resultType = (t1 == TYPE_MATRIX||t2 == TYPE_MATRIX ? TYPE_MATRIX : TYPE_DOUBLE);
			ret = logicMult(t1, t2, p1, p2);
			break;
			case '/':
			*resultType = (t2 == TYPE_MATRIX ? TYPE_UNKNOWN : t1);
			ret = logicDivide(t1, t2, p1, p2);
			break;
			case '%':
			*resultType = TYPE_DOUBLE;
			ret = logicModulus(t1, t2, p1, p2);
			break;
			case '&':
			*resultType = TYPE_MATRIX;
			ret = logicCat(t1, t2, p1, p2);
			break;
		}
		*resultType = (ret) ? *resultType : TYPE_UNKNOWN;
		if (!expect(NULL, cmd, ctp)) {
			return NULL;
		}
		return ret;
	} else {
		if (!expect(NULL, cmd, ctp)) {
			return NULL;
		}
		// No op, so result type = t1
		*resultType = t1;
		return p1;
	}
	return NULL;
}

void *getVarPointer(char *tok) {
	if (!tok || strlen(tok)!=1) return NULL;
	switch (tok[0]) {
		case 'a': return &a; case 'b': return &b;
		case 'c': return &c; case 'd': return &d;
		case 'W': return &W; case 'X': return &X;
		case 'Y': return &Y; case 'Z': return &Z;
		default: return NULL;
	}
}


