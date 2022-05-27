/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   CEcoEigenfaces1
 * </сводка>
 *
 * <описание>
 *   Данный исходный код описывает реализацию интерфейсов CEcoEigenfaces1
 * </описание>
 *
 * <автор>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 */

#include "IEcoSystem1.h"
#include "IEcoInterfaceBus1.h"
#include "CEcoEigenfaces1.h"
#include "IdEcoString1.h"
#include "IdEcoMemoryManager1.h"
#include <stdio.h>
#include <math.h>

/*
 *
 * <сводка>
 *   Функция QueryInterface
 * </сводка>
 *
 * <описание>
 *   Функция QueryInterface для интерфейса IEcoEigenfaces1
 * </описание>
 *
 */
int16_t CEcoEigenfaces1_QueryInterface(/* in */ struct IEcoEigenfaces1* me, /* in */ const UGUID* riid, /* out */ void** ppv) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;
    int16_t result = -1;

    /* Проверка указателей */
    if (me == 0 || ppv == 0) {
        return result;
    }

    /* Проверка и получение запрошенного интерфейса */
    if ( IsEqualUGUID(riid, &IID_IEcoEigenfaces1) ) {
        *ppv = &pCMe->m_pVTblIEcoEigenfaces1;
        pCMe->m_pVTblIEcoEigenfaces1->AddRef((IEcoEigenfaces1*)pCMe);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoUnknown) ) {
        *ppv = &pCMe->m_pVTblIEcoEigenfaces1;
        pCMe->m_pVTblIEcoEigenfaces1->AddRef((IEcoEigenfaces1*)pCMe);
    }
    else {
        *ppv = 0;
        return -1;
    }
    return 0;
}

/*
 *
 * <сводка>
 *   Функция AddRef
 * </сводка>
 *
 * <описание>
 *   Функция AddRef для интерфейса IEcoEigenfaces1
 * </описание>
 *
 */
uint32_t CEcoEigenfaces1_AddRef(/* in */ struct IEcoEigenfaces1* me) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;

    /* Проверка указателя */
    if (me == 0 ) {
        return -1;
    }

    return ++pCMe->m_cRef;
}

/*
 *
 * <сводка>
 *   Функция Release
 * </сводка>
 *
 * <описание>
 *   Функция Release для интерфейса IEcoEigenfaces1
 * </описание>
 *
 */
uint32_t CEcoEigenfaces1_Release(/* in */ struct IEcoEigenfaces1* me) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;

    /* Проверка указателя */
    if (me == 0 ) {
        return -1;
    }

    /* Уменьшение счетчика ссылок на компонент */
    --pCMe->m_cRef;

    /* В случае обнуления счетчика, освобождение данных экземпляра */
    if ( pCMe->m_cRef == 0 ) {
        deleteCEcoEigenfaces1((IEcoEigenfaces1*)pCMe);
        return 0;
    }
    return pCMe->m_cRef;
}


void calcImageCovMatrix(IEcoMemoryAllocator1* pIMem, float_t* covMatrix, int32_t size, uint8_t** bitmaps, int32_t numberImages) {
    int32_t i;
    int32_t j;
    int32_t n;
    int64_t sum;
    int64_t tmp_int64;
    int64_t pixel_i;
    int64_t pixel_j;

    // Iterate pixels
    // Calculate only element above main diagonal because cov matrix is symmetric
    for (i = 0; i < size; i++) {
        for (j = i; j < size; j++) {
            if (i == j) {
                // Main diagonal -> calculate DX
                // DXi = sum(Xi - MXi) / (n - 1)
                // DXi = sum(Xi) / (n - 1) because MXi = 0
                sum = 0;
                for (n = 0; n < numberImages; n++) {
                    // tmp_int64 = bitmaps[n][i * 3] + (bitmaps[n][i * 3 + 1] << 8) + (bitmaps[n][i * 3 + 2] << 16);
                    tmp_int64 = bitmaps[n][i * 3] + (bitmaps[n][i * 3 + 1] << 8) + (bitmaps[n][i * 3 + 2] << 16);
                    sum += tmp_int64 * tmp_int64;
                }
                covMatrix[i * size + j] = sum / (numberImages - 1);
            }
            else {
                // Calculate Cov
                // Cov(Xi, Xj) = sum((Xi - MXi)(Xj - MXj)) / (n - 1) = sum(Xi * Xj) / (n - 1)
                sum = 0;
                for (n = 0; n < numberImages; n++) {
                    pixel_i = bitmaps[n][i * 3] + (bitmaps[n][i * 3 + 1] << 8) + (bitmaps[n][i * 3 + 2] << 16);
                    pixel_j = bitmaps[n][j * 3] + (bitmaps[n][j * 3 + 1] << 8) + (bitmaps[n][j * 3 + 2] << 16);

                    /*pixel_i = bitmaps[n][i * 3] + bitmaps[n][i * 3 + 1] + bitmaps[n][i * 3 + 2];
                    pixel_j = bitmaps[n][j * 3] + bitmaps[n][j * 3 + 1] + bitmaps[n][j * 3 + 2];*/
                    sum += pixel_i * pixel_j;
                }
                covMatrix[i * size + j] = sum / (numberImages - 1);
            }
        }
    }

    // Copy elements above main diagonal to symmetric elements below main diagonal
    for (i = 0; i < size; i++) {
        for (j = 0; j < i; j++) {
            covMatrix[i * size + j] = covMatrix[j * size + i];
        }
    }
}

float_t scalarVectorProduct(float_t* matrix1, int32_t index_a, float_t* matrix2, int32_t index_b, uint32_t vectorSize, uint32_t vectorNum1, uint32_t vectorNum2) {
    float_t product = 0.0;
    uint32_t i = 0;
    float_t sqrt_sum_a = 0.0;
    float_t sqrt_sum_b = 0.0;
    float_t sum_mult = 0.0;

    for (i = 0; i < vectorSize; i++) {
        sum_mult += matrix1[i * vectorNum1 + index_a] * matrix2[i * vectorNum2 + index_b];
    }

    return sum_mult;
}

float_t scalarVectorProduct_array(float_t* matrix1, int32_t index_a, float_t* vector_b, uint32_t size) {
    float_t product = 0.0;
    uint32_t i = 0;
    float_t sqrt_sum_a = 0.0;
    float_t sqrt_sum_b = 0.0;
    float_t sum_mult = 0.0;

    for (i = 0; i < size; i++) {
        sum_mult += matrix1[i * size + index_a] * vector_b[i];
    }

    return sum_mult;
}

float_t scalarVectorProduct_two_arrays(float_t* vector_a, float_t* vector_b, uint32_t size) {
    float_t product = 0.0;
    uint32_t i = 0;
    float_t sqrt_sum_a = 0.0;
    float_t sqrt_sum_b = 0.0;
    float_t sum_mult = 0.0;

    for (i = 0; i < size; i++) {
        sum_mult += vector_a[i] * vector_b[i];
    }

    return sum_mult;
}

void scalarToVectorProduct_output_as_array(float_t scalar, float_t* matrix, int32_t index, uint32_t vectorLen, uint32_t vectorNum, /* out */ float_t* res_vector) {
    uint32_t i = 0;

    for (i = 0; i < vectorLen; i++) {
        res_vector[i] = matrix[i * vectorNum + index] * scalar;
    }
}

