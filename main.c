#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_NUMBER_LENGTH 10
#define FILE_NAME_LENGTTH 20

typedef struct matrix
{
    int row;
    int column;
    int **mat;
} Matrix_t;

typedef struct argumentsPerRow
{
    int *row;
    Matrix_t *transposed;
    int length;
    Matrix_t *res;
    int ind;
} ArgumentsPerRow_t;

typedef struct argumentsPerElement
{
    int *row;
    int *column;
    int length;
    Matrix_t *res;
    int i;
    int j;
} ArgumentsPerElement_t;

typedef enum threading
{
    PER_MATRIX,
    PER_ROW,
    PER_ELEMENT
} En_threadingType_t;

int strToInt(char *str); // convert number as string to integer
void readInputFiles(Matrix_t *M, char name);
Matrix_t multiplyMatrix(Matrix_t *x, Matrix_t *y, En_threadingType_t type);
int multiplyRowAndColumn(int *row, int *column, int length);
Matrix_t transpose(Matrix_t *m);
void writeOutputFiles(Matrix_t *m, char name, En_threadingType_t type);
void *perRowMultiply(void *arg);
void *perElementMultiply(void *arg);

int main(int argc, char *argv[])
{
    /* Handling The arguments */
    char a = 'a';
    char b = 'b';
    char c = 'c';
    if (argc > 1)
    {
        a = *(argv[1]);
        b = *(argv[2]);
        c = *(argv[3]);
    }
    int i, j;
    /* Read the input matrices from the txt files */
    Matrix_t A, B, C;
    readInputFiles(&A, a);
    readInputFiles(&B, b);

    /* Variables for calculating time */
    struct timeval stop, start;

    /////////////////////Per Matrix/////////////////////

    /* Start The timer */
    printf("/////////////////Per Matrix Time/////////////////\n");
    gettimeofday(&start, NULL);
    C = multiplyMatrix(&A, &B, PER_MATRIX);
    gettimeofday(&stop, NULL);
    /* Stop the timer */

    /* Print The time */
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    writeOutputFiles(&C, c, PER_MATRIX);
    for (i = 0; i < C.row; i++)
        free(C.mat[i]);
    free(C.mat);

    ///////////////////////Per ROW///////////////////////

    /* Start The timer */
    printf("/////////////////Per ROW Time/////////////////\n");
    gettimeofday(&start, NULL);
    C = multiplyMatrix(&A, &B, PER_ROW);
    gettimeofday(&stop, NULL);
    /* Stop the timer */

    /* print the time */
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    writeOutputFiles(&C, c, PER_ROW);
    for (i = 0; i < C.row; i++)
        free(C.mat[i]);
    free(C.mat);

    /////////////////////Per ELEMENT/////////////////////

    /* Start The timer */
    printf("///////////////Per ELEMENT Time///////////////\n");
    gettimeofday(&start, NULL);
    C = multiplyMatrix(&A, &B, PER_ELEMENT);
    gettimeofday(&stop, NULL);
    /* Stop the timer */

    /* print the time */
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    writeOutputFiles(&C, c, PER_ELEMENT);
    for (i = 0; i < C.row; i++)
        free(C.mat[i]);
    free(C.mat);


    /* Free Allocated Space */
    for (i = 0; i < A.row; i++)
        free(A.mat[i]);
    free(A.mat);
    for (i = 0; i < B.row; i++)
        free(B.mat[i]);
    free(B.mat);
}

int strToInt(char *str)
{
    int res = 0;
    for (int i = (int)strlen(str) - 1; i >= 0; i--)
        res += (str[i] - '0') * ((int)pow((double)10, (double)(strlen(str) - 1 - i)));
    return res;
}

