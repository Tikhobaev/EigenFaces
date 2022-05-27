/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   EcoEigenfaces1
 * </сводка>
 *
 * <описание>
 *   Данный исходный файл является точкой входа
 * </описание>
 *
 * <автор>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 */



/* Eco OS */
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoFileSystemManagement1.h"
#include "IdEcoString1.h"
#include "IdEcoEigenfaces1.h"
#include "IdEcoImageBMP3.h"
#include "EcoFormDialog.h"

#include <stdio.h>
#include <math.h>
#include <time.h>


typedef struct ECO_RGBA_BITMAP {
    int16_t width;
    int16_t height;
    int8_t channels; /* 3 - RGB, 4 - RGBA*/
    uint8_t* bitmap;
} ECO_RGBA_BITMAP;



typedef struct ECO_MY_RECORD {
    char_t fileName[64];
    int16_t width;
    int16_t height;
    int8_t channels; /* 3 - RGB, 4 - RGBA*/
    uint8_t* bitmap;
    int16_t faceX;
    int16_t faceY;
    int16_t faceWidth;
    int16_t faceHeight;
    int16_t leftEyeX;
    int16_t leftEyeY;
    int16_t rightEyeX;
    int16_t rightEyeY;
    uint32_t personId;
    uint8_t gender;
} ECO_MY_RECORD;


/* Указатель на интерфейс для работы с BMP файлом */
IEcoImageBMP3File* g_pIBMP1 = 0;
IEcoImageBMP3File* g_pIBMP2 = 0;
IEcoImageBMP3File* g_pIBMPMean = 0;
IEcoImageBMP3File** train_images = 0;
IEcoImageBMP3File** test_images = 0;


/*
 *
 * <сводка>
 *   Функция ReadImages
 * </сводка>
 *
 * <описание>
 *   Функция ReadImages
 * </описание>
 *
 */
int16_t ReadImages(IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr, IEcoImageBMP3* pIBMP, char_t* pszPathToDataDir, uint32_t numberDir, uint32_t numberImages, 
    IEcoImageBMP3File** images, uint8_t** bitmaps, uint32_t* size) {
    
    int16_t result = -1;
    int32_t n = 0;
    int32_t s = 0;
    int32_t k = 0;
    char_t* pszFullPathToFile = 0;

    for (s = 0; s < numberDir; s++) {
        for (n = 0; n < numberImages; n++) {
            /* Получаем путь */
            pszFullPathToFile = pIStr->pVTbl->Append(pIStr, pszFullPathToFile, pszPathToDataDir);
            pszFullPathToFile = pIStr->pVTbl->Append(pIStr, pszFullPathToFile, "s");
            pszFullPathToFile = pIStr->pVTbl->Append(pIStr, pszFullPathToFile, pIStr->pVTbl->ConvertIntToString(pIStr, s + 1));
            pszFullPathToFile = pIStr->pVTbl->Append(pIStr, pszFullPathToFile, "\\");
            pszFullPathToFile = pIStr->pVTbl->Append(pIStr, pszFullPathToFile, pIStr->pVTbl->ConvertIntToString(pIStr, n + 1));
            pszFullPathToFile = pIStr->pVTbl->Append(pIStr, pszFullPathToFile, ".bmp");

            result = pIBMP->pVTbl->Load(pIBMP, pszFullPathToFile, &images[s * numberImages + n]);

            pIStr->pVTbl->Free(pIStr, pszFullPathToFile);
            pszFullPathToFile = 0;

            if (images[s * numberImages + n] == 0) {
                return -1;
            }

            bitmaps[s * numberImages + n] = (byte_t*)pIMem->pVTbl->Alloc(pIMem, *size);
            if ((result = images[s * numberImages + n]->pVTbl->ReadBitmapData(images[s * numberImages + n], bitmaps[s * numberImages + n], size)) != 0) {
                return -1;
            }

            k = s * numberImages + n;

            if (bitmaps[s * numberImages + n] == 0 || bitmaps[s * numberImages + n][0] == 0) {
                return -1;
            }
        }
    }

    return 0;
}