void scalarToVectorProduct_array(float_t scalar, float_t* vector, uint32_t length, /* out */ float_t* resVector) {
    uint32_t i = 0;

    for (i = 0; i < length; i++) {
        resVector[i] = vector[i] * scalar;
    }
}

void vectorProjection(float_t* matrix1, int32_t index_a, float_t* vector_b, uint32_t size, /* out */ float_t* res_vector) {
    float_t scalar = scalarVectorProduct_array(matrix1, index_a, vector_b, size) / scalarVectorProduct_two_arrays(vector_b, vector_b, size);

    scalarToVectorProduct_array(scalar, vector_b, size, res_vector);
}

void vectorProjectionMatrix(float_t* matrix1, int32_t index_a, float_t* matrix2, int32_t index_b, uint32_t size1, uint32_t size2, /* out */ float_t* res_vector) {
    float_t scalar = scalarVectorProduct(matrix1, index_a, matrix2, index_b, size1, size1, size2) / scalarVectorProduct(matrix2, index_b, matrix2, index_b, size1, size2, size2);

    scalarToVectorProduct_output_as_array(scalar, matrix2, index_b, size1, size2, res_vector);
}

float_t normOfVector(float_t* matrix, int32_t index, int32_t size) {
    float_t norm = 0.0;
    uint32_t i = 0;

    for (i = 0; i < size; i++) {
        norm += matrix[i * size + index] * matrix[i * size + index];
    }
    return sqrt(norm);
}

float_t normOfVector_array(float_t* vector, int32_t size) {
    float_t norm = 0.0;
    uint32_t i = 0;

    for (i = 0; i < size; i++) {
        norm += vector[i] * vector[i];
    }
    return sqrt(norm);
}

void transpose(float_t* matrix, uint32_t size) {
    uint32_t i;
    uint32_t j;
    float_t tmp;

    for (i = 0; i < size; i++) {
        for (j = i; j < size; j++) {
            tmp = matrix[i * size + j];
            matrix[i * size + j] = matrix[j * size + i];
            matrix[j * size + i] = tmp;
        }
    }
}

void matricesProduct(float_t* Q, float_t* R, float_t* result, uint32_t size) {
    uint32_t i;
    uint32_t j;
    uint32_t k;
    float_t sum;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            sum = 0;
            for (k = 0; k < size; k++) {
                sum += Q[i * size + k] * R[k * size + j];
            }

            result[i * size + j] = sum;
        }
    }
}

void DecomposeMatrixToColumnVectorsFlattened(float_t* matrix, int32_t size, float_t** resultMatrix) {
    uint32_t i;
    uint32_t j;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            resultMatrix[i][j] = matrix[j * size + i];
        }
    }
}

int32_t writeArrayToFile(float_t* e, int32_t size, char_t* filename) {
    FILE *file = NULL;

    file = fopen(filename, "wb");
    if (file == NULL) {
        return -1;
    }

    // Write count of elements to the beginning of a file
    fwrite(&size, sizeof(int32_t), 1, file);
    fwrite(e, sizeof(float_t), size, file);
    fclose(file);

    return 0;
}

int32_t readArrayFromFile(float_t* ei, char_t* filename) {
    FILE *Read = fopen(filename, "rb");
    int32_t lenRead = 0;

    if (Read == NULL) {
        return -1;
    }

    fread(&lenRead, sizeof(int32_t), 1, Read);
    fread(ei, sizeof(float_t), lenRead, Read);
    fclose(Read);

    return 0;
}

void printMatrixFlattened(float_t* matrix, uint32_t vectorLen, uint32_t vectorNum) {
    uint32_t i;
    uint32_t j;
    for (i = 0; i < vectorLen; i++) {
        for (j = 0; j < vectorNum; j++) {
            printf("%.7f ", matrix[i * vectorNum + j]);
        }
        printf("\n");
    }
}

void printVectorFromMatrixFlattened(float_t* matrix, uint32_t index, uint32_t vectorLen, uint32_t vectorNum) {
    uint32_t i;
    uint32_t j;
    printf("Maxtix[%d]\n", index);
    for (i = 0; i < vectorLen; i++) {
        printf("%.7f ", matrix[i * vectorNum + index]);
    }
    printf("\n");
}

int16_t QRDecompositionGramShmidth(float_t* matrix, int32_t size, IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr, /* out */ float_t* Q, float_t* R) {
    int32_t i;
    int32_t j;
    int32_t k;
    int32_t success;
    float_t* e_matrix;
    int32_t current_quarter_read_number = 0;
    float_t* e;
    float_t* ei;
    float_t* projAcc;
    float_t* proj;
    float_t* ui;
    float_t tmp;
    char_t* filename = 0;
    int32_t MAX_E_VECTORS_COUNT = 500;

    e = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(float_t));
    ei = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(float_t));
    ui = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(float_t));
    proj = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(float_t));
    projAcc = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(float_t));


    e_matrix = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * size * sizeof(float_t));

    if (e == 0 || ei == 0 || ui == 0 || proj == 0 || projAcc == 0 || e_matrix == 0) {
        return -1;
    }

    // Init first vectors U and E
    // u[0] = vectors[0];
    scalarToVectorProduct_output_as_array(1 / normOfVector(matrix, 0, size), matrix, 0, size, size, e);

    // Save e0 to array
    for (i = 0; i < size; i++) {
        e_matrix[i * size + 0] = e[i];
    }

    // printVectorFromMatrixFlattened(e_quarter, 0, size, e_quarter_size);

    // Fill Q and R matrices by values from first e vector
    for (i = 0; i < size; i++) {
        Q[i * size + 0] = e[i];
        R[i * size + 0] = scalarVectorProduct_array(matrix, i, e, size);
    }

    // Calculate the next u and e vectors
    for (i = 1; i < size; i++) {
        if (i % 100 == 0) {
            printf("GS - iter %d\n", i);
        }

        // Zero accumulator
        for (j = 0; j < size; j++) {
            projAcc[j] = 0;
        }


        for (j = 0; j < i; j++) {
            vectorProjectionMatrix(matrix, i, e_matrix, j, size, size, proj);

            for (k = 0; k < size; k++) {
                projAcc[k] += proj[k];
            }
        }

        for (j = 0; j < size; j++) {
            ui[j] = matrix[j * size + i] - projAcc[j];
        }

        // scalarToVectorProduct(1 / normOfVector(ui, size), ui, size, e[i]);
        scalarToVectorProduct_array(1 / normOfVector_array(ui, size), ui, size, e);

        // write calculated e vector to e_quarter
        for (j = 0; j < size; j++) {
            e_matrix[j * size + (i % size)] = e[j];
        }

        // Fill Q and R matrices by values from a newly calculated e vector
        for (k = 0; k < size; k++) {
            Q[k * size + i] = e[k];
            if (k >= i) {
                R[k * size + i] = scalarVectorProduct_array(matrix, k, e, size);
            }
            else {
                R[k * size + i] = 0;
            }
        }
    }

    transpose(R, size);


    // Free u and e matrices
    pIMem->pVTbl->Free(pIMem, ui);
    pIMem->pVTbl->Free(pIMem, e);
    pIMem->pVTbl->Free(pIMem, proj);
    pIMem->pVTbl->Free(pIMem, projAcc);
    return 0;
}

