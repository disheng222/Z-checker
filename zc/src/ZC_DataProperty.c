#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include "ZC_DataProperty.h"
#include "zc.h"
#include "iniparser.h"

/* For entropy calculation */
void hash_init(HashEntry *table, size_t table_size)
{
	size_t i;
	for (i = 0; i < table_size; i++)
		table[i].flag = 0;
}

size_t hash_get(HashEntry *table, unsigned long key, size_t table_size)
{
      int hash = (key % table_size);
      while (table[hash].flag != 0 && table[hash].key != key)
            hash = (hash + 1) % table_size;
      if (table[hash].flag == 0)
            return -1;
      else
            return table[hash].num;
}

void hash_put(HashEntry *table, unsigned long key, size_t table_size)
{
	size_t hash = (key % table_size);
	size_t i = 0;
	for (i=0;table[hash].flag != 0 && table[hash].key != key && i<table_size;i++)
		hash = (hash + 1) % table_size;
		
	if(i==table_size)
	{
		printf("Error: hash table_size is not long enough!\n");
		exit(0);
	}	
		
	if (table[hash].flag != 0)
	table[hash].num++;
	else
	{
	table[hash].flag = 1;
	table[hash].key = key;
	table[hash].num = 1;
	}
}

int ZC_computeDimension(size_t r5, size_t r4, size_t r3, size_t r2, size_t r1)
{
	if(r1==0)
		return 0;
	if(r2==0)
		return 1;
	if(r3==0)
		return 2;
	if(r4==0)
		return 3;
	if(r5==0)
		return 4;
	else
		return 5;
		
}

void ZC_constructDimString(size_t r5, size_t r4, size_t r3, size_t r2, size_t r1, char* output)
{
	int dim = ZC_computeDimension(r5, r4, r3, r2, r1);
	switch(dim)
	{
	case 1:
		sprintf(output, "%zu\0", r1);
		break;
	case 2: 
		sprintf(output, "%zuX%zu\0", r2, r1);
		break;
	case 3: 
		sprintf(output, "%zuX%zuX%zu\0", r3, r2, r1);
		break;
	case 4:
		sprintf(output, "%zuX%zuX%zuX%zu\0", r4, r3, r2, r1);
		break;
	case 5:
		sprintf(output, "%zuX%zuX%zuX%zuX%zu\0", r5, r4, r3, r2, r1);
		break;
	}
}

/* For FFT and iFFT calculation */
void fft(complex *v, size_t n, complex *tmp)
{
	if(n>1) {			/* otherwise, do nothing and return */
		size_t k,m;
		complex z, w, *vo, *ve;
		ve = tmp;
		vo = tmp+n/2;
		for(k=0; k<n/2; k++) {
			ve[k] = v[2*k];
			vo[k] = v[2*k+1];
		}
		fft(ve, n/2, v);		/* FFT on even-indexed elements of v[] */
		fft(vo, n/2, v);		/* FFT on odd-indexed elements of v[] */
		for(m = 0; m < n/2; m++) {
			w.Re = cos(2*PI*m/(double)n);
			w.Im = -sin(2*PI*m/(double)n);
			z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
			z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
			v[m].Re = ve[m].Re + z.Re;
			v[m].Im = ve[m].Im + z.Im;
			v[m+n/2].Re = ve[m].Re - z.Re;
			v[m+n/2].Im = ve[m].Im - z.Im;
		}
	}
	return;
}

void ifft(complex *v, size_t n, complex *tmp)
{
	if(n>1) {			/* otherwise, do nothing and return */
		size_t k,m;
		complex z, w, *vo, *ve;
		ve = tmp;
		vo = tmp+n/2;
		for(k = 0; k < n/2; k++) {
			ve[k] = v[2*k];
			vo[k] = v[2*k+1];
		}
		ifft(ve, n/2, v);		/* FFT on even-indexed elements of v[] */
		ifft(vo, n/2, v);		/* FFT on odd-indexed elements of v[] */
		for(m = 0; m < n/2; m++) {
			w.Re = cos(2*PI*m/(double)n);
			w.Im = sin(2*PI*m/(double)n);
			z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
			z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
			v[m].Re = ve[m].Re + z.Re;
			v[m].Im = ve[m].Im + z.Im;
			v[m+n/2].Re = ve[m].Re - z.Re;
			v[m+n/2].Im = ve[m].Im - z.Im;
		}
	}
	return;
}

