#include "minimat.h"

void _printMatrix(TMatrix *m, FILE *fp, int printDimensions);

TMatrix *_allocated_matrices[MAX_ALLOCATED_MATRICES] = { NULL };

void printMatrix(TMatrix *m) {
	// print matrix to stdout without dimensions
	_printMatrix(m, stdout, 0);
}

void printMatrixToFile(char **cmd, size_t *ctp) {
	(*ctp)++;
	if (!consume("(", cmd, ctp)) {
		return;
	}
	TMatrix *m = readMatrix(cmd, ctp);
	if (!m) {
		return;
	}
	// consume filename
	char *filename = cmd[*ctp];
	if (!consumeFilename(cmd, ctp)) return;

	if (!consume(")", cmd, ctp)) {
		return;
	}

	FILE *fp = fopen(filename, "w");
	if (!fp) {
		printf("Couldn't open %s for writing\n", filename);
		return;
	}
	// print matrix to file with dimensions
	_printMatrix(m, fp, 1);
	fclose(fp);

	
}

void _printMatrix(TMatrix *m, FILE *fp, int printDimensions) {
	if (!m) return;
	int r = m->rows;
	int c = m->cols;
	if (printDimensions) {
		fprintf(fp, "%d %d\n", r, c);		
	}
	int i;
	for (i = 0;i<r;i++) {
		int j;
		for (j = 0;j<c;j++) {
			// Workaround to avoid ugly -0
			if (m->matrix[i][j]==-0) m->matrix[i][j]=0;
			// newline when at end of row
			fprintf(fp, "%10.3f%c", m->matrix[i][j],j==(c-1)?'\n':' ');
		}
	}
}

TMatrix *allocMatrix(int rows, int cols){
	TMatrix *m = malloc(sizeof(TMatrix));
	m->rows = rows;
	m->cols = cols;
	m->matrix = malloc(sizeof(double *) * rows);
	int i;
	for (i = 0;i<rows;i++) {
		m->matrix[i] = malloc(sizeof(double)*cols);
	}
	addAllocatedMatrix(m);
	return m;
}

void freeMatrix(TMatrix *m){
	if (m != NULL) {
		free(m->matrix);
		free(m);
	}
}

void addAllocatedMatrix(TMatrix *m) {
	int i;
	for (i = 0; i<MAX_ALLOCATED_MATRICES&&_allocated_matrices[i]!=NULL;i++) {
		// Set i to the null pointer
	}
	_allocated_matrices[i] = m;
	_allocated_matrices[i+1] = NULL;
}

void freeAllMatrices() {
	int i;
	TMatrix **w = getVarPointer("W");
	TMatrix **x = getVarPointer("X");
	TMatrix **y = getVarPointer("Y");
	TMatrix **z = getVarPointer("Z");
	for (i = 0;i<MAX_ALLOCATED_MATRICES;i++) {
		TMatrix *ptr = _allocated_matrices[i];
		if (ptr!=NULL&&ptr!=*w&&ptr!=*x&&ptr!=*y&&ptr!=*z){
			freeMatrix(ptr);
			_allocated_matrices[i]=NULL;
		}		
	}
}

void fillMatrix(TMatrix *m, double val) {
	if (!m) return;
	int r = m->rows;
	int c = m->cols;
	int i;
	for (i = 0;i<r;i++) {
		int j;
		for (j = 0;j<c;j++) {
			m->matrix[i][j]=val;
		}
	}
}

TMatrix *mOpposite(TMatrix *m) {
	if (!m) return NULL;
	int r = m->rows;
	int c = m->cols;
	TMatrix *tmp = allocMatrix(r,c);
	int i;
	for (i = 0;i<r;i++) {
		int j;
		for (j = 0;j<c;j++) {
			tmp->matrix[i][j] *= m->matrix[i][j]*-1;
		}
	}
	return tmp;
}

TMatrix *mAdd(TMatrix *m, TMatrix *m2) {	
	if (!m || !m2){ return NULL; }
	int r = m->rows;
	int c = m->cols;
	if (r != m2->rows || c != m2->cols) {
		printf("Error, to add matrices, dimensions have to be the same\n");
		return NULL;
	}
	TMatrix *tmp = allocMatrix(r, c);
	int i;
	for (i = 0;i<r;i++) {
		int j;
		for (j = 0;j<c;j++) {
			tmp->matrix[i][j]=m->matrix[i][j]+m2->matrix[i][j];
		}
	}
	return tmp;
}

TMatrix *mSub(TMatrix *m, TMatrix *m2) {
	return mAdd(m, mOpposite(m2));
}

TMatrix *mMult(TMatrix *m1, TMatrix *m2) {
	if (!m1||!m2) { return NULL; }
	int r1 = m1->rows;
	int c1 = m1->cols;
	int r2 = m2->rows;
	int c2 = m2->cols;
	if (c1 != r2) {
		printf("Can't multiply these matrices.\n");
		return NULL;
	}
	TMatrix *res = allocMatrix(r1, c2);
	int i;
	for (i = 0;i<r1;i++) {
		int j;
		for (j = 0;j<c2;j++) {
			// For each IJ of the resulting matrix:
			double resultElement = 0;
			int k;
			for (k = 0; k < c1; k++) {
				resultElement += m1->matrix[i][k] * m2->matrix[k][j];
			}
			res->matrix[i][j] = resultElement;
		}	
	}
	return res;
}

