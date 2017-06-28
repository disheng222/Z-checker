/**
 *  @file ZC_callR.c
 *  @author Sheng Di
 *  @date June, 2017
 *  @brief Calling R from Z-checker
 *  (C) 2017 by Mathematics and Computer Science (MCS), Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Rinternals.h>
#include <Rembedded.h>

#include "ZC_callR.h"

/**
 * Invokes the command source("func.R").
 */
void source(const char *name)
{
    SEXP e;

    PROTECT(e = lang2(install("source"), mkString(name)));
    R_tryEval(e, R_GlobalEnv, NULL);
    UNPROTECT(1);
}


int ZC_callR_1_1d(char* rFuncName, int vecType, int inLen, void* in, int* outLen, double** out)
{
	// Allocate an R vector and copy the C array into it.
    SEXP arg;	
    float *float_in;
    double *double_in;
    int i;
	switch(vecType)
	{
	case ZC_R_Int:
		PROTECT(arg = allocVector(INTSXP, inLen));
		memcpy(INTEGER(arg), (int*)in, inLen* sizeof(int));		
		break;
	case ZC_R_Float:
		float_in = (float*)in;
		double_in = (double*)malloc(inLen*sizeof(double));
		for(i=0;i<inLen;i++)
			double_in[i] = float_in[i];
		PROTECT(arg = allocVector(REALSXP, inLen));
		memcpy(REAL(arg), double_in, inLen* sizeof(double));
		break;
	case ZC_R_Double:
		PROTECT(arg = allocVector(REALSXP, inLen));
		memcpy(REAL(arg), (double*)in, inLen* sizeof(double));	
		break;
	default: 
		printf("Error: vecType (==%d) should only be ZC_R_Int(0), ZC_R_Float(1), or ZC_R_Double(2).\n", vecType);
		return ZC_R_ERROR;
	}

    // Setup a call to the R function
    SEXP r_call;
    PROTECT(r_call = lang2(install(rFuncName), arg));

    // Execute the function
    int errorOccurred;
    SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);
    
    if (!errorOccurred)
    {
        double *val = REAL(ret);
        *outLen = LENGTH(ret);
        *out = (double*)malloc(*outLen*sizeof(double));
        memcpy(*out, val, *outLen*sizeof(double));
        UNPROTECT(2);
        return ZC_R_SUCCESS;
    }
    else
    {
        printf("Error occurred calling R\n");
        UNPROTECT(2);
        return ZC_R_ERROR;
    }
}

int ZC_callR_2_1d(char* rFuncName, int vecType, int in1Len, void* in1, int in2Len, void* in2, int* outLen, double** out)
{
	// Allocate an R vector and copy the C array into it.
    SEXP arg1, arg2;
    float *float_in1, *float_in2;
    double *double_in1, *double_in2;
    int i;
	switch(vecType)
	{
	case ZC_R_Int:
		PROTECT(arg1 = allocVector(INTSXP, in1Len));
		memcpy(INTEGER(arg1), (int*)in1, in1Len* sizeof(int));
		PROTECT(arg2 = allocVector(INTSXP, in2Len));
		memcpy(INTEGER(arg2), (int*)in2, in2Len* sizeof(int));
		break;
	case ZC_R_Float:
		float_in1 = (float*)in1;
		float_in2 = (float*)in2;
		double_in1 = (double*)malloc(in1Len*sizeof(double));
		double_in2 = (double*)malloc(in2Len*sizeof(double));
		for(i=0;i<in1Len;i++)
			double_in1[i] = float_in1[i];
		for(i=0;i<in2Len;i++)
			double_in2[i] = float_in2[i];
		PROTECT(arg1 = allocVector(REALSXP, in1Len));
		PROTECT(arg2 = allocVector(REALSXP, in2Len));
		memcpy(REAL(arg1), double_in1, in1Len* sizeof(double));
		memcpy(REAL(arg2), double_in2, in2Len* sizeof(double));		
		break;
	case ZC_R_Double:
		PROTECT(arg1 = allocVector(REALSXP, in1Len));
		PROTECT(arg2 = allocVector(REALSXP, in2Len));
		memcpy(REAL(arg1), (double*)in1, in1Len* sizeof(double));
		memcpy(REAL(arg2), (double*)in2, in2Len* sizeof(double));	
		break;
	default: 
		printf("Error: vecType (==%d) should only be ZC_R_Int(0), ZC_R_Float(1), or ZC_R_Double(2).\n", vecType);
		return ZC_R_ERROR;
	}

    // Setup a call to the R function
    SEXP r_call;
    PROTECT(r_call = lang3(install(rFuncName), arg1, arg2));

    // Execute the function
    int errorOccurred;
    SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);
    
    if (!errorOccurred)
    {
        double *val = REAL(ret);
        *outLen = LENGTH(ret);
        *out = (double*)malloc(*outLen*sizeof(double));
        memcpy(*out, val, *outLen*sizeof(double));
        UNPROTECT(3);
        return ZC_R_SUCCESS;
    }
    else
    {
        printf("Error occurred calling R\n");
        UNPROTECT(3);
        return ZC_R_ERROR;
    }
}