void printMatrix(float_t** matrix, uint32_t size) {
    uint32_t i;
    uint32_t j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            printf("%.4f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void DecomposeMatrixToColumnVectors(float_t* matrix, int32_t size, float_t** resultMatrix) {
    uint32_t i;
    uint32_t j;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            resultMatrix[i][j] = matrix[j * size + i];
        }
    }
}

void calcCovMatrix(int32_t size, int32_t numberOfSamples, float_t matrix[4][3], IEcoMemoryAllocator1* pIMem, /* out */ float_t* covMatrix) {
    uint32_t i;
    uint32_t j;
    uint32_t n;
    float_t sum;

    float_t tmp_1;
    float_t tmp_2;

    for (i = 0; i < size; i++) {
        sum = 0;
        for (j = 0; j < numberOfSamples; j++) {
            sum += matrix[j][i];
        }

        sum /= numberOfSamples;
        for (j = 0; j < numberOfSamples; j++) {
            matrix[j][i] -= sum;
        }
    }

    /*for (i = 0; i < numberOfSamples; i++) {
        for (j = 0; j < size; j++) {
            printf("matrix[%d][%d] = %f\n", i, j, matrix[i][j]);
        }
    }*/

    // Calculate only element above main diagonal because cov matrix is symmetric
    for (i = 0; i < size; i++) {
        for (j = i; j < size; j++) {
            if (i == j) {
                // Main diagonal -> calculate DX
                // DXi = sum((Xi - MXi)^2) / (n - 1)
                // DXi = sum(Xi^2) / (n - 1) because MXi = 0
                sum = 0;
                for (n = 0; n < numberOfSamples; n++) {
                    sum += matrix[n][i] * matrix[n][i];
                }
                covMatrix[i * size + j] = sum / (numberOfSamples - 1);
            }
            else {
                // Calculate Cov
                // Cov(Xi, Xj) = sum((Xi - MXi)(Xj - MXj)) / (n - 1) = sum(Xi * Xj) / (n - 1)
                sum = 0;
                for (n = 0; n < numberOfSamples; n++) {
                    tmp_1 = matrix[n][i];
                    tmp_2 = matrix[n][j];
                    sum += matrix[n][i] * matrix[n][j];
                }
                covMatrix[i * size + j] = sum / (numberOfSamples - 1);
            }
        }
    }

    for (i = 1; i < size; i++) {
        for (j = 0; j < i; j++) {
            covMatrix[i * size + j] = covMatrix[j * size + i];
        }
    }

}

void EigenVectorValuesExtractionQRIterative(/* in */ float_t* matrix, int32_t size, IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr, float_t accuracy, int32_t maxIter,
                                            /* out */ float_t* resQ, float_t* resMatrix) {
    int32_t i;
    int32_t j;
    int32_t n;
    int32_t m;
    float_t* Q;
    float_t* R;
    float_t* qOld = NULL;
    float_t* qNew;
    float_t* currentMatrix;
    bool_t accuracyAcheived = 0;

    currentMatrix = resMatrix;

    // In case when we use the same source matrix and result matrix (matrix is mutable in this case)
    if (matrix != resMatrix) {
        for (i = 0; i < size * size; i++) {
            currentMatrix[i] = matrix[i];
        }
    }

    Q = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * size * sizeof(float_t));
    R = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * size * sizeof(float_t));
    qNew = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * size * sizeof(float_t));

    for (i = 0; i < maxIter; i++) {
        QRDecompositionGramShmidth(currentMatrix, size, pIMem, pIStr, Q, R);
        //QRDecompositionGramShmidth______BACKUP_FULLY_WORKING_WITH_SAVING_TO_ONE_FILE_EACH_E_VECTOR(currentMatrix, size, pIMem, pIStr, Q, R);
        printf("Multiplying matrices\n");
        matricesProduct(R, Q, currentMatrix, size);
        // matricesProductFlattenedFake(R, Q, currentMatrix, size);
        printf("Iteration %d completed\n", i);

        if (qOld == NULL) {
            qOld = resQ;
            for (j = 0; j < size * size; j++) {
                qOld[j] = Q[j];
            }
        }
        else {
            matricesProduct(qOld, Q, qNew, size);
            accuracyAcheived = 1;
            for (n = 0; n < size; n++) {
                for (m = 0; m < size; m++) {
                    if (fabs(fabs(qNew[n * size + m]) - fabs(qOld[n * size + m])) > accuracy) {
                        accuracyAcheived = 0;
                        break;
                    }
                }
                // Found a big diff -> break and do one more iteration
                if (accuracyAcheived == 0) {
                    break;
                }
            }

            for (n = 0; n < size; n++) {
                for (j = 0; j < size; j++) {
                    qOld[n * size + j] = qNew[n * size + j];
                }
            }

            if (accuracyAcheived != 0) {
                printf("Accuracy achieved, stop QR decomposition process\n");
                break;
            }
        }
    }

    // Free resources
    /*for (i = 0; i < size; i++) {
        pIMem->pVTbl->Free(pIMem, Q[i]);
        pIMem->pVTbl->Free(pIMem, R[i]);
        pIMem->pVTbl->Free(pIMem, qNew[i]);
    }*/
    pIMem->pVTbl->Free(pIMem, Q);
    pIMem->pVTbl->Free(pIMem, R);
    pIMem->pVTbl->Free(pIMem, qNew);
}

void PCA_init(/* in */ float_t* covMatrix, uint32_t size, IEcoMemoryAllocator1* pIMem, 
              /* in */ IEcoString1* pIStr, float_t accuracy, int32_t maxIter, int32_t componentsNumber,
              /* out */ float_t* resEigenVectors, uint8_t** bitmaps, int32_t bitmapSize, int32_t numImages) {

    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t n = 0;
    uint32_t k = 0;
    float_t** eigenVectors;
    float_t* resQ;
    float_t* resMatrix;
    int16_t result;
    int32_t count = 0;
    float_t* eigenVector = 0; // tmp value
    int64_t sum = 0;
    int64_t tmp = 0;

    if (componentsNumber > size || componentsNumber != numImages) {
        return -1;
    }

    // Allocate resources
    resQ = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * size * sizeof(float_t));
    eigenVector = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * sizeof(float_t));

    // Do calculations

    EigenVectorValuesExtractionQRIterative(covMatrix, size, pIMem, pIStr, accuracy, maxIter, resQ, covMatrix);  // use covMatrix as a result matrix

    printf("Printing eigenvalues:");
    for (i = 0; i < size; i++) {
        printf("%f ", covMatrix[i * size + i]);
    }
    printf("\n");

    // Construct eigenvectors as columns of matrix resQ
    // Eigenvectors are also written as columns
    for (i = 0; i < componentsNumber; i++) {
        // Fill EVi
        for (j = 0; j < size; j++) {
            // resEigenVectors[j * componentsNumber + i] = resQ[j * size + i];
            eigenVector[j] = resQ[j * size + i];
        }

        // Mul bitmaps on EVi
        for (j = 0; j < bitmapSize; j++) {
            sum = 0;
            for (k = 0; k < componentsNumber; k++) {
                tmp = bitmaps[k][j * 3] + (bitmaps[k][j * 3 + 1] << 8) + (bitmaps[n][j * 3 + 2] << 16);
                sum += bitmaps[k][j] * eigenVector[k];
            }
            resEigenVectors[j * componentsNumber + i] = sum;
        }

    }

    pIMem->pVTbl->Free(pIMem, resQ);
    pIMem->pVTbl->Free(pIMem, eigenVector);
}

