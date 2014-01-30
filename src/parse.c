#include <math.h>
#include "minimat.h"

char tryReadOperator(char **cmd, size_t *ctp) {
	if (!cmd || !cmd[*ctp]) { return 0; }
	char op = cmd[*ctp][0];
	if (strlen(cmd[*ctp]) == 1 && getTokType(cmd[*ctp]) == TYPE_OP) {
		// consume operator
		(*ctp)++;
		return op;
	}
	return 0;
}
double *readDouble(char **cmd, size_t *ctp) {
	if (!cmd || !cmd[*ctp]) { return NULL; }
	
	if (strcmp(cmd[*ctp],CMD_DET) == 0) {
		// readDet consumes tokens
		return readDet(cmd, ctp);
	} else if (strcmp(cmd[*ctp], CMD_SQRT) == 0) {
		return readSqrt(cmd, ctp);
	} else if (validateDouble(cmd[*ctp])) {
		double *s = malloc(sizeof(double *));
		*s = strtod(cmd[*ctp],NULL);
		// consume this double
		(*ctp)++;
		return s;
	} else {
		double *s = malloc(sizeof(double *));
		void *ptr = getVarPointer(cmd[*ctp]);
		// consume this double
		if (ptr!=NULL) {
			(*ctp)++;
		}
		return ptr;
	}
	return NULL;
}

double *readDet(char **cmd, size_t *ctp) {
	if (!cmd || !cmd[*ctp]) { return NULL; }
	if (!consume(CMD_DET, cmd, ctp) || !consume("(",cmd, ctp)) { 
		return NULL; 
	}
	TMatrix *m = readMatrix(cmd, ctp);
	// ctp is now right after matrix
	if (!m) { return NULL; }
	if (!consume(")",cmd, ctp)) { 
		return NULL; 
	}
	return mDeterminant(m);
}

TMatrix *readMatrix(char **cmd, size_t *ctp) {
	if (!cmd || !cmd[*ctp]) {
		printErrorNearToken("{ [ 1 ] }", cmd, *ctp);
		return NULL;
	}
	// W X Y Z
	if (strlen(cmd[*ctp]) == 1 && cmd[*ctp][0] >= 'W' && cmd[*ctp][0] <= 'Z') {
		TMatrix **m = (TMatrix**)getVarPointer(cmd[*ctp]);
		if (!m || *m == NULL) { 
			printf("Invalid reference to not-assigned matrix %c\n", cmd[*ctp][0]);
			return NULL;
		}
		(*ctp)++;
		return *m;
	} 
	else if (strcmp(cmd[*ctp],CMD_READ) == 0) {
		return readMatrixFromFile(cmd, ctp);
	}
	else if (strcmp(cmd[*ctp],CMD_READ) == 0) {
		return readMatrixFromFile(cmd, ctp);
	}
	else if (strcmp(cmd[*ctp],CMD_INVERSE) == 0) {
		return readInverse(cmd, ctp);
	}
	else if (strcmp(cmd[*ctp],CMD_TRANSPOSE) == 0) {
		return readTranspose(cmd, ctp);
	}
	else if (strcmp(cmd[*ctp],CMD_ADJOINT) == 0) {
		return readAdjoint(cmd, ctp);
	} 
	else if (strcmp(cmd[*ctp],CMD_COFACTOR) == 0) {
		return readCofactor(cmd, ctp);
	} 
	else {
		return readMatrixLiteral(cmd, ctp);
	}
}

TMatrix *readMatrixLiteral(char **cmd, size_t *ctp) {
	int r, c;
	if(!validateMatrix(cmd, ctp, &r, &c)) { return NULL; }
	
	TMatrix *s = allocMatrix(r, c);
	double **m = s->matrix;
	int i;
	int j;
	// consume {} (already checked by validateMatrix, so no need to use consume)
	(*ctp)++; 
	for (i = 0; i < r; i++) {
			 // consume[] 
		(*ctp)++;
		for (j = 0; j < c; j++) {
			// consume element
			m[i][j] = *readDouble(cmd, ctp); 
		}
		 // consume []
		(*ctp)++;
	}
	// consume {}
	(*ctp)++; 
	return s;
}