/*
 *
 * <сводка>
 *   Функция Processing
 * </сводка>
 *
 * <описание>
 *   Функция Processing
 * </описание>
 *
 */
int16_t Processing(IEcoMemoryAllocator1* pIMem, IEcoString1* pIStr, IEcoImageBMP3* pIBMP, IEcoEigenfaces1* pIEcoEigenfaces1) {
    int32_t i = 0;
    int32_t j = 0;
    int32_t c = 0;
    int32_t n = 0;
    int32_t k = 0;
    char_t* pszFullPathToFile = 0;
    int16_t result = -1;
    char_t* pszPathToDataDirTrain = "..\\..\\..\\ImageFiles\\train\\medium\\";
    char_t* pszPathToDataDirTest = "..\\..\\..\\ImageFiles\\test\\medium\\";

    char_t* filename1 = "..\\..\\..\\ImageFiles\\train\\medium\\s1\\1.bmp";
    char_t* filename2 = 0;
    char_t* filename3 = 0;


    int16_t width = 0;
    int16_t height = 0;
    int8_t channels = 0;
    uint32_t size = 0;
    uint32_t size_div_3 = 0;
    int32_t elem = 0;
    long_t mean = 0;

    uint32_t TRAIN_NUMBER_IMAGES_PER_DIR = 6;
    uint32_t TRAIN_NUMBER_DIRS = 7;
    uint32_t TRAIN_NUMBER_IMAGES = 0;

    uint32_t TEST_NUMBER_IMAGES_PER_DIR =4;
    uint32_t TEST_NUMBER_DIRS = 7;
    uint32_t TEST_NUMBER_IMAGES = 0;

    uint8_t* bitmap1;
    uint8_t* bitmap2;
    uint8_t** train_bitmaps;
    float_t* transformedTrainBitmaps;

    uint8_t** test_bitmaps;
    int32_t* trainLabels = 0;
    int32_t* testLabels = 0;
    float_t* tmp;
    clock_t begin;
    clock_t end;
    int32_t predictedLabel = -1;

    double_t test_accuracy = 0.0;
    double_t num_correct = 0.0;
    
    TRAIN_NUMBER_IMAGES = TRAIN_NUMBER_IMAGES_PER_DIR * TRAIN_NUMBER_DIRS;
    TEST_NUMBER_IMAGES = TEST_NUMBER_IMAGES_PER_DIR * TEST_NUMBER_DIRS;
        
    /* Загрузка изображения */
    result = pIBMP->pVTbl->Load(pIBMP, filename1, &g_pIBMP1);

    // TODO remove g_pIBMP1 and use first image from array "images"
    if (g_pIBMP1 == 0) {
        printf("Cannot open file by path %s\n", filename1);
        return -1;
    }

    width = g_pIBMP1->pVTbl->get_InfoHeader(g_pIBMP1)->biWidth;
    height = g_pIBMP1->pVTbl->get_InfoHeader(g_pIBMP1)->biHeight;
    channels = g_pIBMP1->pVTbl->get_InfoHeader(g_pIBMP1)->biBitCount / 8;  // Size of 1 pixel in bytes

    size = width * channels * height;
    size_div_3 = size / 3;

    train_bitmaps = (uint8_t**)pIMem->pVTbl->Alloc(pIMem, sizeof(uint8_t*) * TRAIN_NUMBER_IMAGES);
    train_images = (IEcoImageBMP3File**)pIMem->pVTbl->Alloc(pIMem, TRAIN_NUMBER_IMAGES * sizeof(IEcoImageBMP3File*));

    // Train dataset
    ReadImages(pIMem, pIStr, pIBMP, pszPathToDataDirTrain, TRAIN_NUMBER_DIRS, TRAIN_NUMBER_IMAGES_PER_DIR, train_images, train_bitmaps, &size);
    

    
    filename1 = "ev.bin";
    filename2 = "train_weights.bin";
    filename3 = "labels.bin";
    pIEcoEigenfaces1->pVTbl->Train(pIEcoEigenfaces1, train_bitmaps, size, TRAIN_NUMBER_IMAGES);

   
    trainLabels = (int32_t*)pIMem->pVTbl->Alloc(pIMem, TRAIN_NUMBER_IMAGES * sizeof(int32_t));
    for (i = 0; i < TRAIN_NUMBER_DIRS; i++) {
        for (j = 0; j < TRAIN_NUMBER_IMAGES_PER_DIR; j++) {
            trainLabels[i * TRAIN_NUMBER_IMAGES_PER_DIR + j] = i;
        }
    }
    
    /*for (j = 0; j < TRAIN_NUMBER_IMAGES; j++) {
        printf("%d ", trainLabels[j]);
    }
    printf("\n");*/

    pIEcoEigenfaces1->pVTbl->Save(pIEcoEigenfaces1, trainLabels, filename1, filename2, filename3);




    
    /* Predict method */
    // Test dataset
    testLabels = (int32_t*)pIMem->pVTbl->Alloc(pIMem, TEST_NUMBER_IMAGES * sizeof(int32_t));
    for (i = 0; i < TEST_NUMBER_DIRS; i++) {
        for (j = 0; j < TEST_NUMBER_IMAGES_PER_DIR; j++) {
            testLabels[i * TEST_NUMBER_IMAGES_PER_DIR + j] = i;
        }
    }

    test_bitmaps = (uint8_t**)pIMem->pVTbl->Alloc(pIMem, sizeof(uint8_t*) * TEST_NUMBER_IMAGES);
    test_images = (IEcoImageBMP3File**)pIMem->pVTbl->Alloc(pIMem, sizeof(IEcoImageBMP3File*) * TEST_NUMBER_IMAGES);
    ReadImages(pIMem, pIStr, pIBMP, pszPathToDataDirTest, TEST_NUMBER_DIRS, TEST_NUMBER_IMAGES_PER_DIR, test_images, test_bitmaps, &size);

    pIEcoEigenfaces1->pVTbl->Load(pIEcoEigenfaces1, filename1, filename2, filename3);

    for (i = 0; i < TEST_NUMBER_IMAGES; i++) {
        if (i > 19) {
            j = 0;
        }

        begin = clock();
        pIEcoEigenfaces1->pVTbl->Predict(pIEcoEigenfaces1, test_bitmaps[i], size, &predictedLabel); 

        end = clock();
        // printf("Total predict time: %lf\n", (double_t)(end - begin) / CLOCKS_PER_SEC);
        printf("Real / Predicted label: %d / %d\n", testLabels[i], predictedLabel);
        if (testLabels[i] == predictedLabel) {
            num_correct += 1;
        }
    }

    test_accuracy = num_correct / TEST_NUMBER_IMAGES;
    printf("Test accuracy: %lf\n", test_accuracy);

    /* LAST PHASE - free all resources */
    for (i = 0; i < TRAIN_NUMBER_IMAGES; i++) {
        train_images[i]->pVTbl->Release(train_images[i]);
        train_images[i] = 0;

        tmp = train_bitmaps[i];
        pIMem->pVTbl->Free(pIMem, train_bitmaps[i]);
    }

    pIMem->pVTbl->Free(pIMem, train_images);
    pIMem->pVTbl->Free(pIMem, train_bitmaps);
    

    g_pIBMP1->pVTbl->Release(g_pIBMP1);
    g_pIBMP1 = 0;
    // g_pIBMPMean->pVTbl->Release(g_pIBMPMean);
    g_pIBMPMean = 0;

    // Выводим попиксельно изображение. Начиная с нижней строки пикселей, слева направо, снизу вверх
    /*
    for (i = 0; i < height; i++) {
    for (j = 0; j < width * 3; j += 3) {
    printf("(%d, %d, %d) ", bitmap[i * width + j], bitmap[i * width + j + 1], bitmap[i * width + j + 2]);
    }
    printf("\n");
    }
    */

    //result = pIBMP->pVTbl->Save(pIBMP, filename_to_save, pIBMPFile);
    //if (result != 0) {
    //    printf("Can't save data");
    //}

    return 0;
}