/*
 *
 * <сводка>
 *   Функция PCA_transformBitmap
 * </сводка>
 *
 * <описание>
 *  Params:
 *    eigenvectors - matrix with eigenvectors written as columns, row size = numberEigenvectors, col size = dataItemSize
 *    dataItemSize - length of bitmap and length of one eigenvector
 *    result - vector with transformed data, length = numberEigenvectors
 * </описание>
 *
 */
void PCA_transform(float_t* dataItem, float_t* eigenVectors, int32_t dataItemSize, int32_t numberEigenvectors, /* out */ float_t* result) {
    uint32_t i;
    uint32_t j;
    int64_t tmp;

    for (i = 0; i < numberEigenvectors; i++) {
        result[i] = 0;
        for (j = 0; j < dataItemSize; j++) {
            result[i] += eigenVectors[j * numberEigenvectors + i] * dataItem[j];
        }
    }
}

/*
 *
 * <сводка>
 *   Функция PCA_transformBitmap
 * </сводка>
 *
 * <описание>
 *  Params:
*     bitmap - vector with pixels of an image, size = dataItemSize * 3 because each pixel consist of three uint8_t elements (R, G, B components)
 *    eigenvectors - matrix with eigenvectors written as columns, row size = numberEigenvectors, col size = dataItemSize
 *    dataItemSize - length of bitmap and length of one eigenvector
 *    resultMatrix - matrix with transformed data, transformed bitmaps written as columns, row size = numberImages, col size = numberEigenvectors
 * </описание>
 *
 */
void PCA_transformBitmap(uint8_t* bitmap, float_t* eigenVectors, int32_t dataItemSize, int32_t numberEigenvectors, /* out */ float_t* resultMatrix, int32_t numberImages, int32_t index) {
    uint32_t i;
    uint32_t j;
    int64_t tmp;
    double_t mul;

    for (i = 0; i < numberEigenvectors; i++) {
        resultMatrix[i * numberImages + index] = 0;
        for (j = 0; j < dataItemSize; j++) {
            tmp = bitmap[j * 3] + (bitmap[j * 3 + 1] << 8) + (bitmap[j * 3 + 2] << 16);
            // tmp = bitmap[j * 3] + bitmap[j * 3 + 1] + bitmap[j * 3 + 2];
            mul = eigenVectors[j * numberEigenvectors + i] * tmp;
            resultMatrix[i * numberImages + index] += eigenVectors[j * numberEigenvectors + i] * tmp;
        }
    }
}

void PCA_reconstruct(float_t* transformedDataItem, float_t* eigenVectors, int32_t transformedDataItemSize, int32_t numberEigenvectors, int32_t eigenvectorLength, /* out */ float_t* result) {
    uint32_t i;
    uint32_t j;

    for (i = 0; i < eigenvectorLength; i++) {
        result[i] = 0;
        for (j = 0; j < numberEigenvectors; j++) {
            result[i] += eigenVectors[i * numberEigenvectors + j] * transformedDataItem[j];
        }
    }
}