TMatrix *readMatrixFromFile(char **cmd, size_t *ctp) {
	if (!cmd||!cmd[*ctp]) return NULL;
	// only non-space filenames are supported due to the way the program handles parameters
	(*ctp)++; //  consume read
	if (!consume("(",cmd, ctp)) { 
		return NULL; 
	}

	// consume filename
	char *filename = cmd[*ctp];
	if (!consumeFilename(cmd, ctp)) return NULL;

	if (!consume(")",cmd, ctp)) {
		return NULL;
	}
	int r = 0;
	int c = 0;
	char *tokens[256];

	FILE *fp;
	char line[READ_FILE_LINE_LENGTH + 1];
	size_t len = 0;
	ssize_t read;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Couldn't open matrix file for reading\n");
		return NULL;
	}

	if (!fgets(line, READ_FILE_LINE_LENGTH, fp) || 
		tokenize(line, " ", tokens) != 2 ||
		!validateDouble(tokens[0]) ||
		!validateDouble(tokens[1])
		)
	{
		printf("Expected rows [space] cols on line 1 of %s\n", cmd[*ctp]);
		return NULL;
	}
	r = strtod(tokens[0], NULL);
	c = strtod(tokens[1], NULL);

	TMatrix *m = allocMatrix(r, c);
	// MATRIX
	int i = 0;
	for(i = 0;i<r;i++) {
		if (!fgets(line, READ_FILE_LINE_LENGTH, fp)) {
			printf("%s:%d: expected %d rows, but %d were found instead\n", filename, i+3, r, i);
			return NULL;
		}
		// printf("Matrix line: %s\n", line);
		size_t numTok;
		if ((numTok = tokenize(line, DELIM, tokens)) != c) {
			printf("%s:%d: expected %d columns, but %zu were found instead\n", filename, i+3, c, numTok);
			return NULL;
		}
		int j;
		for (j=0;j<c;j++) {
			m->matrix[i][j] = strtod(tokens[j], NULL);
		}
	}
	freeTokens(tokens);
	return m;

}

int validateMatrix(char **cmd, size_t *ctp, int *r, int *c) {
	// -1 to force 1 row and 1 column
	int tmpR = 0;
	int tmpC = 0;
	*c = 0;
	*r = 0;
	// don't consume ctp, this is only validating
	size_t i = *ctp;
	if (!cmd) return 0; 
	if (!consume("{",cmd, ctp)) { 
		return 0; 
	}
	// i++;
	while ((cmd[*ctp]&&strcmp(cmd[*ctp],"}")!=0)||!tmpR) {
		// Read open bracket
		if (!consume("[",cmd, ctp)) { 
			return 0; 
		}
		tmpC = 0;
		while (readDouble(cmd, ctp)) {
			tmpC++;
		}
		if (tmpC == 0) {
			printf("At least one column was expected in matrix row\n");
			printErrorNearToken("0", cmd, *ctp);
			return 0;
		}
		if (!consume("]",cmd, ctp)) { return 0; }
		if (*c&&tmpC!=*c){
			printf("Error: The matrix must have the same number of columns in each row\n");
			return 0;
		}
		else *c = tmpC;
		tmpR++;
	}
	if (!consume("}",cmd, ctp)) {  return 0; }
	*r = tmpR;
	*c = tmpC;
	*ctp=i;
	return 1;
}

TMatrix *readTranspose(char **cmd, size_t *ctp) {
	if (!consume(CMD_TRANSPOSE, cmd, ctp) || !consume("(", cmd, ctp)) {
		return NULL;
	}
	TMatrix *ret =  mTranspose(readMatrix(cmd, ctp));
	if (ret) {
		if (!consume(")", cmd, ctp)) {
			return NULL;
		}
	}
	return ret;
}

TMatrix *readInverse(char **cmd, size_t *ctp) {
	if (!consume(CMD_INVERSE, cmd, ctp) || !consume("(", cmd, ctp)) {
		return NULL;
	}
	TMatrix *ret =  mInverse(readMatrix(cmd, ctp));
	if (ret) {
		if (!consume(")", cmd, ctp)) {
			return NULL;
		}
	}
	return ret;
}

TMatrix *readAdjoint(char **cmd, size_t *ctp) {
	if (!consume(CMD_ADJOINT, cmd, ctp) || !consume("(", cmd, ctp)) {
		return NULL;
	}
	TMatrix *ret =  mAdjoint(readMatrix(cmd, ctp));
	if (ret) {
		if (!consume(")", cmd, ctp)) {
			return NULL;
		}
	}
	return ret;
}