void readInputFiles(Matrix_t *M, char name)
{
    char fileName[FILE_NAME_LENGTTH] = "x.txt";
    fileName[0] = name;
    FILE *fptr = fopen(fileName, "r");
    char line[50];
    char num[MAX_NUMBER_LENGTH];
    int i, j;
    char flag = 0;
    /* Read First Line */
    fgets(line, 50, fptr);
    for (i = 0; i < 50; i++)
    {
        if (line[i] == '=')
        {
            j = 0;
            flag++;
            i++;
            while ((line[i] != ' ') && (line[i] != '\t') && (line[i] != '\n'))
            {
                num[j] = line[i];
                i++;
                j++;
            }
            num[j] = '\0';
            if (flag == 1)
                M->row = strToInt(num);
            else
                M->column = strToInt(num);
        }
    }
    /* Allocate Space for the Matrix */
    M->mat = (int **)malloc(sizeof(int *) * M->row);
    for (i = 0; i < M->row; i++)
        M->mat[i] = (int *)malloc(sizeof(int) * M->column);
    
    /* Read Matrix elements and write them in the matrix variable */
    int k, l;
    for (i = 0; i < M->row; i++)
    {
        fgets(line, 50, fptr);
        k = 0;
        for (j = 0; j < M->column; j++)
        {
            l = 0;
            while (line[k] != ' ' && line[k] != '\n' && line[k] != '\0' && (line[k] != '\t'))
            {
                num[l] = line[k];
                l++;
                k++;
            }
            k++;
            num[l] = '\0';
            M->mat[i][j] = strToInt(num);
        }
    }
    fclose(fptr);
}

Matrix_t multiplyMatrix(Matrix_t *x, Matrix_t *y, En_threadingType_t type)
{
    int i, j;
    if (x->column != y->row)
    {
        printf("Error This two matrices cannot be multiplied\n");
        exit(0);
    }

    /* Result matrix from multiplicatoion */
    Matrix_t res;
    res.row = x->row;
    res.column = y->column;
    res.mat = (int **)malloc(sizeof(int *) * res.row);
    for (i = 0; i < res.row; i++)
        res.mat[i] = (int *)malloc(sizeof(int) * res.column);

    // Transpose the second matrix
    Matrix_t transposed = transpose(y);
    /* Multiplication of each row with each column */
    switch (type)
    {
    case PER_MATRIX:
        printf("Number of threads created: 1\n");
        for (i = 0; i < res.row; i++)
        {
            for (j = 0; j < res.column; j++)
                res.mat[i][j] = multiplyRowAndColumn(x->mat[i], transposed.mat[j], x->column);
        }
        break;
    case PER_ROW:
        pthread_t *t_row = (pthread_t *)malloc(sizeof(pthread_t) * res.row);
        printf("Number of threads created: %d\n", res.row);
        ArgumentsPerRow_t **argPtrs = (ArgumentsPerRow_t **)malloc(sizeof(ArgumentsPerRow_t *) * res.row);
        for (i = 0; i < res.row; i++)
        {
            argPtrs[i] = (ArgumentsPerRow_t *)malloc(sizeof(ArgumentsPerRow_t));
            argPtrs[i]->length = x->column;
            argPtrs[i]->transposed = &transposed;
            argPtrs[i]->res = &res;
        }
        for (i = 0; i < res.row; i++)
        {
            argPtrs[i]->row = x->mat[i];
            argPtrs[i]->ind = i;
            pthread_create(&t_row[i], NULL, &perRowMultiply, (void *)argPtrs[i]);
        }
        for (i = 0; i < res.row; i++)
            pthread_join(t_row[i], NULL);
        
        /* Free Allocated Space */
        for(i=0; i<res.row; i++)
            free(argPtrs[i]);
        free(argPtrs);
        free(t_row);
        break;
    case PER_ELEMENT:
        pthread_t **t_element = (pthread_t **)malloc(sizeof(pthread_t*) * res.row);
        for(i=0; i<res.row; i++)
            t_element[i] = (pthread_t *)malloc(sizeof(pthread_t) * res.column);
        printf("Number of threads created: %d\n", res.row*res.column);
        ArgumentsPerElement_t ***perElementArgPtrs = (ArgumentsPerElement_t ***)malloc(sizeof(ArgumentsPerElement_t **) * res.row);
        for(i=0; i<res.row; i++)
            perElementArgPtrs[i] = (ArgumentsPerElement_t **)malloc(sizeof(ArgumentsPerElement_t *) * res.column);

        for (i = 0; i < res.row; i++)
        {
            for (j = 0; j < res.column; j++){
                perElementArgPtrs[i][j] = (ArgumentsPerElement_t *)malloc(sizeof(ArgumentsPerElement_t));
                perElementArgPtrs[i][j]->i=i;
                perElementArgPtrs[i][j]->j=j;
                perElementArgPtrs[i][j]->row = x->mat[i];
                perElementArgPtrs[i][j]->column = transposed.mat[j];
                perElementArgPtrs[i][j]->length = x->column;
                perElementArgPtrs[i][j]->res = &res;
                pthread_create(&t_element[i][j], NULL, &perElementMultiply, (void *)perElementArgPtrs[i][j]);
            }
        }
        for (i = 0; i < res.row; i++)
        {
            for (j = 0; j < res.column; j++)
                pthread_join(t_element[i][j], NULL);
        }
        /* Free Allocated Space */
        for (i = 0; i < res.row; i++)
        {
            for (j = 0; j < res.column; j++)
                free(perElementArgPtrs[i][j]);
            free(perElementArgPtrs[i]);
            free(t_element[i]);
        }
        free(perElementArgPtrs);
        free(t_element);
        break;
    default:
        break;
    }
    /* Free Allocated Space */
    for (int i = 0; i < transposed.row; i++)
        free(transposed.mat[i]);
    free(transposed.mat);
    return res;
}