int16_t saveEigenVectors(float_t* eigenVectors, int32_t vectorLen, int32_t vectorNum, char_t* filenameBin, char_t* filenameTxt) {
    int32_t i;
    int32_t j;
    FILE *file = NULL;
    int32_t totalSize = vectorLen * vectorNum;

    file = fopen(filenameBin, "wb");
    if (file == NULL) {
        return -1;
    }

    // Write count of elements to the beginning of a file
    fwrite(&vectorLen, sizeof(int32_t), 1, file);
    fwrite(&vectorNum, sizeof(int32_t), 1, file);
    fwrite(eigenVectors, sizeof(float_t), totalSize, file);
    fclose(file);


    file = NULL;
    file = fopen(filenameTxt, "w");
    if (file == NULL) {
        return -1;
    }

    fprintf(file, "Vector len = %d, Vector num = %d, total count of elements = %d\n", vectorLen, vectorNum, totalSize);
    for (i = 0; i < vectorLen; i++) {
        for (j = 0; j < vectorNum; j++) {
            // eigenVectors array contains eigenvectors write as columns, e.g. eigenVectors = {ev0[0], ev1[0], ev0[1], ev1[1]}
            fprintf(file, "%f ", eigenVectors[i * vectorNum + j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);

    return 0;
}

int16_t saveWeights(float_t* weights, int32_t vectorLen, int32_t vectorNum, char_t* filenameBin, char_t* filenameTxt) {
    int32_t i;
    int32_t j;
    FILE *file = NULL;
    int32_t totalSize = vectorLen * vectorNum;

    file = fopen(filenameBin, "wb");
    if (file == NULL) {
        return -1;
    }

    // Write count of elements to the beginning of a file
    fwrite(&vectorLen, sizeof(int32_t), 1, file);
    fwrite(&vectorNum, sizeof(int32_t), 1, file);
    fwrite(weights, sizeof(float_t), totalSize, file);
    fclose(file);


    file = NULL;
    file = fopen(filenameTxt, "w");
    if (file == NULL) {
        return -1;
    }

    fprintf(file, "Vector len = %d, Vector num = %d, total count of elements = %d\n", vectorLen, vectorNum, totalSize);
    for (i = 0; i < vectorLen; i++) {
        for (j = 0; j < vectorNum; j++) {
            // weights matrix contains transformed data write as columns, e.g. weights = {w0[0], w1[0], w0[1], w1[1]}
            fprintf(file, "%f ", weights[i * vectorNum + j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);

    return 0;
}

int16_t saveLabels(int32_t* labels, int32_t vectorLen, char_t* filenameBin, char_t* filenameTxt) {
    int32_t i;
    int32_t j;
    FILE *file = NULL;

    file = fopen(filenameBin, "wb");
    if (file == NULL) {
        return -1;
    }

    // Write count of elements to the beginning of a file
    fwrite(&vectorLen, sizeof(int32_t), 1, file);
    fwrite(labels, sizeof(int32_t), vectorLen, file);
    fclose(file);


    file = NULL;
    file = fopen(filenameTxt, "w");
    if (file == NULL) {
        return -1;
    }

    fprintf(file, "Count of labels = %d\n", vectorLen);
    for (i = 0; i < vectorLen; i++) {
        fprintf(file, "%d ", labels[i]);
    }
    fclose(file);

    return 0;
}

int16_t saveMeanBitmap(uint8_t* meanBitmap, int32_t vectorLen, char_t* filenameBin, char_t* filenameTxt) {
    int32_t i;
    int32_t j;
    FILE *file = NULL;

    file = fopen(filenameBin, "wb");
    if (file == NULL) {
        return -1;
    }

    // Write count of elements to the beginning of a file
    fwrite(&vectorLen, sizeof(int32_t), 1, file);
    fwrite(meanBitmap, sizeof(uint8_t), vectorLen, file);
    fclose(file);


    file = NULL;
    file = fopen(filenameTxt, "w");
    if (file == NULL) {
        return -1;
    }

    // For debug purposes
    fprintf(file, "Size of bitmap = %d\n", vectorLen);
    for (i = 0; i < vectorLen; i++) {
        fprintf(file, "%d ", meanBitmap[i]);
    }
    fclose(file);

    return 0;
}

int16_t readEigenVectors(struct CEcoEigenfaces1* pCMe, char_t* filenameBin) {
    int32_t i;
    int32_t j;
    FILE *file = fopen(filenameBin, "rb");
    int32_t vectorLen = 0;
    int32_t vectorNum = 0;

    if (file == 0) {
        return -1;
    }

    fread(&vectorLen, sizeof(int32_t), 1, file);
    fread(&vectorNum, sizeof(int32_t), 1, file);

    if (vectorLen <= 0 && vectorNum <= 0) {
        return -1;
    }

    pCMe->size = vectorLen;
    pCMe->numberOfEigenVectors = vectorNum;
    pCMe->eigenVectors = (float_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, vectorLen * vectorNum * sizeof(float_t));
    
    if (pCMe->eigenVectors == 0) {
        return -1;
    }
    
    fread(pCMe->eigenVectors, sizeof(float_t), vectorLen * vectorNum, file);
    fclose(file);

    return 0;
}

int16_t readWeights(struct CEcoEigenfaces1* pCMe, char_t* filenameBin) {
    int32_t i;
    int32_t j;
    FILE *file = fopen(filenameBin, "rb");
    int32_t vectorLen = 0;
    int32_t vectorNum = 0;

    if (file == 0) {
        return -1;
    }

    fread(&vectorLen, sizeof(int32_t), 1, file);
    fread(&vectorNum, sizeof(int32_t), 1, file);

    if (vectorLen <= 0 && vectorNum <= 0) {
        return -1;
    }

    pCMe->reducedSize = vectorLen;
    pCMe->numberOfEigenVectors = vectorNum;
    pCMe->weights = (float_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, vectorLen * vectorNum * sizeof(float_t));

    if (pCMe->weights == 0) {
        return -1;
    }

    fread(pCMe->weights, sizeof(float_t), vectorLen * vectorNum, file);
    fclose(file);

    return 0;
}

int16_t readLabels(struct CEcoEigenfaces1* pCMe, char_t* filenameBin) {
    int32_t i;
    int32_t j;
    FILE *file = fopen(filenameBin, "rb");
    int32_t vectorLen = 0;

    if (file == 0) {
        return -1;
    }

    fread(&vectorLen, sizeof(int32_t), 1, file);

    if (vectorLen <= 0) {
        return -1;
    }

    pCMe->numberLabels = vectorLen;
    pCMe->labels = (int32_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, vectorLen * sizeof(int32_t));

    if (pCMe->labels == 0) {
        return -1;
    }

    fread(pCMe->labels, sizeof(int32_t), vectorLen, file);
    fclose(file);

    return 0;
}

int16_t readMeanBitmap(struct CEcoEigenfaces1* pCMe, char_t* filenameBin) {
    int32_t i;
    int32_t j;
    FILE *file = fopen(filenameBin, "rb");
    int32_t vectorLen = 0;

    if (file == 0) {
        return -1;
    }

    fread(&vectorLen, sizeof(int32_t), 1, file);

    if (vectorLen <= 0 || vectorLen != pCMe->size * 3) {
        return -1;
    }

    pCMe->meanBitmap = (uint8_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, vectorLen * sizeof(uint8_t));

    if (pCMe->meanBitmap == 0) {
        return -1;
    }

    fread(pCMe->meanBitmap, sizeof(uint8_t), vectorLen, file);
    fclose(file);

    return 0;
}

double_t euclideanDistance(float_t* vector, float_t* matrix, int32_t vectorLen, int32_t vectorNum, int32_t index) {
    int32_t i = 0;
    float_t diff = 0.0;
    float_t e1;
    float_t e2;

    double_t distance = 0.0;

    for (i = 0; i < vectorLen; i++) {
        e1 = vector[i];
        e2 = matrix[i * vectorNum + index];
        diff = vector[i] - matrix[i * vectorNum + index];
        distance += diff * diff;
    }
    distance = sqrt(distance);

    return distance;
}




void QRDecompositionGramShmidth_UnitTest(IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr) {
    int32_t i;
    int32_t j;
    float_t* Q;
    float_t* R;
    float_t eps = 0.02;
    int16_t epsExceed = 0;
    float_t testMatrixA[9] = { 12, -51, 4, 6, 167, -68, -4, 24, -41 };
    float_t expectedR[9] = { 14, 21, -14, 0, 175, -70, 0, 0, 35 };
    float_t expectedQ[9] = { 6.0 / 7, -69.0 / 175, -58.0 / 175, 3.0 / 7, 158.0 / 175, 6.0 / 175, -2.0 / 7, 6.0 / 35, -33.0 / 35 };

    Q = (float_t*)pIMem->pVTbl->Alloc(pIMem, 3 * 3 * sizeof(float_t));
    R = (float_t*)pIMem->pVTbl->Alloc(pIMem, 3 * 3 * sizeof(float_t));

    QRDecompositionGramShmidth(testMatrixA, 3, pIMem, pIStr, Q, R);

    printf("\n------- Unit test of Gramm Shmidth --------\n");
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf("Q real[%d][%d] = %f\n", i, j, Q[i * 3 + j]);
            printf("Q expe[%d][%d] = %f\n", i, j, expectedQ[i * 3 + j]);
            printf("***************************\n");
            printf("R real[%d][%d] = %f\n", i, j, R[i * 3 + j]);
            printf("R expe[%d][%d] = %f\n", i, j, expectedR[i * 3 + j]);
            printf("***************************\n");

            if (fabs(Q[i * 3 + j] - expectedQ[i * 3 + j]) > eps) {
                printf("Q matrix values are different from expected\n");
                epsExceed = 1;
            }
            if (fabs(R[i * 3 + j] - expectedR[i * 3 + j]) > eps) {
                printf("R matrix values are different from expected\n");
                epsExceed = 1;
            }
            if (epsExceed) {
                printf("Test FAILED - epsilon %f exceed\n", eps);
                break;
            }
        }
        if (epsExceed) {
            break;
        }
    }
    if (!epsExceed) {
        printf("Test PASSED - ok\n");
    }
    printf("-------------- End of a test --------------\n");
}

void QRDecompositionGramShmidth_UnitTest__four_elems(IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr) {
    int32_t i;
    int32_t j;
    float_t* Q;
    float_t* R;
    float_t eps = 0.02;
    int16_t epsExceed = 0;
    int32_t size = 4;
    float_t testMatrixA[12] = { 12, -51, 4, 6, 167, -68, -4, 24, -41, 0, 0, 0 };
    float_t expectedR[12] = { 14, 21, -14, 0, 175, -70, 0, 0, 35, 0, 0, 0 };
    float_t expectedQ[12] = { 6.0 / 7, -69.0 / 175, -58.0 / 175, 3.0 / 7, 158.0 / 175, 6.0 / 175, -2.0 / 7, 6.0 / 35, -33.0 / 35, 0, 0, 0 };

    Q = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * size * sizeof(float_t));
    R = (float_t*)pIMem->pVTbl->Alloc(pIMem, size * size * sizeof(float_t));

    QRDecompositionGramShmidth(testMatrixA, size, pIMem, pIStr, Q, R);

    printf("\n------- Unit test of Gramm Shmidth --------\n");
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            printf("Q real[%d][%d] = %f\n", i, j, Q[i * size + j]);
            printf("Q expe[%d][%d] = %f\n", i, j, expectedQ[i * size + j]);
            printf("***************************\n");
            printf("R real[%d][%d] = %f\n", i, j, R[i * size + j]);
            printf("R expe[%d][%d] = %f\n", i, j, expectedR[i * size + j]);
            printf("***************************\n");

            if (fabs(Q[i * size + j] - expectedQ[i * size + j]) > eps) {
                printf("Q matrix values are different from expected\n");
                epsExceed = 1;
            }
            if (fabs(R[i * size + j] - expectedR[i * size + j]) > eps) {
                printf("R matrix values are different from expected\n");
                epsExceed = 1;
            }
            if (epsExceed) {
                printf("Test FAILED - epsilon %f exceed\n", eps);
                // break;
            }
        }
        if (epsExceed) {
            // break;
        }
    }
    if (!epsExceed) {
        printf("Test PASSED - ok\n");
    }
    printf("-------------- End of a test --------------\n");
}

void EigenVectorValuesExtractionQRIterative_UnitTest(IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr) {
    int32_t i;
    int32_t j;
    float_t eps = 0.02;
    int16_t epsExceed = 0;
    float_t* Q;
    float_t* resultMatrix;
    float_t testMatrixB[9] = { 1, 2, 4, 2, 9, 8, 4, 8, 2 };
    float_t expected[3] = { 15.2964, 4.3487, 1.0523 };

    resultMatrix = (float_t*)pIMem->pVTbl->Alloc(pIMem, 3 * 3 * sizeof(float_t));
    Q = (float_t*)pIMem->pVTbl->Alloc(pIMem, 3 * 3 * sizeof(float_t));
    EigenVectorValuesExtractionQRIterative(testMatrixB, 3, pIMem, pIStr, 0.001, 1000, Q, resultMatrix);

    printf("\n------- Unit test of QR iterative algorithm --------\n");
    for (i = 0; i < 3; i++) {
        printf("  Result[%d][%d] = %lf\n", i, i, resultMatrix[i * 3 + i]);
        printf("Expected[%d][%d] = %lf\n", i, i, expected[i]);
        printf("***************************\n");

        if (resultMatrix[i * 3 + i] < 0 && expected[i] < 0 || resultMatrix[i * 3 + i] > 0 && expected[i] > 0) {
            // Same signs
            if (fabs(resultMatrix[i * 3 + i] - expected[i]) > eps) {
                printf("Result matrix values are different from expected\n");
                epsExceed = 1;
            }
        }
        else {
            // Opposite signs - it is ok, just eigenvectors have the opposite direction of expected
            if (fabs(resultMatrix[i * 3 + i] + expected[i]) > eps) {
                printf("Result matrix values are different from expected\n");
                epsExceed = 1;
            }
        }
        if (epsExceed) {
            printf("Test FAILED - epsilon %f exceed\n", eps);
            break;
        }
    }
    if (!epsExceed) {
        printf("Test PASSED - ok\n", eps);
    }
    printf("------------------ End of a test -------------------\n");

    pIMem->pVTbl->Free(pIMem, resultMatrix);
    pIMem->pVTbl->Free(pIMem, Q);
}

void PCA_UnitTest(IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr) {
    int32_t i;
    int32_t j;
    float_t eps = 0.02;
    int16_t epsExceed = 0;
    float_t* Q;
    float_t _data[4][3] = { {1, 2, 23}, {-3, 17, 5}, {13, -6, 7}, {7, 8, -9} };
    float_t _v[3] = { 1, 0, 3 };
    float_t expReduced[2] = { -2.75008, 0.19959 };
    float_t expReconstructed[3] = { -0.21218, -0.87852, 2.60499 };
    float_t* testCovMatrix;
    float_t** testEigenVectors;
    float_t testTransformResultMatrix[2];
    float_t testReconstructResultMatrix[3];
    float_t** eigenVectors;


    testEigenVectors = (float_t**)pIMem->pVTbl->Alloc(pIMem, 2 * sizeof(float_t*));
    testCovMatrix = (float_t*)pIMem->pVTbl->Alloc(pIMem, 3 * 3 * sizeof(float_t));
    calcCovMatrix(3, 4, _data, pIMem, testCovMatrix);

    PCA_init(testCovMatrix, 3, pIMem, pIStr, 0.0001, 1000, 2, testEigenVectors, 0, 0, 0);


    printf("\n------- Unit test of PCA transform method --------\n");
    PCA_transform(_v, testEigenVectors, 3, 2, testTransformResultMatrix);
    for (i = 0; i < 2; i++) {
        printf("Real = %lf, expected = %lf\n", testTransformResultMatrix[i], expReduced[i]);

        if (fabs(testTransformResultMatrix[i] - expReduced[i]) > eps) {
            printf("Transformed matrix values are different from expected\n");
            epsExceed = 1;
        }
        if (epsExceed) {
            printf("Test FAILED - epsilon %f exceed\n", eps);
            break;
        }
    }
    if (!epsExceed) {
        printf("Test PASSED - ok\n");
    }
    printf("----------------- End of a test ------------------\n");

    printf("\n------- Unit test of PCA reconstruct method --------\n");
    PCA_reconstruct(testTransformResultMatrix, testEigenVectors, 2, 2, 3, testReconstructResultMatrix);
    for (i = 0; i < 3; i++) {
        printf("Real = %lf, expected = %lf\n", testReconstructResultMatrix[i], expReconstructed[i]);
        if (fabs(testReconstructResultMatrix[i] - expReconstructed[i]) > eps) {
            printf("Reconstructed matrix values are different from expected\n");
            epsExceed = 1;
        }
        if (epsExceed) {
            printf("Test FAILED - epsilon %f exceed\n", eps);
            break;
        }
    }

    if (!epsExceed) {
        printf("Test PASSED - ok\n");
    }
    printf("------------------ End of a test -------------------\n");


    for (i = 0; i < 2; i++) {
        pIMem->pVTbl->Free(pIMem, testEigenVectors[i]);
    }
    pIMem->pVTbl->Free(pIMem, testCovMatrix);
    pIMem->pVTbl->Free(pIMem, testEigenVectors);

}



int16_t CEcoEigenfaces1_Train(/* in */ struct IEcoEigenfaces1* me, uint8_t** bitmaps, uint32_t bitmapSize, int32_t numImages) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;
    int32_t i = 0;
    int32_t j = 0;
    int32_t c = 0;
    int32_t n = 0;
    int32_t k = 0;

    byte_t* buffer = 0;
    int16_t result = -1;

    uint32_t covMatrixSize = 0;
    uint32_t size_div_3 = 0;
    int32_t elem = 0;
    int64_t mean = 0;
    int64_t sum = 0;
    int32_t pixel_i = 0;
    int32_t pixel_j = 0;

    uint8_t test[2][6] = { {1, 2, 3, 4, 5, 6}, {7, 8, 9, 10, 11, 12} };


    // uint8_t** bitmaps;
    float_t* transformedBitmaps;

    // For covariance matrix calculations
    float_t* covMatrix = 0;

    // For PCA
    // float_t* eigenVectors;
    int32_t newDimension;


    /* Проверка указателей */
    if (me == 0) {
        return -1;
    }

    QRDecompositionGramShmidth_UnitTest(pCMe->m_pIMem, pCMe->m_pIString);
    EigenVectorValuesExtractionQRIterative_UnitTest(pCMe->m_pIMem, pCMe->m_pIString);

    /* PHASE 1 - Calculate mean image and subtract it from all bitmaps */
    size_div_3 = bitmapSize / 3;
    buffer = (uint8_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, bitmapSize * sizeof(uint8_t));

    for (i = 0; i < bitmapSize; i++) {
        mean = 0;
        for (n = 0; n < numImages; n++) {
            mean += bitmaps[n][i];
        }
        buffer[i] = mean / numImages;

        for (n = 0; n < numImages; n++) {
            bitmaps[n][i] -= buffer[i];
        }
    }
    pCMe->meanBitmap = buffer;

    /* PHASE 2 - Calculate covariation matrix */
    /*covMatrixSize = size_div_3 * size_div_3;
    covMatrix = (float_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, covMatrixSize * sizeof(float_t));
    calcImageCovMatrix(pCMe->m_pIMem, covMatrix, size_div_3, bitmaps, numImages);*/

    // Try to use T_transposed * T instead covariation matrix, where T is a matrix of normalized bitmaps (each column is a bitmap)
    // The approach explanation
    // Cov matrix = S, T - matrix of normalized bitmaps, L - eigenvalues, V - eigenvectors
    // S = T * T_transposed
    // S * V = T * T_transposed * V = L * V
    // Let's calculate T_transposed * T instead of cov matrix S:
    // T_transposed * T * U = L U
    // Multiply each parts by T:
    // T * T_transposed * T * U = L * T * U
    // Then U is eigenvectors of T_transposed * T and V = T * U
    // More info about this: https://en.wikipedia.org/wiki/Eigenface#Computing_the_eigenvectors (Section "Computing the eigenvectors")

    covMatrix = (float_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, numImages * numImages * sizeof(float_t));
    for (i = 0; i < numImages; i++) {
        for (j = 0; j < numImages; j++) {
            sum = 0;
            for (k = 0; k < size_div_3; k++) {
                pixel_i = bitmaps[i][k * 3] + (bitmaps[i][k * 3 + 1] << 8) + (bitmaps[i][k * 3 + 2] << 16);
                pixel_j = bitmaps[j][k * 3] + (bitmaps[j][k * 3 + 1] << 8) + (bitmaps[j][k * 3 + 2] << 16);
                
                sum += pixel_i * pixel_j;
            }
            covMatrix[i * numImages + j] = sum;
        }
    }

    /* PHASE 3 - Apply PCA */
    pCMe->eigenVectors = (float_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, numImages * size_div_3 * sizeof(float_t));

    newDimension = numImages;
    // PCA_init(covMatrix, size_div_3, pCMe->m_pIMem, pCMe->m_pIString, 0.0001, 10, newDimension, pCMe->eigenVectors);
    PCA_init(covMatrix, numImages, pCMe->m_pIMem, pCMe->m_pIString, 0.0001, 1000, newDimension, pCMe->eigenVectors, bitmaps, size_div_3, numImages);

    /* PHASE 4 - Do transformation of test images */
    pCMe->size = size_div_3;
    pCMe->reducedSize = newDimension;
    pCMe->numberOfEigenVectors = numImages;

    pCMe->weights = (float_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, numImages * newDimension * sizeof(float_t));
    for (i = 0; i < numImages; i++) {
        PCA_transformBitmap(bitmaps[i], pCMe->eigenVectors, size_div_3, newDimension, pCMe->weights, numImages, i);
    }

    // Clean up, restore old bitmap values
    for (i = 0; i < bitmapSize; i++) {
        for (n = 0; n < numImages; n++) {
            bitmaps[n][i] += buffer[i];
        }
    }

    /* LAST PHASE - free all resources */
    pCMe->m_pIMem->pVTbl->Free(pCMe->m_pIMem, covMatrix);
    // pCMe->m_pIMem->pVTbl->Free(pCMe->m_pIMem, buffer);

    return 0;
}