int ZC_callR_1_2d(char* rFuncName, int vecType, int in_n2, int in_n1, void* in, int* outLen, double** out)
{
	// Allocate an R vector and copy the C array into it.
    SEXP arg;	
    float *float_in;
    double *double_in;
    int i, j;
	switch(vecType)
	{
	case ZC_R_Int:
		PROTECT(arg = allocMatrix(INTSXP, in_n2, in_n1));
		memcpy(INTEGER(arg), (int*)in, in_n2*in_n1* sizeof(int));		
		break;
	case ZC_R_Float:
		float_in = (float*)in;
		double_in = (double*)malloc(in_n2*in_n1*sizeof(double));
		for(i=0;i<in_n2;i++)
			for(j=0;j<in_n1;j++)
				double_in[i*in_n1+j] = float_in[i*in_n1+j];
		PROTECT(arg = allocMatrix(REALSXP, in_n2, in_n1));
		memcpy(REAL(arg), double_in, in_n2*in_n1* sizeof(double));
		break;
	case ZC_R_Double:
		PROTECT(arg = allocMatrix(REALSXP, in_n2, in_n1));
		memcpy(REAL(arg), (double*)in, in_n2*in_n1*sizeof(double));	
		break;
	default: 
		printf("Error: vecType (==%d) should only be ZC_R_Int(0), ZC_R_Float(1), or ZC_R_Double(2).\n", vecType);
		return ZC_R_ERROR;
	}

    // Setup a call to the R function
    SEXP r_call;
    PROTECT(r_call = lang2(install(rFuncName), arg));

    // Execute the function
    int errorOccurred;
    SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);
    
    if (!errorOccurred)
    {
        double *val = REAL(ret);
        *outLen = LENGTH(ret);
        *out = (double*)malloc(*outLen*sizeof(double));
        memcpy(*out, val, *outLen*sizeof(double));
        UNPROTECT(2);
        return ZC_R_SUCCESS;
    }
    else
    {
        printf("Error occurred calling R\n");
        UNPROTECT(2);
        return ZC_R_ERROR;
    }
}

