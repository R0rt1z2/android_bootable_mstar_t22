/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    drvSPINAND.c
/// @brief  SPI NAND Flash Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/string.h>
// Common Definition
#include "MsCommon.h"
#include "MsVersion.h"
#include "MsOS.h"

// Internal Definition
#include "../../inc/common/drvSPINAND.h"
#include "../../inc/common/drvSPICMD.h"
#include "drvBDMA.h"
#include "drvMMIO.h"
#if defined(MSOS_TYPE_LINUX)
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#endif

//==========================================================================
// Define
//==========================================================================

//==========================================================================
// Global Variable
//==========================================================================
U8 u8MagicData[] = {0x4D, 0x53, 0x54, 0x41, 0x52, 0x53, 0x45, 0x4D, 0x49, 0x55, 0x53, 0x46, 0x44, 0x43, 0x49, 0x53};

SPINAND_FLASH_INFO_t gtSpiNandInfoTable[]=
{    //u8_IDByteCnt           au8_ID                u16_SpareByteCnt   u16_PageByteCnt   u16_BlkPageCnt   u16_BlkCnt  u16_SectorByteCnt
    {2, {MID_GD     , 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 4096, 512, 0},
    {2, {MID_GD     , 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
    {2, {MID_GD     , 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
    {2, {MID_GD     , 0xD1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
    {2, {MID_GD     , 0xD2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 2048, 512, 0},
    {2, {MID_MICRON , 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 2},
    {2, {MID_MICRON , 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 2048, 512, 2},
    {2, {MID_MICRON , 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 4096, 512, 2},
    {2, {MID_ATO    , 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
    {2, {MID_WINBOND, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
    {2, {MID_MXIC   , 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
    {2, {MID_MXIC   , 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 2048, 512, 0},
    {2, {0x00       , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  0,    0,  0,    0,   0, 0},
};

//==========================================================================
// Global Function
//==========================================================================
static BOOL _MDrv_SPINAND_GET_INFO(void)
{
    U32 u32Ret;
    U8 u8Spare[16];
    U8 u8Data[512];
    U8 u8Idx, u8Status;
    U8 *u8SrcAddr, *u8DstAddr;
    U16 u16PageIndex;
    u8SrcAddr = u8Data;
    u8DstAddr = (U8*)(&_gtSpinandInfo);
    RIU_FLAG = TRUE;
    // if ecc error read back up block(block2 ,4, 6, 8)
    for(u8Idx = 0; u8Idx < 10; u8Idx+=2)
    {
        u16PageIndex = 64 * u8Idx;
        //read data to cache first
        u32Ret = HAL_SPINAND_RFC(u16PageIndex, &u8Status);
        if(u32Ret != ERR_SPINAND_SUCCESS || u8Status & ECC_STATUS_ERR)
            continue;

        // Read SPINand Data
        u32Ret = HAL_SPINAND_Read (0, 512, u8SrcAddr);
        if(u32Ret != ERR_SPINAND_SUCCESS)
            continue;

        // Read SPINand Spare Data
        u32Ret = HAL_SPINAND_Read(2048, 16, u8Spare);
        if(u32Ret != ERR_SPINAND_SUCCESS)
            continue;

        if(memcmp((const void *) u8SrcAddr, (const void *) u8MagicData, sizeof(u8MagicData)) != 0)
            continue;

        u8SrcAddr += sizeof(u8MagicData);

        memcpy(u8DstAddr, u8SrcAddr, sizeof(SPINAND_FLASH_INFO_t));
        {
            U8 u8ID[_gtSpinandInfo.u8_IDByteCnt];
            HAL_SPINAND_ReadID(_gtSpinandInfo.u8_IDByteCnt, u8ID);
            if(memcmp((const void *) _gtSpinandInfo.au8_ID, (const void *)u8ID, _gtSpinandInfo.u8_IDByteCnt) != 0)
                continue;
        }
        return TRUE;
    }

    return FALSE;    
}

BOOL MDrv_SPINAND_Init(SPINAND_FLASH_INFO_t *tSpinandInfo)
{
    #define SPINAND_ID_SIZE 2
    U8 u8ID[SPINAND_ID_SIZE];
    U32 u32Index;
    U32 u32Ret;
    BOOL bReFind = FALSE;
    // 1. HAL init
    //
    _u8SPINANDDbgLevel = E_SPINAND_DBGLV_DEBUG;

MDrv_SPINAND_Init_Detect_ID:

    u32Ret = HAL_SPINAND_Init();
    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        printf("Init SPI NAND fail!!!!\r\n");
        tSpinandInfo->au8_ID[0] = 0xFF;
        tSpinandInfo->au8_ID[1] = 0xFF;
        tSpinandInfo->u8_IDByteCnt = 2;
        if(!bReFind)
        {
            bReFind = TRUE;
            HAL_SPINAND_CSCONFIG();
            goto MDrv_SPINAND_Init_Detect_ID;
        }
        return FALSE;
    }

    if(!_MDrv_SPINAND_GET_INFO())
    {
        u32Ret = HAL_SPINAND_ReadID(SPINAND_ID_SIZE, u8ID);
        if(u32Ret != ERR_SPINAND_SUCCESS)
        {
            printf("Can't not Detect SPINAND Device!!!!\r\n");
            tSpinandInfo->au8_ID[0] = 0xFF;
            tSpinandInfo->au8_ID[1] = 0xFF;
            tSpinandInfo->u8_IDByteCnt = 2;
            return FALSE;
        }

        printf("MID =%x, DID =%x \r\n",u8ID[0], u8ID[1]);
       

        for (u32Index = 0; gtSpiNandInfoTable[u32Index].au8_ID[0] != 0; u32Index++)
        {
            if(gtSpiNandInfoTable[u32Index].au8_ID[0] == u8ID[0] && 
               gtSpiNandInfoTable[u32Index].au8_ID[1] == u8ID[1])
            {
                printf("SPINAND Device DETECT\r\n");
                memcpy(tSpinandInfo, &gtSpiNandInfoTable[u32Index], sizeof(SPINAND_FLASH_INFO_t));
                memcpy(&_gtSpinandInfo, &gtSpiNandInfoTable[u32Index], sizeof(SPINAND_FLASH_INFO_t));
                break;
            }
        }
        if((!gtSpiNandInfoTable[u32Index].au8_ID[0]) && (!gtSpiNandInfoTable[u32Index].au8_ID[1]))
        {
            printf("Can't not Detect SPINAND Device!!!!\r\n");
            tSpinandInfo->au8_ID[0] = u8ID[0];
            tSpinandInfo->au8_ID[1] = u8ID[1];
            tSpinandInfo->u8_IDByteCnt = 2;
            if(!bReFind)
            {
                bReFind = TRUE;
                HAL_SPINAND_CSCONFIG();
                goto MDrv_SPINAND_Init_Detect_ID;
            }

            return FALSE;
        }
    }
    else
    {
        memcpy(tSpinandInfo, &_gtSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));
    }

    //HAL_SPINAND_SetCKG(CLKCFG);
    return TRUE;
}

BOOL MDrv_SPINAND_ForceInit(SPINAND_FLASH_INFO_t *tSpinandInfo)
{
    memcpy(&_gtSpinandInfo, tSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
// Read SPINAND Data
// @param u32_PageIdx : page index of read data in specific block
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Enable ISP engine, the XIU mode does not work
//-------------------------------------------------------------------------------------------------

U32 MDrv_SPINAND_Read(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    U8 u8Status;
    U32 u32Ret = ERR_SPINAND_SUCCESS;

    //read data to cache first
    u32Ret = HAL_SPINAND_RFC(u32_PageIdx, &u8Status);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    //DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printf("u8Status %x\r\n", u8Status));

    HAL_SPINAND_PLANE_HANDLER(u32_PageIdx);

    // Read SPINand Data
    u32Ret = HAL_SPINAND_Read (0x0, PAGE_SIZE, u8Data);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    //printf("READ SPARE DATA\r\n");
    // Read SPINand Spare Data
    u32Ret= HAL_SPINAND_Read(PAGE_SIZE, SPARE_SIZE, pu8_SpareBuf);
    if(u32Ret == ERR_SPINAND_SUCCESS){
        if(u8Status & ECC_STATUS_ERR)
            u32Ret = ERR_SPINAND_ECC_ERROR;
        else if(u8Status & ECC_STATUS_BITFLIP)
            u32Ret = ERR_SPINAND_ECC_BITFLIP;
    }
	
    return u32Ret;
}

//-------------------------------------------------------------------------------------------------
// Read SPINAND Data From Random column address
// @param u32_PageIdx : page index of read data in specific block
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Enable ISP engine, the XIU mode does not work
//-------------------------------------------------------------------------------------------------
U32 MDrv_SPINAND_Read_RandomIn(U32 u32_PageIdx, U32 u32_Column, U32 u32_Byte, U8 *u8Data)
{
    U8 u8Status;
    U32 u32Ret = ERR_SPINAND_SUCCESS;
    
    //read data to cache first
    u32Ret = HAL_SPINAND_RFC(u32_PageIdx, &u8Status);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    //DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("u8Status %x\r\n", u8Status));

    HAL_SPINAND_PLANE_HANDLER(u32_PageIdx);

    // Read SPINand Data
    u32Ret = HAL_SPINAND_Read (u32_Column, u32_Byte, u8Data);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    if(u32Ret == ERR_SPINAND_SUCCESS){
        if(u8Status & ECC_STATUS_ERR)
            u32Ret = ERR_SPINAND_ECC_ERROR;
        else if(u8Status & ECC_STATUS_BITFLIP)
            u32Ret = ERR_SPINAND_ECC_BITFLIP;
    }

    return u32Ret;
}


U32 MDrv_SPINAND_SetMode(SPINAND_MODE eMode)
{
    return HAL_SPINAND_SetMode(eMode);
}

U32 MDrv_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    return HAL_SPINAND_Write(u32_PageIdx, u8Data, pu8_SpareBuf);
}

U8 MDrv_SPINAND_ReadID(U16 u16Size, U8 *u8Data)
{
   return HAL_SPINAND_ReadID(u16Size, u8Data);
}

U32 MDrv_SPINAND_BLOCK_ERASE(U32 u32_PageIdx)
{
    return HAL_SPINAND_BLOCKERASE(u32_PageIdx);
}

void SpiNandMain(unsigned int dwSramAddress, unsigned int dwSramSize)
{
    U8 u8Data[2];
    MDrv_SPINAND_ReadID(2,u8Data);
}