int16_t CEcoEigenfaces1_Save(/* in */ struct IEcoEigenfaces1* me, int32_t* labels, char_t* filenameEigenVectorsBinary, char_t* filenameWeightsBinary, char_t* filenameLabels) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;
    int16_t success1 = -2;
    int16_t success2 = -2;
    int16_t success3 = -2;
    int16_t success4 = -2;

    /* Check pointers */
    if (me == 0 || filenameEigenVectorsBinary == 0 || filenameWeightsBinary == 0 || pCMe->eigenVectors == 0 || pCMe->weights == 0) {
        return -1;
    }

    pCMe->numberLabels = pCMe->numberOfEigenVectors;

    success1 = saveEigenVectors(pCMe->eigenVectors, pCMe->size, pCMe->numberOfEigenVectors, filenameEigenVectorsBinary, "ev.txt");
    success2 = saveWeights(pCMe->weights, pCMe->reducedSize, pCMe->numberOfEigenVectors, filenameWeightsBinary, "train_weights.txt");
    success3 = saveLabels(labels, pCMe->numberLabels, filenameLabels, "labels.txt");
    success4 = saveMeanBitmap(pCMe->meanBitmap, pCMe->size * 3, "mean_bitmap.bin", "mean_bitmap.txt");  // Mul 3 because each pixel consist of 3 bytes (rgb)

    if (success1 != 0 || success2 != 0 || success3 != 0 || success4 != 0) {
        return -1;
    }

    return 0;
}

