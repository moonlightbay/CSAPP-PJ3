/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    //case1:M=N=32,此时A矩阵每行需要4个cache行，意味着可以存8行
    if (M == 32 && N == 32){
        int i, j, k;     //循环所需变量
        int temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;   //临时变量
        for (i = 0; i < N; i += 8){           //8*8分块处理
            for (j = 0; j < M; j += 8){
                for (k = i; k < i + 8; k++){
                    temp0 = A[k][j];
                    temp1 = A[k][j + 1];
                    temp2 = A[k][j + 2];
                    temp3 = A[k][j + 3];
                    temp4 = A[k][j + 4];
                    temp5 = A[k][j + 5];
                    temp6 = A[k][j + 6];
                    temp7 = A[k][j + 7];
                    B[j][k] = temp0;
                    B[j + 1][k] = temp1;
                    B[j + 2][k] = temp2;
                    B[j + 3][k] = temp3;
                    B[j + 4][k] = temp4;
                    B[j + 5][k] = temp5;
                    B[j + 6][k] = temp6;
                    B[j + 7][k] = temp7;
                }
            }
        }
    }
    //case2:M=N=64,此时A矩阵每行需要8个cache行，意味着只能存4行
    else if (M == 64 && N == 64)
    {
        int i, j, k, l;
        int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
        for (i = 0; i < N; i += 8)
            for (j = 0; j < M; j += 8)               //8*8分块，然后分成4个4*4小块
            {
                for (k = i; k < i + 4; ++k)          //读四行4*8,转置一个4*4（左上）,转置暂存一个4*4，以全部利用读取的 cache 行
                {
                    temp1 = A[k][j]; temp2 = A[k][j+1]; temp3 = A[k][j+2]; temp4 = A[k][j+3];
                    temp5 = A[k][j+4]; temp6 = A[k][j+5]; temp7 = A[k][j+6]; temp8 = A[k][j+7];
                    
                    B[j][k] = temp1; B[j+1][k] = temp2; B[j+2][k] = temp3; B[j+3][k] = temp4;
                    B[j][k+4] = temp5; B[j+1][k+4] = temp6; B[j+2][k+4] = temp7; B[j+3][k+4] = temp8;
                }

                for (l = j; l < j + 4; ++l)          //转移一个4*4（到左下），转置一个4*4（到右上）
                {
                    temp1 = A[i+4][l]; temp2 = A[i+5][l]; temp3 = A[i+6][l]; temp4 = A[i+7][l];
                    temp5 = B[l][i+4]; temp6 = B[l][i+5]; temp7 = B[l][i+6]; temp8 = B[l][i+7];
                    
                    B[l][i+4] = temp1; B[l][i+5] = temp2; B[l][i+6] = temp3; B[l][i+7] = temp4;
                    B[l+4][i] = temp5; B[l+4][i+1] = temp6; B[l+4][i+2] = temp7; B[l+4][i+3] = temp8;
                }
                for (k = i + 4; k < i + 8; ++k)    //转置一个4*4（右下）
                {
                    temp1 = A[k][j+4]; temp2 = A[k][j+5]; temp3 = A[k][j+6]; temp4 = A[k][j+7];
                    B[j+4][k] = temp1; B[j+5][k] = temp2; B[j+6][k] = temp3; B[j+7][k] = temp4;
                }
            }
    }
    
    //case3：M=61,N=67
    else if(M == 61 && N == 67)
    {
        int i, j, temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
        int k = N / 8 * 8;               //余下的列数为5
        int l = M / 8 * 8;               //余下的行数为3
        for (j = 0; j < l; j += 8)       //8*8分块，转置所有完整的8*8块
            for (i = 0; i < k; ++i)
            {
                temp0 = A[i][j];
                temp1 = A[i][j+1];
                temp2 = A[i][j+2];
                temp3 = A[i][j+3];
                temp4 = A[i][j+4];
                temp5 = A[i][j+5];
                temp6 = A[i][j+6];
                temp7 = A[i][j+7];
                
                B[j][i] = temp0;
                B[j+1][i] = temp1;
                B[j+2][i] = temp2;
                B[j+3][i] = temp3;
                B[j+4][i] = temp4;
                B[j+5][i] = temp5;
                B[j+6][i] = temp6;
                B[j+7][i] = temp7;
            }
        for (i = k; i < N; ++i)       //转置剩余的不完整的8*8块(右下角)
            for (j = l; j < M; ++j)
            {
                temp0 = A[i][j];
                B[j][i] = temp0;
            }
        for (i = 0; i < N; ++i)      //转置剩余的不完整的8*8块(右方)
            for (j = l; j < M; ++j)
            {
                temp0 = A[i][j];
                B[j][i] = temp0;
            }
        for (i = k; i < N; ++i)       //转置剩余的不完整的8*8块(下方)
            for (j = 0; j < M; ++j)
            {
                temp0 = A[i][j];
                B[j][i] = temp0;
            }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