void computeLap(double *data, double *lap, size_t r5, size_t r4, size_t r3, size_t r2, size_t r1)
{
	if (r2 == 0)		// compute Laplacian of 1D data
	{
		size_t x, y;
			for (x = 0; x < r1; x++) {
				unsigned long i = max(1u, min(x, r1 - 2));
				double fxx = 1 * data[(i - 1)] - 2 * data[(i + 0)] + 1 * data[(i + 1)];
				double ddf = fxx;
				lap[x] = ddf;
			}
	}
	else if (r3 ==0)	// computer Laplacian of 2D data
	{
		size_t x, y;
		for (y = 0; y < r2; y++) {
			unsigned long j = max(1u, min(y, r2 - 2));
			for (x = 0; x < r1; x++) {
				unsigned long i = max(1u, min(x, r1 - 2));
				double fxx = +1 * data[(i - 1) + r1 * (j + 0)]
						-2 * data[(i + 0) + r1 * (j + 0)]
								+1 * data[(i + 1) + r1 * (j + 0)];
				double fyy = +1 * data[(i + 0) + r1 * (j - 1)]
						-2 * data[(i + 0) + r1 * (j + 0)]
								+1 * data[(i + 0) + r1 * (j + 1)];
				double ddf = fxx + fyy;
				lap[x + y * r1] = ddf;
			}
		}
	}
	else if (r4 == 0)	/*computer Laplacian of 3D data*/
	{
		size_t x, y, z;
		for (z = 0; z < r3; z++) {
			unsigned long k = max(1u, min(z, r3 - 2));
			for (y = 0; y < r2; y++) {
				unsigned long j = max(1u, min(y, r2 - 2));
				for (x = 0; x < r1; x++) {
					unsigned long i = max(1u, min(x, r1 - 2));
					double fxx = +1 * data[(i - 1) + r1 * ((j + 0) + r2 * (k + 0))]
											   -2 * data[(i + 0) + r1 * ((j + 0) + r2 * (k + 0))]
															 +1 * data[(i + 1) + r1 * ((j + 0) + r2 * (k + 0))];
					double fyy = +1 * data[(i + 0) + r1 * ((j - 1) + r2 * (k + 0))]
											   -2 * data[(i + 0) + r1 * ((j + 0) + r2 * (k + 0))]
															 +1 * data[(i + 0) + r1 * ((j + 1) + r2 * (k + 0))];
					double fzz = +1 * data[(i + 0) + r1 * ((j + 0) + r2 * (k - 1))]
											   -2 * data[(i + 0) + r1 * ((j + 0) + r2 * (k + 0))]
															 +1 * data[(i + 0) + r1 * ((j + 0) + r2 * (k + 1))];
					double ddf = fxx + fyy + fzz;
					lap[x + y * r1 + z * r1 * r2] = ddf;
				}
			}
		}
	}
	/* TODO: computer Laplacian of 4D and 5D data*/

	return;
}

void freeDataProperty(ZC_DataProperty* dataProperty)
{
	if(dataProperty->varName!=NULL)
		free(dataProperty->varName);
	if(dataProperty->autocorr!=NULL)
		free(dataProperty->autocorr);
	if(dataProperty->autocorr3D!=NULL)
		free(dataProperty->autocorr3D);
	if(dataProperty->fftCoeff!=NULL)
		free(dataProperty->fftCoeff);
	if(dataProperty->lap!=NULL)
		free(dataProperty->lap);
	free(dataProperty);
}

ZC_DataProperty* ZC_constructDataProperty(char* varName, int dataType, size_t r5, size_t r4, size_t r3, size_t r2, size_t r1, 
size_t numOfElem, double minValue, double maxValue, double valueRange, double avgValue, 
double entropy, double* autocorr, complex* fftCoeff)
{
	ZC_DataProperty* this = (ZC_DataProperty*)malloc(sizeof(ZC_DataProperty));
	this->varName = (char*)malloc(strlen(varName)+1);
	strcpy(this->varName,varName);
	this->dataType = dataType;
	this->r5 = r5;
	this->r4 = r4;
	this->r3 = r3;
	this->r2 = r2;
	this->r1 = r1;
	this->numOfElem = numOfElem;
	this->minValue = minValue;
	this->maxValue = maxValue;
	this->valueRange = valueRange;
	this->avgValue = avgValue;
	this->entropy = entropy;
	this->autocorr = autocorr;
	this->fftCoeff = fftCoeff;
	return this;
}

