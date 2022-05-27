/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   IEcoEigenfaces1
 * </сводка>
 *
 * <описание>
 *   Данный заголовок описывает интерфейс IEcoEigenfaces1
 * </описание>
 *
 * <ссылка>
 *
 * </ссылка>
 *
 * <автор>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 */

#ifndef __I_ECOEIGENFACES1_H__
#define __I_ECOEIGENFACES1_H__

#include "IEcoBase1.h"
#include "IEcoString1.h"
#include "IdEcoMemoryManager1.h"


/* IEcoEigenfaces1 IID = {E7CE2CE5-7119-4704-8CA6-542C41D93A13} */
#ifndef __IID_IEcoEigenfaces1
static const UGUID IID_IEcoEigenfaces1 = {0x01, 0x10, 0xE7, 0xCE, 0x2C, 0xE5, 0x71, 0x19, 0x47, 0x04, 0x8C, 0xA6, 0x54, 0x2C, 0x41, 0xD9, 0x3A, 0x13};
#endif /* __IID_IEcoEigenfaces1 */

typedef struct IEcoEigenfaces1VTbl {

    /* IEcoUnknown */
    int16_t (*QueryInterface)(/* in */ struct IEcoEigenfaces1* me, /* in */ const UGUID* riid, /* out */ void **ppv);
    uint32_t (*AddRef)(/* in */ struct IEcoEigenfaces1* me);
    uint32_t (*Release)(/* in */ struct IEcoEigenfaces1* me);

    /* IEcoEigenfaces1 */
	int16_t (*Train)(/* in */ struct IEcoEigenfaces1* me, uint8_t** bitmaps, uint32_t bitmapSize, int32_t numImages);
    int16_t (*Save)(/* in */ struct IEcoEigenfaces1* me, int32_t* labels, char_t* filenameEigenVectorsBinary, char_t* filenameWeightsBinary, char_t* filenameLabels);
    int16_t (*Load)(/* in */ struct IEcoEigenfaces1* me, char_t* filenameEigenVectorsBinary, char_t* filenameWeightsBinary, char_t* filenameLabels);
    int16_t (*Predict)(/* in */ struct IEcoEigenfaces1* me, uint8_t* bitmap, uint32_t bitmapSize, int32_t* predictedLabel);


} IEcoEigenfaces1VTbl, *IEcoEigenfaces1VTblPtr;

interface IEcoEigenfaces1 {
    struct IEcoEigenfaces1VTbl *pVTbl;
} IEcoEigenfaces1;


#endif /* __I_ECOEIGENFACES1_H__ */