int ZC_callR_2_2d(char* rFuncName, int vecType, 
int in1_n2, int in1_n1, void* in1, 
int in2_n2, int in2_n1, void* in2, 
int* outLen, double** out)
{
	// Allocate an R vector and copy the C array into it.
    SEXP arg1, arg2;
    float *float_in1, *float_in2;
    double *double_in1, *double_in2;
    int i,j;
	switch(vecType)
	{
	case ZC_R_Int:
		PROTECT(arg1 = allocMatrix(INTSXP, in1_n2, in1_n1));
		memcpy(INTEGER(arg1), (int*)in1, in1_n2*in1_n1* sizeof(int));
		PROTECT(arg2 = allocMatrix(INTSXP, in2_n2, in2_n1));
		memcpy(INTEGER(arg2), (int*)in2, in2_n2*in2_n1* sizeof(int));
		break;
	case ZC_R_Float:
		float_in1 = (float*)in1;
		float_in2 = (float*)in2;
		double_in1 = (double*)malloc(in1_n2*in1_n1*sizeof(double));
		double_in2 = (double*)malloc(in2_n2*in2_n1*sizeof(double));
		for(i=0;i<in1_n2;i++)
			for(j=0;j<in1_n1;j++)
				double_in1[i*in1_n1+j] = float_in1[i*in1_n1+j];
		for(i=0;i<in2_n2;i++)
			for(j=0;j<in2_n1;j++)
				double_in2[i*in2_n1+j] = float_in2[i*in2_n1+j];
		PROTECT(arg1 = allocMatrix(REALSXP, in1_n2, in1_n1));
		PROTECT(arg2 = allocMatrix(REALSXP, in2_n2, in2_n1));
		memcpy(REAL(arg1), double_in1, in1_n2*in1_n1* sizeof(double));
		memcpy(REAL(arg2), double_in2, in2_n2	*in2_n1* sizeof(double));		
		break;
	case ZC_R_Double:
		PROTECT(arg1 = allocMatrix(REALSXP, in1_n2, in1_n1));
		PROTECT(arg2 = allocMatrix(REALSXP, in2_n2, in2_n1));
		memcpy(REAL(arg1), (double*)in1, in1_n2*in1_n1* sizeof(double));
		memcpy(REAL(arg2), (double*)in2, in2_n2*in2_n1* sizeof(double));	
		break;
	default: 
		printf("Error: vecType (==%d) should only be ZC_R_Int(0), ZC_R_Float(1), or ZC_R_Double(2).\n", vecType);
		return ZC_R_ERROR;
	}

    // Setup a call to the R function
    SEXP r_call;
    PROTECT(r_call = lang3(install(rFuncName), arg1, arg2));

    // Execute the function
    int errorOccurred;
    SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);
    
    if (!errorOccurred)
    {
        double *val = REAL(ret);
        *outLen = LENGTH(ret);
        *out = (double*)malloc(*outLen*sizeof(double));
        memcpy(*out, val, *outLen*sizeof(double));
        UNPROTECT(3);
        return ZC_R_SUCCESS;
    }
    else
    {
        printf("Error occurred calling R\n");
        UNPROTECT(3);
        return ZC_R_ERROR;
    }
}

int ZC_callR_1_3d(char* rFuncName, int vecType, int in_n3, int in_n2, int in_n1, void* in, int* outLen, double** out)
{
	// Allocate an R vector and copy the C array into it.
    SEXP arg;	
    float *float_in;
    double *double_in;
    int i, j, k, n21;
	switch(vecType)
	{
	case ZC_R_Int:
		PROTECT(arg = alloc3DArray(INTSXP, in_n2, in_n1, in_n3));
		memcpy(INTEGER(arg), (int*)in, in_n3*in_n2*in_n1* sizeof(int));		
		break;
	case ZC_R_Float:
		float_in = (float*)in;
		double_in = (double*)malloc(in_n3*in_n2*in_n1*sizeof(double));
		n21 = in_n2*in_n1;
		for(i=0;i<in_n3;i++)
			for(j=0;j<in_n2;j++)
				for(k=0;k<in_n1;k++)
					double_in[i*n21+j*in_n1+k] = float_in[i*n21+j*in_n1+k];
		PROTECT(arg = alloc3DArray(REALSXP, in_n2, in_n1, in_n3));
		memcpy(REAL(arg), double_in, in_n3*in_n2*in_n1* sizeof(double));
		break;
	case ZC_R_Double:
		PROTECT(arg = alloc3DArray(REALSXP, in_n2, in_n1, in_n3));
		memcpy(REAL(arg), (double*)in, in_n3*in_n2*in_n1*sizeof(double));	
		break;
	default: 
		printf("Error: vecType (==%d) should only be ZC_R_Int(0), ZC_R_Float(1), or ZC_R_Double(2).\n", vecType);
		return ZC_R_ERROR;
	}

    // Setup a call to the R function
    SEXP r_call;
    PROTECT(r_call = lang2(install(rFuncName), arg));

    // Execute the function
    int errorOccurred;
    SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);
    
    if (!errorOccurred)
    {
        double *val = REAL(ret);
        *outLen = LENGTH(ret);
        *out = (double*)malloc(*outLen*sizeof(double));
        memcpy(*out, val, *outLen*sizeof(double));
        UNPROTECT(2);
        return ZC_R_SUCCESS;
    }
    else
    {
        printf("Error occurred calling R\n");
        UNPROTECT(2);
        return ZC_R_ERROR;
    }
}