complex* ZC_computeFFT(void* data, size_t n, int dataType)
{
	size_t i;
	complex *fftCoeff = (complex*)malloc(n*sizeof(complex));
    complex *scratch  = (complex*)malloc(n*sizeof(complex));

	if(dataType==ZC_FLOAT)
	{
		float* data_ = (float*)data;
		for (i = 0; i < n; i++)
		{
			fftCoeff[i].Re = data_[i];
			fftCoeff[i].Im = 0;
		}		
	}
	else if(dataType==ZC_DOUBLE)
	{
		double* data_ = (double*)data;
		for (i = 0; i < n; i++)
		{
			fftCoeff[i].Re = data_[i];
			fftCoeff[i].Im = 0;
		}
	}
	else
	{
		printf("Error: Wrong data type!\n");
		exit(0);
	}

	fft(fftCoeff, n, scratch);
    for (i = 0; i < n; i++)
    {
        fftCoeff[i].Amp = sqrt(fftCoeff[i].Re*fftCoeff[i].Re + fftCoeff[i].Im*fftCoeff[i].Im);
    }
    
    free(scratch);
    
	return fftCoeff;
}

ZC_DataProperty* ZC_genProperties(char* varName, int dataType, void *oriData, size_t r5, size_t r4, size_t r3, size_t r2, size_t r1)
{
	ZC_DataProperty* property = NULL;
	size_t numOfElem = ZC_computeDataLength(r5,r4,r3,r2,r1);
	if(dataType==ZC_FLOAT)
	{
		float* data = (float*)oriData;
#ifdef HAVE_MPI		
		if(executionMode==ZC_OFFLINE)
			property = ZC_genProperties_float(varName, data, numOfElem, r5, r4, r3, r2, r1);
		else
			property = ZC_genProperties_float_online(varName, data, numOfElem, r5, r4, r3, r2, r1);
#else
		property = ZC_genProperties_float(varName, data, numOfElem, r5, r4, r3, r2, r1);
#endif
	}
	else if(dataType==ZC_DOUBLE)
	{
		double* data = (double*)oriData;
#ifdef HAVE_MPI
		if(executionMode==ZC_OFFLINE)
			property = ZC_genProperties_double(varName, data, numOfElem, r5, r4, r3, r2, r1);
		else
			property = ZC_genProperties_double_online(varName, data, numOfElem, r5, r4, r3, r2, r1);
#else
		property = ZC_genProperties_double(varName, data, numOfElem, r5, r4, r3, r2, r1);
#endif
	}
	else
	{
		printf("Error: dataType is wrong!\n");
		exit(0);
	}
	if(property!=NULL)
	{
		property->r5 = r5;
		property->r4 = r4;
		property->r3 = r3;
		property->r2 = r2;
		property->r1 = r1;
	}

	return property;
}

void ZC_printDataProperty(ZC_DataProperty* property)
{
	printf("number of elements: %zu\n", property->numOfElem);
	printf("min value: %f\n", property->minValue);
	printf("avg value: %f\n", property->avgValue);
	printf("max value: %f\n", property->maxValue);
	printf("value range: %f\n", property->valueRange);
	printf("entropy: %f\n", property->entropy);
	//printf("(property->autocorr)[1]=%f\n", (property->autocorr)[1]);
	//printf("(property->autocorr)[20]=%f\n", (property->autocorr)[20]);
	//printf("(property->autocorr)[90]=%f\n", (property->autocorr)[90]);
}

