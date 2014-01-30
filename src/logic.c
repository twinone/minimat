#include "minimat.h"

void *logicAdd(int t1, int t2, void *p1, void *p2) {
	if (t1 != t2){
		printf("Can't operate %s + %s\n", getTypeString(t1), getTypeString(t2));
		return NULL;
	}
	if (t1 == TYPE_DOUBLE) {
		double *res = malloc(sizeof(double *));
		*res = (*(double*)p1 + *(double*)p2);
		return res;
	} else {
		return mAdd(p1, p2);
	}
}

void *logicSub(int t1, int t2, void *p1, void *p2) {
	if (t1 != t2){
		printf("Can't operate %s - %s\n",getTypeString(t1), getTypeString(t2));
		return NULL;
	}
	if (t1==TYPE_DOUBLE) {
		double *res = malloc(sizeof(double *));
		*res = (*(double*)p1-*(double*)p2);
		return res;
	} else {
		return mSub(p1, p2);
	}
}

void *logicMult(int t1, int t2, void *p1, void *p2) {
	if (t1 == TYPE_DOUBLE && t2 == TYPE_DOUBLE) {
		double *s = malloc(sizeof(double *));
		*s = *(double*)p1 * (*(double*)p2);
		return s;
	}  else if (t1 != t2) {
		return (t1 == TYPE_DOUBLE ? mMultDouble(p2,p1) : mMultDouble(p1,p2));
	} else {
		return mMult(p1, p2);
	}
}

void *logicDivide(int t1, int t2, void *p1, void *p2) {
	if (t1==TYPE_DOUBLE && t2==TYPE_DOUBLE) {
		double *s = malloc(sizeof(double *));
		*s = *(double*)p1 / (*(double*)p2);
		return s;
	}  else if (t1!=t2) {
		if (t1==TYPE_DOUBLE) {
			printf("Can't operate %s / %s\n", getTypeString(TYPE_DOUBLE), getTypeString(TYPE_MATRIX));
		}
		return mDivideDouble(p1,p2);
	} else {
		printf("Can't divide matrices\n");
		return NULL;
	}
}
void *logicModulus(int t1, int t2, void *p1, void *p2) {
	if (t1!=TYPE_DOUBLE || t2!=TYPE_DOUBLE) {
		printf("Can't operate %s %% %s\n", getTypeString(t1), getTypeString(t2));
		return NULL;
	}
	double *s = malloc(sizeof(double *));
	*s = ((long)*(double *)p1 % (long)*(double *)p2);
	return s;
}

void *logicCat(int t1, int t2, void *p1, void *p2) {
	TMatrix *m1 = (TMatrix *)p1;
	TMatrix *m2 = (TMatrix *)p2;
	if (t1 == TYPE_DOUBLE) {
		m1 = allocMatrix(1, 1);
		m1->matrix[0][0] = *(double*)p1;
	}
	if (t2 == TYPE_DOUBLE) {
		m2 = allocMatrix(1, 1);
		m2->matrix[0][0] = *(double*)p2;
	}
	return mCat(m1, m2);
}