/*
 *
 * <сводка>
 *   Функция EcoMain
 * </сводка>
 *
 * <описание>
 *   Функция EcoMain - точка входа
 * </описание>
 *
 */
int16_t EcoMain(IEcoUnknown* pIUnk) {
    int16_t result = -1;
    /* Указатель на системный интерфейс */
    IEcoSystem1* pISys = 0;
    /* Указатель на интерфейс работы с системной интерфейсной шиной */
    IEcoInterfaceBus1* pIBus = 0;
    /* Указатель на интерфейс работы с памятью */
    IEcoMemoryAllocator1* pIMem = 0;
    /* Указатель на интерфейс работы со строкой */
    IEcoString1* pIStr = 0;
    /* Указатель на тестируемый интерфейс */
    IEcoEigenfaces1* pIEcoEigenfaces1 = 0;
    IEcoImageBMP3* pIBMP = 0;

    /* Проверка и создание системного интрефейса */
    if (pISys == 0) {
        result = pIUnk->pVTbl->QueryInterface(pIUnk, &GID_IEcoSystem1, (void **)&pISys);
        if (result != 0 && pISys == 0) {
        /* Освобождение системного интерфейса в случае ошибки */
            goto Release;
        }
    }

    /* Получение интерфейса для работы с интерфейсной шиной */
    result = pISys->pVTbl->QueryInterface(pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);
    if (result != 0 || pIBus == 0) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
#ifdef ECO_LIB
    /* Регистрация статического компонента для работы с Eigenfaces */
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoEigenfaces1, (IEcoUnknown*)GetIEcoComponentFactoryPtr_E9E9C1DF6AF44C268A97FE52AFDAB124);
    if (result != 0 ) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
    /* Регистрация статического компонента для работы с форматом файла BMP v.3.0 */
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoImageBMP3, (IEcoUnknown*)GetIEcoComponentFactoryPtr_CE50C623798D457C93ADA508A70259D9);
    if (result != 0) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
    /* Регистрация статического компонента для работы со строкой */
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoString1, (IEcoUnknown*)GetIEcoComponentFactoryPtr_84CC0A7DBABD44EEBE749C9A8312D37E);
    if (result != 0 ) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