char** constructDataPropertyString(ZC_DataProperty* property)
{
	char** s = (char**)malloc(15*sizeof(char*));
	s[0] = (char*)malloc(100*sizeof(char));
	sprintf(s[0], "[PROPERTY]\n");
	
	s[1] = (char*)malloc(100*sizeof(char));
	sprintf(s[1], "varName = %s\n", property->varName);
	s[2] = (char*)malloc(100*sizeof(char));
	sprintf(s[2], "dataType = %d\n", property->dataType);	
	s[3] = (char*)malloc(100*sizeof(char));
	sprintf(s[3], "r5 = %zu\n", property->r5);
	s[4] = (char*)malloc(100*sizeof(char));
	sprintf(s[4], "r4 = %zu\n", property->r4);
	s[5] = (char*)malloc(100*sizeof(char));
	sprintf(s[5], "r3 = %zu\n", property->r3);
	s[6] = (char*)malloc(100*sizeof(char));
	sprintf(s[6], "r2 = %zu\n", property->r2);
	s[7] = (char*)malloc(100*sizeof(char));
	sprintf(s[7], "r1 = %zu\n", property->r1);	
	s[8] = (char*)malloc(100*sizeof(char));
	sprintf(s[8], "numOfElem = %zu\n", property->numOfElem);
	s[9] = (char*)malloc(100*sizeof(char));
	sprintf(s[9], "minValue = %.10G\n", property->minValue);
	s[10] = (char*)malloc(100*sizeof(char));
	sprintf(s[10], "maxValue = %.10G\n", property->maxValue);
	s[11] = (char*)malloc(100*sizeof(char));
	sprintf(s[11], "valueRange = %.10G\n", property->valueRange);
	s[12] = (char*)malloc(100*sizeof(char));
	sprintf(s[12], "avgValue = %.10G\n", property->avgValue);
	s[13] = (char*)malloc(100*sizeof(char));
	sprintf(s[13], "entropy = %.10G\n", property->entropy);
	s[14] = (char*)malloc(100*sizeof(char));
	if(property->autocorr!=NULL)
		sprintf(s[14], "autocorr = %.10G\n", (property->autocorr)[1]);
	else 
		strcpy(s[14], "autocorr = -\n");
	return s;
}

void ZC_writeFFTResults(char* varName, complex* fftCoeff, char* tgtWorkspaceDir)
{
	size_t i;
	char tgtFilePath[ZC_BUFS] = {0};
	if(fftCoeff!=NULL)
	{
		//write coefficients
		char *ss[FFT_SIZE+1];
		ss[0] = (char*)malloc(sizeof(char)*50);
		sprintf(ss[0], "#Frequency Real Imag\n");

		for(i=0;i<FFT_SIZE;i++)
		{
			ss[i+1] = (char*)malloc(sizeof(char)*ZC_BUFS);
			sprintf(ss[i+1], "%zu/%d %f %f\n", i, FFT_SIZE, fftCoeff[i].Re, fftCoeff[i].Im);
		}
		memset(tgtFilePath, 0, ZC_BUFS);
		sprintf(tgtFilePath, "%s/%s.fft", tgtWorkspaceDir, varName);
		ZC_writeStrings(FFT_SIZE+1, ss, tgtFilePath);

		//write amplitudes
		sprintf(ss[0], "#Frequency Amplitude\n");
		for(i=0;i<FFT_SIZE;i++)
		{
//			double Re = fftCoeff[i].Re;
//			double Im = fftCoeff[i].Im;
//			double amplitude = sqrt(Re*Re+Im*Im);
			sprintf(ss[i+1], "%zu/%d %f\n", i, FFT_SIZE, fftCoeff[i].Amp);
		}
		memset(tgtFilePath, 0, ZC_BUFS);
		sprintf(tgtFilePath, "%s/%s.fft.amp", tgtWorkspaceDir, varName);
		ZC_writeStrings(FFT_SIZE+1, ss, tgtFilePath);

		for(i=0;i<=FFT_SIZE;i++)
			free(ss[i]);
	}
}