int16_t CEcoEigenfaces1_Load(/* in */ struct IEcoEigenfaces1* me, char_t* filenameEigenVectorsBinary, char_t* filenameWeightsBinary, char_t* filenameLabels) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;
    int16_t success1 = -2;
    int16_t success2 = -2;
    int16_t success3 = -2;
    int16_t success4 = -2;

    /* Check pointers */
    if (me == 0 || filenameEigenVectorsBinary == 0 || filenameWeightsBinary == 0 || filenameLabels == 0) {
        return -1;
    }

    success1 = readEigenVectors(pCMe, filenameEigenVectorsBinary);
    success2 = readWeights(pCMe, filenameWeightsBinary);
    success3 = readLabels(pCMe, filenameLabels);
    success4 = readMeanBitmap(pCMe, "mean_bitmap.bin");

    if (success1 != 0 || success2 != 0 || success3 != 0 || success4 != 0) {
        return -1;
    }

    return 0;
}

int16_t CEcoEigenfaces1_Predict(/* in */ struct IEcoEigenfaces1* me, uint8_t* bitmap, uint32_t bitmapSize, int32_t* predictedLabel) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;
    int32_t i = 0;
    int32_t j = 0;
    float_t* weights = 0;
    uint32_t tmp = 0;
    double_t distance = 0.0;
    double_t min_distance = 1000000000000.0;
    double_t second_min_distance = 1000000000000.0;
    double_t sumDistances = 0.0;
    double_t distanceDiff;
    int32_t label = 0;

    /* Проверка указателей */
    if (me == 0) {
        return -1;
    }

    if (bitmapSize != pCMe->size * 3) {
        return -1;
    }

    /* PHASE 1 - Subtract mean image from a given bitmap */
    for (i = 0; i < pCMe->size * 3; i++) {
        tmp = bitmap[i];
        bitmap[i] -= pCMe->meanBitmap[i];
        tmp = bitmap[i];
    }

    /* PHASE 2 - Apply PCA */
    weights = (float_t*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, pCMe->reducedSize * sizeof(float_t));
    PCA_transformBitmap(bitmap, pCMe->eigenVectors, pCMe->size, pCMe->reducedSize, weights, 1, 0);

    /* PHASE 3 - Calculate distances and choose a target class */
    // printf("Predicted distances\n");
    for (i = 0; i < pCMe->reducedSize; i++) {
        distance = euclideanDistance(weights, pCMe->weights, pCMe->reducedSize, pCMe->reducedSize, i);
        sumDistances += distance;
        if (i == 0) {
            min_distance = distance;
            second_min_distance = distance + 1;
        }
        if (distance < min_distance) {
            second_min_distance = min_distance;
            min_distance = distance;
            label = pCMe->labels[i];
        }
        // printf("Distance [%d], %f \n", i, distance);
    }

    distanceDiff = (second_min_distance - min_distance) / min_distance;
    printf("Sec distance = %f \n", second_min_distance);
    printf("Min distance = %f, distance diff = %f %\n", min_distance, distanceDiff);
    *predictedLabel = label;

    // Clean up, restore old bitmap values
    for (i = 0; i < pCMe->size * 3; i++) {
        bitmap[i] += pCMe->meanBitmap[i];
    }

    return 0;
}