void *perRowMultiply(void *arg)
{
    ArgumentsPerRow_t *argPtr;
    argPtr = (ArgumentsPerRow_t *)arg;
    for (int j = 0; j < argPtr->res->column; j++)
        argPtr->res->mat[argPtr->ind][j] = multiplyRowAndColumn(argPtr->row, argPtr->transposed->mat[j], argPtr->length);
}

void *perElementMultiply(void *arg)
{
    ArgumentsPerElement_t *argPtr;
    argPtr = (ArgumentsPerElement_t *)arg;
    argPtr->res->mat[argPtr->i][argPtr->j] = multiplyRowAndColumn(argPtr->row, argPtr->column, argPtr->length);
}

int multiplyRowAndColumn(int *row, int *column, int length)
{
    int res = 0;
    for (int i = 0; i < length; i++)
        res += row[i] * column[i];
    return res;
}

Matrix_t transpose(Matrix_t *m)
{
    int i, j;
    /* Result Matrix After Transpose */
    Matrix_t transposed;
    transposed.column = m->row;
    transposed.row = m->column;
    transposed.mat = (int **)malloc(sizeof(int *) * transposed.row);
    for (i = 0; i < transposed.row; i++)
        transposed.mat[i] = (int *)malloc(sizeof(int) * transposed.column);

    for (i = 0; i < transposed.row; i++)
    {
        for (j = 0; j < transposed.column; j++)
            transposed.mat[i][j] = m->mat[j][i];
    }
    return transposed;
}

void writeOutputFiles(Matrix_t *m, char name, En_threadingType_t type)
{
    int i, j;
    FILE *fptr;
    char fileName[FILE_NAME_LENGTTH];
    switch (type)
    {
    case PER_MATRIX:
        strcpy(fileName, "c_per_matrix.txt");
        fileName[0] = name;
        fptr = fopen(fileName, "w");
        fputs("Method: A thread per matrix\n", fptr);
        break;
    case PER_ROW:
        strcpy(fileName, "c_per_row.txt");
        fileName[0] = name;
        fptr = fopen(fileName, "w");
        fputs("Method: A thread per row\n", fptr);
        break;
    case PER_ELEMENT:
        strcpy(fileName, "c_per_element.txt");
        fileName[0] = name;
        fptr = fopen(fileName, "w");
        fputs("Method: A thread per element\n", fptr);
        break;
    }
    char num[MAX_NUMBER_LENGTH];
    fputs("row=", fptr);
    sprintf(num, "%d", m->row);
    fputs(num, fptr);
    fputs(" column=", fptr);
    sprintf(num, "%d", m->column);
    fputs(num, fptr);
    fputs("\n", fptr);
    for (i = 0; i < m->row; i++)
    {
        for (j = 0; j < m->column; j++)
        {
            sprintf(num, "%d", m->mat[i][j]);
            fputs(num, fptr);
            fputs(" ", fptr);
        }
        fputs("\n", fptr);
    }
    fclose(fptr);
}