void ZC_writeDataProperty(ZC_DataProperty* property, char* tgtWorkspaceDir)
{
	char** s = constructDataPropertyString(property);
	
	DIR *dir = opendir(tgtWorkspaceDir);
	if(dir==NULL)
		mkdir(tgtWorkspaceDir,0775);

	char tgtFilePath[ZC_BUFS];
	sprintf(tgtFilePath, "%s/%s.prop", tgtWorkspaceDir, property->varName); 
	ZC_writeStrings(15, s, tgtFilePath);
	size_t i;
	for(i=0;i<15;i++)
		free(s[i]);
	free(s);
	/*write the fft coefficients and amplitudes*/
	ZC_writeFFTResults(property->varName, property->fftCoeff, tgtWorkspaceDir);
	/*write auto-correlation coefficients*/
	if(property->autocorr!=NULL)
	{
		char *autocorr[AUTOCORR_SIZE+2];
		autocorr[0] = (char*)malloc(sizeof(char)*ZC_BUFS);
		sprintf(autocorr[0], "- \"\"\n");
		for (i = 1; i < AUTOCORR_SIZE+2; i++)
		{
			autocorr[i] = (char*)malloc(sizeof(char)*ZC_BUFS);
			sprintf(autocorr[i], "%zu %.10G\n", i-1, (property->autocorr)[i-1]);
		}
		memset(tgtFilePath, 0, ZC_BUFS);
		sprintf(tgtFilePath, "%s/%s.autocorr", tgtWorkspaceDir, property->varName);
		ZC_writeStrings(AUTOCORR_SIZE, autocorr, tgtFilePath);
		for (i = 0; i < AUTOCORR_SIZE+2; i++)
			free(autocorr[i]);
	}
	/*write 3d auto-correlation results*/
	if(property->autocorr3D!=NULL)
	{
		memset(tgtFilePath, 0, ZC_BUFS);
		sprintf(tgtFilePath, "%s/%s.ac3d", tgtWorkspaceDir, property->varName);
		if(property->dataType==ZC_FLOAT)
			ZC_writeFloatData_inBytes((float*)(property->autocorr3D), property->numOfElem, tgtFilePath);
		else
			ZC_writeDoubleData_inBytes((double*)(property->autocorr3D), property->numOfElem, tgtFilePath);	
	}

	/*write Laplacian*/
	if(property->lap!=NULL)
	{
		char **lap = (char**)malloc(property->numOfElem*sizeof(char*));

		for (i = 0; i < property->numOfElem; i++)
		{
			lap[i] = (char*)malloc(sizeof(char)*50);
			sprintf(lap[i], "%f\n", (property->lap)[i]);
		}
		memset(tgtFilePath, 0, ZC_BUFS);
		sprintf(tgtFilePath, "%s/%s.lap", tgtWorkspaceDir, property->varName);
		ZC_writeStrings(property->numOfElem, lap, tgtFilePath);
		for (i = 0; i < property->numOfElem; i++)
			free(lap[i]);
		free(lap);
	}
	if(dir!=NULL)
		closedir(dir);
}

ZC_DataProperty* ZC_loadDataProperty(char* propResultFile)
{
	/*TODO put the information into the hashtable, named ecPropertyTable*/
	dictionary *ini;
	char* par;
    /*printf("[ZC] Reading propResultFile (%s) ...\n", propResultFile);*/
    if (access(propResultFile, F_OK) != 0)
    {
        printf("[ZC] propResultFile: %s NOT accessible.\n", propResultFile);
        exit(1);
    }
    ini = iniparser_load(propResultFile);
    if (ini == NULL)
    {
        printf("[ZC] Iniparser failed to parse the conf. file: %s.\n", propResultFile);
        exit(1);
    }
	
	par = iniparser_getstring(ini, "PROPERTY:varName", NULL);
	char* var = (char*)malloc(ZC_BUFS);
	snprintf(var, ZC_BUFS,"%s", par);
	
	int dataType = (int)iniparser_getint(ini, "PROPERTY:dataType", 0);
	size_t r5 = iniparser_getlint(ini, "PROPERTY:r5", 0);
	size_t r4 = iniparser_getlint(ini, "PROPERTY:r4", 0);
	size_t r3 = iniparser_getlint(ini, "PROPERTY:r3", 0);
	size_t r2 = iniparser_getlint(ini, "PROPERTY:r2", 0);
	size_t r1 = iniparser_getlint(ini, "PROPERTY:r1", 0);
	size_t numOfElem = (int)iniparser_getint(ini, "PROPERTY:numOfElem", 0);
	double minValue = (double)iniparser_getdouble(ini, "PROPERTY:minValue", 0);
	double maxValue = (double)iniparser_getdouble(ini, "PROPERTY:maxValue", 0);
	double valueRange = (double)iniparser_getdouble(ini, "PROPERTY:valueRange", 0);
	double avgValue = (double)iniparser_getdouble(ini, "PROPERTY:avgValue", 0);
	double entropy = (double)iniparser_getdouble(ini, "PROPERTY:entropy", 0);
	/*double autocorr = (double)iniparser_getdouble(ini, "PROPERTY:autocorr", 0); //1-step*/
	/*TODO: Read more autocorr. coefficients (Read file) and put them in autocorr_array*/
	double* autocorr_array = NULL;
	
	/*TODO: Read fft coefficients... (Read file)*/
	complex* fftCoeff = NULL;
	
	ZC_DataProperty* property = ZC_constructDataProperty(var, dataType, r5, r4, r3, r2, r1, numOfElem, minValue, maxValue, 
	valueRange, avgValue, entropy, autocorr_array, fftCoeff);
	return property;
}
