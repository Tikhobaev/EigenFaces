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
 *   Данный заголовок описывает реализацию компонента CEcoEigenfaces1
 * </описание>
 *
 * <автор>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 */

#ifndef __C_ECOEIGENFACES1_H__
#define __C_ECOEIGENFACES1_H__

#include "IEcoEigenfaces1.h"
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IEcoInterfaceBus1.h"

typedef struct CEcoEigenfaces1 {

    /* Таблица функций интерфейса IEcoEigenfaces1 */
    IEcoEigenfaces1VTbl* m_pVTblIEcoEigenfaces1;

    /* Счетчик ссылок */
    uint32_t m_cRef;

    /* Интерфейс для работы с памятью */
    IEcoMemoryAllocator1* m_pIMem;

    /* Системный интерфейс */
    IEcoSystem1* m_pISys;

	/* Интерфейс для работы со строкой */
    IEcoString1* m_pIString;

	/* Интерфейсная шина */
    IEcoInterfaceBus1* m_pIBus;

    /* Данные экземпляра */
    float_t* weights;
    float_t* eigenVectors;
    int32_t* labels;
    uint8_t* meanBitmap;

    int32_t size;
    int32_t reducedSize;
    int32_t numberOfEigenVectors;
    int32_t numberLabels;

} CEcoEigenfaces1, *CEcoEigenfaces1Ptr;

/* Инициализация экземпляра */
int16_t initCEcoEigenfaces1(/*in*/ struct IEcoEigenfaces1* me, /* in */ IEcoUnknown *pIUnkSystem);
/* Создание экземпляра */
int16_t createCEcoEigenfaces1(/* in */ IEcoUnknown* pIUnkSystem, /* in */ IEcoUnknown* pIUnkOuter, /* out */ IEcoEigenfaces1** ppIEcoEigenfaces1);
/* Удаление */
void deleteCEcoEigenfaces1(/* in */ IEcoEigenfaces1* pIEcoEigenfaces1);

#endif /* __C_ECOEIGENFACES1_H__ */
