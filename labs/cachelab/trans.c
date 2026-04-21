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
static void transpose32(int A[32][32], int B[32][32]);
static void transpose64(int A[64][64], int B[64][64]);
static void transposeGeneric(int M, int N, int A[N][M], int B[M][N]);

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
    /*
     * Use a layout-specific strategy for the three matrix sizes that the
     * driver cares about. This keeps correctness simple while minimizing
     * misses on the 1KB direct-mapped cache used by Cache Lab.
     */
    if (M == 32 && N == 32) {
        transpose32((int (*)[32])A, (int (*)[32])B);
        return;
    }

    if (M == 64 && N == 64) {
        transpose64((int (*)[64])A, (int (*)[64])B);
        return;
    }

    transposeGeneric(M, N, A, B);
}

/*
 * 32x32 case:
 * 8x8 blocking matches the cache block size (8 ints per cache line).
 * Diagonal elements are delayed until the end of the row to avoid
 * conflict misses between A and B when working on diagonal blocks.
 */
static void transpose32(int A[32][32], int B[32][32])
{
    int i, j, k, l, diag, tmp;

    for (i = 0; i < 32; i += 8) {
        for (j = 0; j < 32; j += 8) {
            for (k = i; k < i + 8; k++) {
                for (l = j; l < j + 8; l++) {
                    if (k != l) {
                        B[l][k] = A[k][l];
                    } else {
                        tmp = A[k][l];
                        diag = k;
                    }
                }

                if (i == j) {
                    B[diag][diag] = tmp;
                }
            }
        }
    }
}

/*
 * 64x64 case:
 * A naive 8x8 transpose conflicts badly in a direct-mapped cache.
 * Split each 8x8 block into four 4x4 quadrants and use the upper-right
 * corner of B as temporary storage before swapping sub-blocks into place.
 *
 * For one 8x8 block of A:
 *   [ A11 | A12 ]
 *   [ A21 | A22 ]
 *
 * we want B to end up as:
 *   [ A11^T | A21^T ]
 *   [ A12^T | A22^T ]
 *
 * The tricky part is that A and B easily map to the same cache sets, so
 * directly transposing the whole 8x8 block causes heavy conflict misses.
 * The implementation below therefore performs the transpose in three stages:
 * 1. Read the top four rows of the 8x8 block and place A11^T directly.
 *    At the same time, temporarily park A12^T in the wrong half of B.
 * 2. Read A21, move the parked A12^T values into their final positions,
 *    and place A21^T where it belongs.
 * 3. Finish the bottom-right 4x4 block A22^T.
 */
static void transpose64(int A[64][64], int B[64][64])
{
    int i, j, k, l;
    int a0, a1, a2, a3, a4, a5, a6, a7;

    for (i = 0; i < 64; i += 8) {
        for (j = 0; j < 64; j += 8) {
            /*
             * Stage 1:
             * Read the top half of the 8x8 block row by row.
             * - a0..a3 belong to A11 and are written directly to B's top-left.
             * - a4..a7 belong to A12 and are temporarily written into B's
             *   top-right area. They will be moved to their true destination
             *   in Stage 2 after we have brought in the A21 values.
             */
            for (k = i; k < i + 4; k++) {
                a0 = A[k][j];
                a1 = A[k][j + 1];
                a2 = A[k][j + 2];
                a3 = A[k][j + 3];
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];

                B[j][k] = a0;
                B[j + 1][k] = a1;
                B[j + 2][k] = a2;
                B[j + 3][k] = a3;
                B[j][k + 4] = a4;
                B[j + 1][k + 4] = a5;
                B[j + 2][k + 4] = a6;
                B[j + 3][k + 4] = a7;
            }

            /*
             * Stage 2:
             * Walk across the left half of the bottom four rows (A21).
             * For each column l:
             * - read one column of A21 into a0..a3
             * - read the temporarily stored A12^T values from B into a4..a7
             * Then:
             * - write A21^T into the top-right quadrant of B
             * - move the parked A12^T values into the bottom-left quadrant
             *
             * This "swap through B" avoids re-reading the upper rows of A and
             * reduces the direct-mapped conflicts that appear in a naive 8x8
             * transpose.
             */
            for (l = 0; l < 4; l++) {
                a0 = A[i + 4][j + l];
                a1 = A[i + 5][j + l];
                a2 = A[i + 6][j + l];
                a3 = A[i + 7][j + l];
                a4 = B[j + l][i + 4];
                a5 = B[j + l][i + 5];
                a6 = B[j + l][i + 6];
                a7 = B[j + l][i + 7];

                B[j + l][i + 4] = a0;
                B[j + l][i + 5] = a1;
                B[j + l][i + 6] = a2;
                B[j + l][i + 7] = a3;
                B[j + l + 4][i] = a4;
                B[j + l + 4][i + 1] = a5;
                B[j + l + 4][i + 2] = a6;
                B[j + l + 4][i + 3] = a7;
            }

            /*
             * Stage 3:
             * The bottom-right 4x4 block A22 has not been written yet.
             * At this point the other three quadrants are already in place,
             * so we can finish the remaining 4x4 transpose directly.
             */
            for (k = i + 4; k < i + 8; k++) {
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];

                B[j + 4][k] = a4;
                B[j + 5][k] = a5;
                B[j + 6][k] = a6;
                B[j + 7][k] = a7;
            }
        }
    }
}

/*
 * 61x67 case:
 * Use a moderate block size so we still get locality benefits without
 * needing any special-case logic for edge tiles.
 */
static void transposeGeneric(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l;

    for (i = 0; i < N; i += 16) {
        for (j = 0; j < M; j += 16) {
            for (k = i; k < N && k < i + 16; k++) {
                for (l = j; l < M && l < j + 16; l++) {
                    B[l][k] = A[k][l];
                }
            }
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