TMatrix *readCofactor(char **cmd, size_t *ctp) {
	if (!consume(CMD_COFACTOR, cmd, ctp) || !consume("(", cmd, ctp)) {
		return NULL;
	}
	TMatrix *ret = readMatrix(cmd, ctp);
	if (ret) {
		double *i;
		double *j;
		if (!(i = readDouble(cmd, ctp)) || !(j = readDouble(cmd, ctp))) {
			printErrorNearToken("1", cmd, *ctp);
			return NULL;
		}
		if (!consume(")", cmd, ctp)) {
			return NULL;
		}
		(*i)--;
		(*j)--;
		// We start at 0, user starts at 1, so decrease 1
		if (*i<0||*i>ret->rows) {
			printf("%d is not a valid row\n", (int)*i+1);
			return NULL;
		}
		if (*j<0||*j>ret->cols) {
			printf("%d is not a valid column\n", (int)*j+1);
			return NULL;
		}
		return mCofactor(ret, (int)*i, (int)*j);
	}
	return NULL;
}

double *readSqrt(char **cmd, size_t *ctp) {
	if (!consume(CMD_SQRT, cmd, ctp) || !consume("(", cmd, ctp)) {
		return NULL;
	}
	int type = getTokType(cmd[*ctp]);
	if (type == TYPE_DOUBLE) {
		double *res = malloc(sizeof(double *));
		double *d = readDouble(cmd, ctp);
		if ( d == NULL) return NULL;
		*res = sqrt(*d);
		if (!consume(")", cmd, ctp)) {
			return NULL;
		}
		return res;
	} else { // Assume TYPE_MATRIX
		TMatrix *r = readMatrix(cmd, ctp);
		if (r == NULL) return NULL;
		if (!consume(")", cmd, ctp)) {
			return NULL;
		}
		if (r->rows != 1 && r->cols != 1) {
			printf("The matrix should be a 1xn or nx1 matrix\n");
			return NULL;
		}
		double **m = r->matrix;
		if (r->rows == 1) {
			double *res = malloc(sizeof(double *));
			int i;
			for (i = 0; i < r->cols; i++) {
				*res += pow(m[0][i], 2);
			}
			*res = sqrt(*res);
			return res;
		} else { // r->cols == 1
			double *res = malloc(sizeof(double *));
			int i;
			for (i = 0; i < r->rows; i++) {
				*res += pow(m[i][0], 2);
			}
			*res = sqrt(*res);
			return res;
		}
	}
	return NULL;
}

int expect(char *tok, char **cmd, size_t *ctp) {
	if (!tok && !cmd[*ctp]) {
		return 1;
	} else if (tok && cmd[*ctp] && strcmp(tok, cmd[*ctp]) == 0) {
		return 1;
	} else if (!tok) {
		printf("Token no esperado: \"%s\"\n", cmd[*ctp]);
	} else if (!cmd[*ctp]) {
		printf("Se esperaba el token \"%s\"\n", tok);
	} else {
		printf("Se esperaba \"%s\" en vez de \"%s\"\n", tok, cmd[*ctp]);
	}
	printErrorNearToken(tok, cmd, *ctp);
	return 0;
}

int consume(char *tok, char **cmd, size_t *ctp) {
	if (expect(tok, cmd, ctp)) {
		(*ctp)++;
		return 1;
	} else {
		return 0;
	}
}

int consumeFilename(char **cmd, size_t *ctp) {
	if (!validateFilename(cmd[*ctp])) {
		if (cmd[*ctp]) {
			printf("%s invalid filename. Valid filenames consist of letters, numbers, dots, / and \\\n", cmd[*ctp]);
		} else {
			printf("Expected filename\n");
		}
		printErrorNearToken("matrix.txt", cmd, *ctp);
		return 0;
	}
	(*ctp)++;
	return 1;
}


int validateDouble(char *str) {
	if (!str) { return 0; }
	char *endPtr;
	strtod(str, &endPtr);
	if (*endPtr == '\0' || *endPtr == '\n') { return 1; }
	else return 0;
}


int validateFilename(char *str) {
	if (!str) { return 0; }
	int i;
	for (i = 0; i < strlen(str);i++) {
		if ((str[i]<'a'||str[i]>'z') &&
			(str[i]<'A'||str[i]>'Z') &&
			(str[i]<'0'||str[i]>'9') &&
			str[i]!='.' && str[i]!='\\' && str[i]!='/')
		{
			return 0;
		}
	}
	return 1;
}


void printErrorNearToken(char *tok, char **cmd, size_t ctp) {
	printf("Suggestion:\n");
	long length = 0;
	size_t i = 0;
	for (i = 0; i < ctp; i++) {
		printf("%s ", cmd[i]);
		length += strlen(cmd[i]) + 1;
	}
	if (tok) {
		printf("%s ", tok);
		// print rest
		for (;cmd[i];i++) {
			printf("%s ", cmd[i]);
		}
		printf("\n");
		for (i = 0; i < length; i++) {
			printf(" ");
		}
		printf("^");
	}
	printf("\n");
}