TMatrix *mMultDouble(TMatrix *m, double *d) {
	if(!m || !d){ return NULL; }
	int r = m->rows;
	int c = m->cols;
	TMatrix *tmp = allocMatrix(r, c);
	int i;
	for (i = 0;i<r;i++) {
		int j;
		for (j = 0;j<c;j++) {
			tmp->matrix[i][j]=m->matrix[i][j] * (*d);
		}
	}
	return tmp;
}

TMatrix *mDivideDouble(TMatrix *m, double *d) {
	if(!m || !d){ return NULL; }
	if (*d == 0){printf("Error: no se puede dividir por 0\n");return NULL;}
	int r = m->rows;
	int c = m->cols;
	TMatrix *tmp = allocMatrix(r, c);
	int i;
	for (i = 0; i<r; i++) {
		int j;
		for (j = 0; j<c; j++) {
			tmp->matrix[i][j] = m->matrix[i][j] / (*d);
		}
	}
	return tmp;
}

TMatrix *mTranspose(TMatrix *m) {
	if (!m) return NULL;
	int r = m->rows;
	int c = m->cols;
	TMatrix *tmp = allocMatrix(c, r);
	int i;
	for (i = 0; i<r; i++) {
		int j;
		for (j = 0; j<c; j++) {
			tmp->matrix[j][i] = m->matrix[i][j];
		}
	}
	return tmp;
}

TMatrix *mAdjoint(TMatrix *m) {
	if (!m) return NULL;
	int r = m->rows;
	int c = m->cols;
	if (r != c) {return NULL;}
	TMatrix *tmp = allocMatrix(r, r);
	int i;
	for (i = 0; i<r; i++) {
		int j;
		for (j = 0; j<r; j++) {
			double sign = ((i+j) % 2 == 0) ? 1 : -1;
			tmp->matrix[i][j] = *mDeterminant(mCofactor(m,i,j)) * sign;
		}
	}
	return tmp;
}

TMatrix *mCofactor(TMatrix *m, int r, int c) {
	if (!m) return NULL;
	int size = m->rows;
	TMatrix *ret = allocMatrix(size-1, size-1);
	int i;
	for (i = 0; i<size; i++) {
		if (i == r) continue;
		int j = 0;
		for (j = 0; j<size; j++) {
			if (j == c) continue;
			ret->matrix[i<r ? i : i-1][j<c ? j : j-1] = m->matrix[i][j];
		}
	}
	return ret;
}

TMatrix *mInverse(TMatrix *m) {
	if (!m) return NULL;
	double *det = mDeterminant(m);
	if (det&&*det) {
		return mDivideDouble(mTranspose(mAdjoint(m)),det);
	} else {
		printf("Error: inverse matrix doesn't exist\n");
		return NULL;
	}
}



// FIXME segfault { [ 1 2 ] } & { [ 1 ] [ 2 ] }
TMatrix *mCat(TMatrix *m1, TMatrix *m2) {
	if (!m1 || !m2) { return NULL; }
	int r1 = m1->rows;
	int c1 = m1->cols;
	int r2 = m2->rows;
	int c2 = m2->cols;
	double **matrix1 = m1->matrix;
	double **matrix2 = m2->matrix;
	int resRows = r1 > r2 ? r1 : r2;
	int resCols = c1 + c2;
	TMatrix *res = allocMatrix(resRows, resCols);
	fillMatrix(res, 0);
	int offset = 0;
	// m1
	int i, j;
	for (i = 0; i < r1; i++) {
		for (j = 0; j < c1; j++) {
			res->matrix[i][j + offset] = matrix1[i][j];
		}
	}
	offset = c1;
	// m2
	for (i = 0; i < r2; i++) {
		for (j = 0; j < c2; j++) {
			res->matrix[i][j + offset] = matrix2[i][j];
		}
	}
	return res;
}

double *mDeterminant(TMatrix *m) {
	int r = m->rows;
	int c = m->cols;
	if (r != c) {
		printf("Matrix must be square for determinant to be calculated\n");
		return NULL;
	}
	else if (r == 1) {
		// determinants of 1x1 matrices are defined as their only element: det(m) = m(1)(1)
		double *d = malloc(sizeof(double *));
		*d = m->matrix[0][0];
		return d;
	} else if (r == 2) {
		// determinants of 2x2 matrices are: det(m) = a * d - b * c
		double *d = malloc(sizeof(double *));
		*d = m->matrix[0][0]*m->matrix[1][1]-m->matrix[0][1]*m->matrix[1][0];
		return d;
	} else {
		// calculate the determinant with adjoints untill we get 2x2 matrices
		double *detp = malloc(sizeof(double *));
		*detp = 0;
		int i;
		for (i = 0;i<r;i++){
			double sign = (i%2==0?1:-1);
			double mij = m->matrix[0][i];
			double mijdet = *mDeterminant(mMultDouble(mCofactor(m, 0, i), &sign));			
			*detp += m->matrix[0][i] * mijdet * sign;
		}
		return detp;
	}
	return NULL;
}