#endif

    /* Получение интерфейса управление файлами */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoImageBMP3, 0, &IID_IEcoImageBMP3, (void**)&pIBMP);
    if (result != 0 || pIBMP == 0) {
        /* Освобождение в случае ошибки */
        goto Release;
    }

    /* Получение интерфейса управления памятью */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoMemoryManager1, 0, &IID_IEcoMemoryAllocator1, (void**) &pIMem);

    /* Проверка */
    if (result != 0 && pIMem == 0) {
        /* Освобождение системного интерфейса в случае ошибки */
        goto Release;
    }

    /* Получение интерфейса для работы со строкой */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoString1, 0, &IID_IEcoString1, (void**) &pIStr);
    /* Проверка */
    if (result != 0 || pIStr == 0) {
        /* Освобождение в случае ошибки */
        goto Release;
    }

    /* Получение тестируемого интерфейса */
    pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoEigenfaces1, 0, &IID_IEcoEigenfaces1, (void**) &pIEcoEigenfaces1);
    if (result != 0 && pIEcoEigenfaces1 == 0) {
        /* Освобождение интерфейсов в случае ошибки */
        goto Release;
    }

    Processing(pIMem, pIStr, pIBMP, pIEcoEigenfaces1);

Release:

    /* Освобождение интерфейса для работы с интерфейсной шиной */
    if (pIBus != 0) {
        pIBus->pVTbl->Release(pIBus);
    }

    /* Освобождение интерфейса работы с памятью */
    if (pIMem != 0) {
        pIMem->pVTbl->Release(pIMem);
    }

    /* Освобождение тестируемого интерфейса */
    if (pIEcoEigenfaces1 != 0) {
        pIEcoEigenfaces1->pVTbl->Release(pIEcoEigenfaces1);
    }


    /* Освобождение системного интерфейса */
    if (pISys != 0) {
        pISys->pVTbl->Release(pISys);
    }

    return result;
}

