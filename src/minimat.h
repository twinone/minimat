#ifndef __INC_MINIMAT_HEADER
#define __INC_MINIMAT_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h> // size_t

#define prompt "minimat> "
#define DELIM " \n\t"

#define CMD_READ "read"
#define CMD_WRITE "write"
#define CMD_DET "det"
#define CMD_EXIT "exit"
#define CMD_TRANSPOSE "transp"
#define CMD_INVERSE "inv"
#define CMD_RANGE "rg"
#define CMD_ADJOINT "adj"
#define CMD_COFACTOR "co"
#define CMD_SQRT "sqrt"


#define MAX_CMD_LEN 4096
#define MAX_TOKENS 2048
#define MAX_ALLOCATED_MATRICES 1024

#define TYPE_UNKNOWN 0
#define TYPE_DOUBLE 1
#define TYPE_MATRIX 2
#define TYPE_OP 3

#define READ_FILE_LINE_LENGTH 1024

// Matrix
typedef struct {
	int rows, cols;
	double **matrix;
} TMatrix;

void *getVarPointer(char *tok);

void *eval(char **cmd, size_t *ctp, int *resultType);


// logic.c
void *logicAdd(int t1, int t2, void *p1, void *p2);
void *logicSub(int t1, int t2, void *p1, void *p2);
void *logicMult(int t1, int t2, void *p1, void *p2);
void *logicDivide(int t1, int t2, void *p1, void *p2);
void *logicModulus(int t1, int t2, void *p1, void *p2);
void *logicCat(int t1, int t2, void *p1, void *p2);

// matrix.c
void printMatrix(TMatrix *matrix);
void printMatrixToFile(char **cmd, size_t *ctp);

// memory management
TMatrix *allocMatrix(int rows, int cols);
void freeMatrix(TMatrix *matrix);
void addAllocatedMatrix(TMatrix *m);
void freeAllMatrices();

/* Operations */
void fillMatrix(TMatrix *m, double val);
// Opposite matrix of m is -m, in which all elements are their opposites
TMatrix *mOpposite(TMatrix *m);
// Add matrix m with matrix m2
TMatrix *mAdd(TMatrix *m, TMatrix *m2);
TMatrix *mSub(TMatrix *m, TMatrix *m2);
TMatrix *mMult(TMatrix *m, TMatrix *m2);
// Multiplies a matrix with a number
TMatrix *mMultDouble(TMatrix *m, double *d);
TMatrix *mDivideDouble(TMatrix *m, double *d);
TMatrix *mTranspose(TMatrix *m);
TMatrix *mAdjoint(TMatrix *m);
// Expects square matrix, gets cofactor matrix of m(i)(j)
TMatrix *mCofactor(TMatrix *m, int i, int j);
TMatrix *mInverse(TMatrix *m);
// Concatenates two matrices
TMatrix *mCat(TMatrix *m1, TMatrix *m2);
double *mDeterminant(TMatrix *m);

// parse.c
// All functions that involve parsing and validating

// distribution function
double *readDouble(char **cmd, size_t *ctp);
double *readDet(char **cmd, size_t *ctp);
double *readSqrt(char **cmd, size_t *ctp);

// distribution function
TMatrix *readMatrix(char **cmd, size_t *ctp);
TMatrix *readMatrixFromFile(char **cmd, size_t *ctp);
TMatrix *readInverse(char **cmd, size_t *ctp);
TMatrix *readAdjoint(char **cmd, size_t *ctp);
TMatrix *readCofactor(char **cmd, size_t *ctp);
TMatrix *readTranspose(char **cmd, size_t *ctp);
// calls validateMatrix
TMatrix *readMatrixLiteral(char **cmd, size_t *ctp);
// doesn't consume ctp
int validateMatrix(char **cmd, size_t *ctp, int *r, int *c);
int validateDouble(char *string);
int validateFilename(char *string);

// only increases ctp if read was successful
char tryReadOperator(char **cmd, size_t *ctp);

// expect a token and if it's not at that position, an error is printed
int expect(char *tok, char **cmd, size_t *ctp);
// same as expect, and increases current token pointer
int consume (char *expect, char **cmd, size_t *ctp);
int consumeFilename(char **cmd, size_t *ctp);

// util.c
size_t tokenize(char *input, char *delim, char ** array);
void freeTokens(char **array);
char *getTypeString(int type);
int getTokType(char *tok);
int getReturnType(int v1, char op, int v2);
void printErrorNearToken(char *tok, char **cmd, size_t ctp);

#endif