/*
 *
 * <сводка>
 *   Функция Init
 * </сводка>
 *
 * <описание>
 *   Функция инициализации экземпляра
 * </описание>
 *
 */
int16_t initCEcoEigenfaces1(/*in*/ struct IEcoEigenfaces1* me, /* in */ struct IEcoUnknown *pIUnkSystem) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)me;
    int16_t result = -1;

    /* Проверка указателей */
    if (me == 0 ) {
        return result;
    }

    /* Сохранение указателя на системный интерфейс */
    pCMe->m_pISys = (IEcoSystem1*)pIUnkSystem;

	/* Получение интерфейс для работы со строкой */
    result = pCMe->m_pIBus->pVTbl->QueryComponent(pCMe->m_pIBus, &CID_EcoString1, 0, &IID_IEcoString1, (void**)&pCMe->m_pIString);
    if (result != 0 || pCMe->m_pIString == 0) {
        return result;
    }

    return result;
}

/* Create Virtual Table */
IEcoEigenfaces1VTbl g_xE7CE2CE5711947048CA6542C41D93A13VTbl = {
    CEcoEigenfaces1_QueryInterface,
    CEcoEigenfaces1_AddRef,
    CEcoEigenfaces1_Release,
    CEcoEigenfaces1_Train,
    CEcoEigenfaces1_Save,
    CEcoEigenfaces1_Load,
    CEcoEigenfaces1_Predict
};


/*
 *
 * <сводка>
 *   Функция Create
 * </сводка>
 *
 * <описание>
 *   Функция создания экземпляра
 * </описание>
 *
 */
int16_t createCEcoEigenfaces1(/* in */ IEcoUnknown* pIUnkSystem, /* in */ IEcoUnknown* pIUnkOuter, /* out */ IEcoEigenfaces1** ppIEcoEigenfaces1) {
    int16_t result = -1;
    IEcoSystem1* pISys = 0;
    IEcoInterfaceBus1* pIBus = 0;
    IEcoMemoryAllocator1* pIMem = 0;
    IEcoMemoryAllocator1* pIStr = 0;
    CEcoEigenfaces1* pCMe = 0;

    /* Проверка указателей */
    if (ppIEcoEigenfaces1 == 0 || pIUnkSystem == 0) {
        return result;
    }

    /* Получение системного интерфейса приложения */
    result = pIUnkSystem->pVTbl->QueryInterface(pIUnkSystem, &GID_IEcoSystem1, (void **)&pISys);

    /* Проверка */
    if (result != 0 && pISys == 0) {
        return result;
    }

    /* Получение интерфейса для работы с интерфейсной шиной */
    result = pISys->pVTbl->QueryInterface(pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);

    /* Получение интерфейса распределителя памяти */
    pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoMemoryManager1, 0, &IID_IEcoMemoryAllocator1, (void**) &pIMem);

    /* Проверка */
    if (result != 0 && pIMem == 0) {
        /* Освобождение системного интерфейса в случае ошибки */
        pISys->pVTbl->Release(pISys);
        return result;
    }

    /* Выделение памяти для данных экземпляра */
    pCMe = (CEcoEigenfaces1*)pIMem->pVTbl->Alloc(pIMem, sizeof(CEcoEigenfaces1));

    /* Сохранение указателя на системный интерфейс */
    pCMe->m_pISys = pISys;

    /* Сохранение указателя на интерфейс для работы с памятью */
    pCMe->m_pIMem = pIMem;

    /* Установка счетчика ссылок на компонент */
    pCMe->m_cRef = 1;

    /* Создание таблицы функций интерфейса IEcoEigenfaces1 */
    pCMe->m_pVTblIEcoEigenfaces1 = &g_xE7CE2CE5711947048CA6542C41D93A13VTbl;

    /* Инициализация данных */
    pCMe->weights = 0;
    pCMe->eigenVectors = 0;
    pCMe->labels = 0;
    pCMe->meanBitmap = 0;

    pCMe->size = 0;
    pCMe->reducedSize = 0;
    pCMe->numberOfEigenVectors = 0;
	pCMe->m_pIString = 0;
	pCMe->m_pIBus = pIBus;

    /* Возврат указателя на интерфейс */
    *ppIEcoEigenfaces1 = (IEcoEigenfaces1*)pCMe;

    /* Освобождение */
    pIBus->pVTbl->Release(pIBus);

    return 0;
}

/*
 *
 * <сводка>
 *   Функция Delete
 * </сводка>
 *
 * <описание>
 *   Функция освобождения экземпляра
 * </описание>
 *
 */
void deleteCEcoEigenfaces1(/* in */ IEcoEigenfaces1* pIEcoEigenfaces1) {
    CEcoEigenfaces1* pCMe = (CEcoEigenfaces1*)pIEcoEigenfaces1;
    IEcoMemoryAllocator1* pIMem = 0;

    if (pIEcoEigenfaces1 != 0 ) {
        pIMem = pCMe->m_pIMem;
        /* Освобождение */
        if (pCMe->weights != 0) {
            pIMem->pVTbl->Free(pIMem, pCMe->weights);
        }

        if (pCMe->eigenVectors != 0) {
            pIMem->pVTbl->Free(pIMem, pCMe->eigenVectors);
        }

        if (pCMe->labels != 0) {
            pIMem->pVTbl->Free(pIMem, pCMe->labels);
        }

        if (pCMe->meanBitmap != 0) {
            pIMem->pVTbl->Free(pIMem, pCMe->meanBitmap);
        }

        if ( pCMe->m_pISys != 0 ) {
            pCMe->m_pISys->pVTbl->Release(pCMe->m_pISys);
        }
        pIMem->pVTbl->Free(pIMem, pCMe);
        pIMem->pVTbl->Release(pIMem);
    }
}
