/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   IdEcoEigenfaces1
 * </сводка>
 *
 * <описание>
 *   Данный заголовок описывает интерфейс IdEcoEigenfaces1
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

#ifndef __ID_ECOEIGENFACES1_H__
#define __ID_ECOEIGENFACES1_H__

#include "IEcoBase1.h"
#include "IEcoEigenfaces1.h"

/* EcoEigenfaces1 CID = {E9E9C1DF-6AF4-4C26-8A97-FE52AFDAB124} */
#ifndef __CID_EcoEigenfaces1
static const UGUID CID_EcoEigenfaces1 = {0x01, 0x10, 0xE9, 0xE9, 0xC1, 0xDF, 0x6A, 0xF4, 0x4C, 0x26, 0x8A, 0x97, 0xFE, 0x52, 0xAF, 0xDA, 0xB1, 0x24};
#endif /* __CID_EcoEigenfaces1 */

/* Фабрика компонента для динамической и статической компановки */
#ifdef ECO_DLL
ECO_EXPORT IEcoComponentFactory* ECOCALLMETHOD GetIEcoComponentFactoryPtr();
#elif ECO_LIB
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_E9E9C1DF6AF44C268A97FE52AFDAB124;
#endif

#endif /* __ID_ECOEIGENFACES1_H__ */