int ZC_callR_2_3d(char* rFuncName, int vecType, 
int in1_n3, int in1_n2, int in1_n1, void* in1, 
int in2_n3, int in2_n2, int in2_n1, void* in2, 
int* outLen, double** out)
{
	// Allocate an R vector and copy the C array into it.
    SEXP arg1, arg2;
    float *float_in1, *float_in2;
    double *double_in1, *double_in2;
    int i,j,k;
	int in1_n21 = in1_n2*in1_n1;
	int in2_n21 = in2_n2*in2_n1;
	switch(vecType)
	{
	case ZC_R_Int:
		PROTECT(arg1 = alloc3DArray(INTSXP, in1_n2, in1_n1, in1_n3));
		memcpy(INTEGER(arg1), (int*)in1, in1_n3*in1_n2*in1_n1* sizeof(int));
		PROTECT(arg2 = alloc3DArray(INTSXP, in2_n2, in2_n1, in2_n3));
		memcpy(INTEGER(arg2), (int*)in2, in2_n3*in2_n2*in2_n1* sizeof(int));
		break;
	case ZC_R_Float:
		float_in1 = (float*)in1;
		float_in2 = (float*)in2;
		double_in1 = (double*)malloc(in1_n3*in1_n2*in1_n1*sizeof(double));
		double_in2 = (double*)malloc(in2_n3*in2_n2*in2_n1*sizeof(double));
		
		for(i=0;i<in1_n3;i++)
			for(j=0;j<in1_n2;j++)
				for(k=0;k<in1_n1;k++)
					double_in1[i*in1_n21+j*in1_n1+k] = float_in1[i*in1_n21+j*in1_n1+k];
		for(i=0;i<in2_n3;i++)
			for(j=0;j<in2_n2;j++)
				for(k=0;k<in2_n1;k++)
					double_in2[i*in2_n21+j*in2_n1+k] = float_in2[i*in2_n21+j*in2_n1+k];
		PROTECT(arg1 = alloc3DArray(REALSXP, in1_n2, in1_n1, in1_n3));
		PROTECT(arg2 = alloc3DArray(REALSXP, in2_n2, in2_n1, in2_n3));
		memcpy(REAL(arg1), double_in1, in1_n3*in1_n2*in1_n1* sizeof(double));
		memcpy(REAL(arg2), double_in2, in2_n3*in2_n2*in2_n1* sizeof(double));		
		break;
	case ZC_R_Double:
		PROTECT(arg1 = alloc3DArray(REALSXP, in1_n2, in1_n1, in1_n3));
		PROTECT(arg2 = alloc3DArray(REALSXP, in2_n2, in2_n1, in2_n3));
		memcpy(REAL(arg1), (double*)in1, in1_n3*in1_n2*in1_n1* sizeof(double));
		memcpy(REAL(arg2), (double*)in2, in2_n3*in2_n2*in2_n1* sizeof(double));	
		break;
	default: 
		printf("Error: vecType (==%d) should only be ZC_R_Int(0), ZC_R_Float(1), or ZC_R_Double(2).\n", vecType);
		return ZC_R_ERROR;
	}

    // Setup a call to the R function
    SEXP r_call;
    PROTECT(r_call = lang3(install(rFuncName), arg1, arg2));

    // Execute the function
    int errorOccurred;
    SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);
    
    if (!errorOccurred)
    {
        double *val = REAL(ret);
        *outLen = LENGTH(ret);
        *out = (double*)malloc(*outLen*sizeof(double));
        memcpy(*out, val, *outLen*sizeof(double));
        UNPROTECT(3);
        return ZC_R_SUCCESS;
    }
    else
    {
        printf("Error occurred calling R\n");
        UNPROTECT(3);
        return ZC_R_ERROR;
    }
}
