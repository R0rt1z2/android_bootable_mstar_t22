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

#include <linux/string.h>
#include "drvNAND.h"
#include "drvNAND_utl.h"


//========================================================
// HAL misc. function definitions
//========================================================
#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
U32  NC_WaitComplete(U16 u16_WaitEvent, U32 u32_MicroSec);

  #define OPTYPE_ERASE        1
  #define OPTYPE_WRITE        2
U32 NC_Toshiba_19nm32GBReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_Toshiba_15nm32GBReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_Samsung_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_Micron_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_SetReadRetryRegValue(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_GetRegDefaultValueDefault(void);
U32 NC_SendReadRetryCmdDefault(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_Hynix_20nm32GBGetRegDefaultValue(void);
U32 NC_Hynix_20nm32GBReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_Hynix_20nm64GBAGetRegDefaultValue(void);
U32 NC_Hynix_20nm64GBAReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);
U32 NC_Hynix_16nm64GB_FDie_GetRegDefaultValue(void);
U32 NC_Hynix_16nm64GB_FDie_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);

U32 NC_Hynix_GetRegDefaultValue(void);
U32 NC_Hynix_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault);


U32 NC_SendCustCmd
(
    U8* pu8_CmdType,
    U8* pu8_CmdValue,
    U32* pu32_AddrRow,
    U32* pu32_AddrCol,
    U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
    U8 u8_CmdCount, U8 u8_CacheOp, U8 u8_plane, U32 u32_PageCnt
);


typedef struct NandReadRetryHal_t
{
	U32 (*NC_GetRegDefaultValue)(void);
	U32(*NC_SendReadRetryCmd)(U8 u8_RetryIndex, U8 u8_SetToDefault);
}NandReadRetryHal_t;

static NandReadRetryHal_t NandReadRetryHal;

void NC_SetupReadRetryCmd(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "Using build-in Read Retry Sequence for Type %d ", pNandDrv->u8_ReadRetryType);

	if(pNandDrv->au8_ID[0] == 0x98)
	{
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "Toshiba MLC\n");
		//Toshiba no need to Get Reg Default Value when initialization
		NandReadRetryHal.NC_GetRegDefaultValue = NC_GetRegDefaultValueDefault;
		switch(pNandDrv->u8_ReadRetryType)
		{
			case 0:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = 6;
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Toshiba_19nm32GBReadRetrySequence;
				break;
			case 1:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = TSB_READRETRY_TIME; 			
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Toshiba_15nm32GBReadRetrySequence;
				break;
			default:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = TSB_READRETRY_TIME;				
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Toshiba_15nm32GBReadRetrySequence;
				break;
		}
	}
	else if(pNandDrv->au8_ID[0] == 0xEC)
	{
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "Samsung MLC\n");			
		//Samsung no need to Get Reg Default Value when initialization
		NandReadRetryHal.NC_GetRegDefaultValue = NC_GetRegDefaultValueDefault;
		switch(pNandDrv->u8_ReadRetryType)
		{
			case 0:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = SAMSUNG_READRETRY_TIME - 1;				
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Samsung_ReadRetrySequence;
				break;
			default:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = SAMSUNG_READRETRY_TIME - 1;				
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Samsung_ReadRetrySequence;
				break;
		}
	}
	else if(pNandDrv->au8_ID[0] == 0x2C)
	{
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "Micron MLC\n");
		//Micron no need to Get Reg Default Value when initialization
		NandReadRetryHal.NC_GetRegDefaultValue = NC_GetRegDefaultValueDefault;
		switch(pNandDrv->u8_ReadRetryType)
		{
			case 0:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = MCIRON_READRETRY_TIME - 1;				
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Micron_ReadRetrySequence;
				break;
			default:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = MCIRON_READRETRY_TIME - 1;
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Micron_ReadRetrySequence;
				break;
		}
	}
	else if(pNandDrv->au8_ID[0] == 0xAD)
	{
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "Hynix MLC\n");

		switch(pNandDrv->u8_ReadRetryType)
		{
			case 0:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = HYNIX20nm32GBMLC_READRETRY_TIME - 1;
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Hynix_20nm32GBReadRetrySequence;
				NandReadRetryHal.NC_GetRegDefaultValue = NC_Hynix_20nm32GBGetRegDefaultValue;
				break;
			case 1:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = HYNIX20nm32GBMLC_READRETRY_TIME - 1;
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Hynix_20nm64GBAReadRetrySequence;
				NandReadRetryHal.NC_GetRegDefaultValue = NC_Hynix_20nm64GBAGetRegDefaultValue;
				break;
            case 2:
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = HYNIX16nm64GBMLC_FDie_READRETRY_TIME;
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Hynix_16nm64GB_FDie_ReadRetrySequence;
				NandReadRetryHal.NC_GetRegDefaultValue = NC_Hynix_16nm64GB_FDie_GetRegDefaultValue;
				break;
			default:				
				pNandDrv->ReadRetry_t.u8_MaxRetryTime = HYNIX20nm32GBMLC_READRETRY_TIME - 1;
				NandReadRetryHal.NC_SendReadRetryCmd = NC_Hynix_ReadRetrySequence;
				NandReadRetryHal.NC_GetRegDefaultValue = NC_Hynix_GetRegDefaultValue;				
				break;
		}		
	}
	else
	{
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "UNKOWN MLC\n");
	
		NandReadRetryHal.NC_GetRegDefaultValue = NC_GetRegDefaultValueDefault;
		NandReadRetryHal.NC_SendReadRetryCmd = NC_SendReadRetryCmdDefault;
	}
}

U32 NC_SendReadRetryCmd(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	return NandReadRetryHal.NC_SendReadRetryCmd(u8_RetryIndex, u8_SetToDefault);
}

U32 NC_GetRegDefaultValue(void)
{
	return NandReadRetryHal.NC_GetRegDefaultValue();
}


U32 NC_GetRegDefaultValueDefault(void)
{
	return UNFD_ST_SUCCESS;
}
U32 NC_SendReadRetryCmdDefault(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	return UNFD_ST_SUCCESS;
}
//==========================TOSHIBA==============================
U8 au8_TSB15nm32GBMLC_ReadRetryValue[TSB_READRETRY_TIME][TSB_REGISTER_NUMBER] = {
	{0,0,0,0},
	{0x2,0x4,0x2,0},
	{0x7c,0,0x7c,0x7c},
	{0x7a,0x7c,0x7a,0x7c},
	{0x78,0,0x78,0x7a},
	{0x7e,0x2,0x7e,0x7a},
	{0x76,0x4,0x76,0x2},
	{0x4,0,0x4,0x78},
	{0x6,0,0x6,0x76},
	{0x74,0x7c,0x74,0x76}
};

U8 au8_TSB19nm32GBMLC_ReadRetryValue[6] = {
    0x00, 0x04, 0x7C, 0x78, 0x74, 0x08
};

U8 au8_TSB19nm32GBMLC_ReadRetryRegister[TSB_REGISTER_NUMBER] = {
    0x4, 0x5, 0x6, 0x7
};

U32 NC_Toshiba_15nm32GBReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8   u8_ValueIdx;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

    if(u8_SetToDefault)
    {
	    u8_ValueIdx = 0;
    }
    else
    {
	    u8_ValueIdx = u8_RetryIndex;
    }

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END);
	NC_ResetFCIE();
	NC_Config();
	// data go through CIFD
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
        ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

    NC_SET_DDR_MODE();

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0xD5);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (0x89 << 8));//LUN address set 0
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
    #if defined(FCIE_EXTEND_tWW) && FCIE_EXTEND_tWW
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_WAIT_IDLE | ACT_WAIT_IDLE);
    #endif

	REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_01|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) | CMD_REG8L);

	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | ACT_RAN_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);

	NC_SetCIFD(&au8_TSB15nm32GBMLC_ReadRetryValue[u8_ValueIdx][0], 0, TSB_REGISTER_NUMBER);


	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, TSB_REGISTER_NUMBER);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); /*offset 0*/

    REG_WRITE_UINT16(NC_CTRL, BIT_NC_CIFD_ACCESS|BIT_NC_JOB_START|BIT_NC_IF_DIR_W);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout RetryIdx %d, ErrCode:%Xh \r\n", u8_RetryIndex, UNFD_ST_ERR_W_TIMEOUT);

	    #if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
	    #else
		nand_stop();
	    #endif
		return UNFD_ST_ERR_SET_FEATURE_TIMEOUT;
	}
	if(u8_SetToDefault ==0)
	{
	    REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	    if(u8_RetryIndex)
	        REG_WRITE_UINT16(NC_AUXREG_DAT, 0xCD);
        else
		    REG_WRITE_UINT16(NC_AUXREG_DAT, 0x26);

        REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
        #if defined(FCIE_EXTEND_tWW) && FCIE_EXTEND_tWW
        REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_WAIT_IDLE | ACT_WAIT_IDLE);
        #endif
        REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | CMD_REG8L);

        REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

  	    if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
  	    {
		    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout RetryIdx %d, ErrCode:%Xh \r\n", u8_RetryIndex, UNFD_ST_ERR_W_TIMEOUT);
            #if 0==IF_IP_VERIFY
		    NC_ResetFCIE();
		    NC_Config();
		    NC_CLR_DDR_MODE();
		    NC_ResetNandFlash();
            #else
		    nand_stop();
            #endif
		    return UNFD_ST_ERR_RST_TIMEOUT;
  	    }
	}
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);

	NC_CLR_DDR_MODE();
	return UNFD_ST_SUCCESS;
}



U32 NC_Toshiba_19nm32GBReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8  u8_idx;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();

	if(u8_SetToDefault)
	{
		return NC_ResetNandFlash();
	}

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END);
	NC_ResetFCIE();
	NC_Config();
	// data go through CIFD
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
	    	~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	nand_clock_setting(FCIE3_SW_SLOWEST_CLK);
	REG_WRITE_UINT16(NC_WIDTH, 0);
	REG_CLR_BITS_UINT16(NC_LATCH_DATA, BIT0|BIT1|BIT2);

	if(u8_RetryIndex == 0)
	{
		//send pre condition
		REG_WRITE_UINT16(NC_AUXREG_ADR, 0x08);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0xC5<<8 | 0x5C);

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_REG8H << 8) | CMD_REG8L);
		REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_BREAK);

		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Pre Condition Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
			#if 0==IF_IP_VERIFY
			NC_ResetFCIE();
			NC_Config();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
			#else
			//nand_stop();
			#endif
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_W_TIMEOUT; // timeout
		}
	}


	REG_WRITE_UINT16(NC_AUXREG_ADR, 0x08);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x55 << 8 | 0x55);

	for(u8_idx = 0; u8_idx < TSB_REGISTER_NUMBER; u8_idx ++)
	{

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
		REG_WRITE_UINT16(NC_AUXREG_DAT, au8_TSB19nm32GBMLC_ReadRetryRegister[u8_idx]);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) | (CMD_REG8L));
		REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_RAN_DOUT);

		NC_SetCIFD(&au8_TSB19nm32GBMLC_ReadRetryValue[u8_RetryIndex], 0, 1);

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);
		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout RetryIdx %d, ErrCode:%Xh \r\n", u8_RetryIndex, UNFD_ST_ERR_W_TIMEOUT);
			#if 0==IF_IP_VERIFY
			NC_ResetFCIE();
			NC_Config();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
			#else
			//nand_stop();
			#endif
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_W_TIMEOUT; // timeout
		}
	}

	REG_WRITE_UINT16(NC_AUXREG_ADR, 0x08);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x5D << 8 | 0x26);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_REG8H << 8) | (CMD_REG8L));
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_BREAK);
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout RetryIdx %d, ErrCode:%Xh \r\n", u8_RetryIndex, UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		#else
		//nand_stop();
		#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);
	nand_clock_setting(pNandDrv->u32_Clk);
	REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

	return UNFD_ST_SUCCESS;
}

//==========================SAMSUNG==============================

//21nm MLC Read Retry Sequence
U8 au8_SSReadRetryRegister[SAMSUNG_REGISTER_NUMBER] = {
	0xA7, 0xA4, 0xA5, 0xA6
};

U8 au8_SSReadRetryValue[SAMSUNG_READRETRY_TIME][SAMSUNG_REGISTER_NUMBER] = {
		{0x00, 0x00, 0x00, 0x00},
		{0x05, 0x0A, 0x00, 0x00},
		{0x28, 0x00, 0xEC, 0xD8},
		{0xED, 0xF5, 0xED, 0xE6},
		{0x0A, 0x0F, 0x05, 0x00},
		{0x0F, 0x0A, 0xFB, 0xEC},
		{0xE8, 0xEF, 0xE8, 0xDC},
		{0xF1, 0xFB, 0xFE, 0xF0},
		{0x0A, 0x00, 0xFB, 0xEC},
		{0xD0, 0xE2, 0xD0, 0xC2},
		{0x14, 0x0F, 0xFB, 0xEC},
		{0xE8, 0xFB, 0xE8, 0xDC},
		{0x1E, 0x14, 0xFB, 0xEC},
		{0xFB, 0xFF, 0xFB, 0xF8},
		{0x07, 0x0C, 0x02, 0x00}
};

U32 NC_Samsung_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8  u8_idx, u8_ValueIdx;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;
	if(u8_SetToDefault)
	{
		u8_ValueIdx = 0;
	}
	else
	{
		u8_ValueIdx = u8_RetryIndex + 1;
	}

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END);
	NC_ResetFCIE();
	NC_Config();
	// data go through CIFD
	if(IF_SPARE_DMA())
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
	    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0xA1 << 8 | 0xA1);

	for(u8_idx = 0; u8_idx < SAMSUNG_REGISTER_NUMBER; u8_idx ++)
	{
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_RAN_DOUT<< 8) | (CMD_REG8L));
		REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_BREAK);

		NC_SetCIFD_Const(0x00, 0, 1);
		NC_SetCIFD(&au8_SSReadRetryRegister[u8_idx], 1, 1);
		NC_SetCIFD(&au8_SSReadRetryValue[u8_ValueIdx][u8_idx], 2, 1);

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 3);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);
		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout RetryIdx %d, ErrCode:%Xh \r\n", u8_RetryIndex, UNFD_ST_ERR_W_TIMEOUT);
			#if 0==IF_IP_VERIFY
			NC_ResetFCIE();
			NC_Config();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
			#else
			//nand_stop();
			#endif
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_W_TIMEOUT; // timeout
		}
		nand_hw_timer_delay(30);		//delay for 30us
	}
	NC_Config();

	return UNFD_ST_SUCCESS;
}

//==========================MCIRON==============================


U8 au8_MicronReadRetryValue[MCIRON_READRETRY_TIME][MICRON_REIGSTER_NUMBER] ={
	{0,0,0,0},
	{1,0,0,0},
	{2,0,0,0},
	{3,0,0,0},
	{4,0,0,0},
	{5,0,0,0},
	{6,0,0,0},
	{7,0,0,0}
};

U32 NC_Micron_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8   u8_ValueIdx;
	#if 0==IF_IP_VERIFY
	volatile U16 u16_Reg;
	#endif
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;	//fix warning	

	if(u8_SetToDefault)
	{
		u8_ValueIdx = 0;
	}
	else
	{
		u8_ValueIdx = u8_RetryIndex + 1;
	}

	// data go through CIFD
		
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END|BIT_MMA_DATA_END);
	NC_ResetFCIE();
	NC_Config();
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
			~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));
	
	NC_SET_DDR_MODE();

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0xEF);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (0x89<<8)|0x89);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ADR_C2T1S0 << 8) | CMD_REG8L);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | ACT_RAN_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);

	NC_SetCIFD(&au8_MicronReadRetryValue[u8_ValueIdx][0], 0, MICRON_REIGSTER_NUMBER);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, MICRON_REIGSTER_NUMBER);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);	/*offset 0*/

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_CIFD_ACCESS|BIT_NC_JOB_START|BIT_NC_IF_DIR_W);

	if (NC_WaitComplete(BIT_NC_JOB_END, DELAY_100ms_in_us) == DELAY_100ms_in_us)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error! timeout!\r\n");

		#if 0==IF_IP_VERIFY
		REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_MIE_EVENT: %Xh \r\n", u16_Reg);
		REG_READ_UINT16(NC_CTRL, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_CTRL: %Xh \r\n", u16_Reg);

		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		#else
		nand_stop();
		#endif
		return UNFD_ST_ERR_SET_FEATURE_TIMEOUT;
	}


	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);

	NC_CLR_DDR_MODE();
	return UNFD_ST_SUCCESS;	// ok
}

//==========================Hynix==============================

U32 NC_Hynix_GetRegDefaultValue(void)
{
	return UNFD_ST_SUCCESS;
}
U32 NC_Hynix_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	return UNFD_ST_SUCCESS;
}

U32 NC_Hynix_GetReadRetryParam(U8* pu8_ReadRetryRegister, int RegisterCount)
{
	U8  u8_idx;
//	U8	au8_ParamValue[8];
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;

	nand_clock_setting(FCIE3_SW_SLOWEST_CLK);
	REG_WRITE_UINT16(NC_WIDTH, 0);
	REG_CLR_BITS_UINT16(NC_LATCH_DATA, BIT0|BIT1|BIT2);

	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x16<<8 | 0x37);
	
	for(u8_idx = 0 ; u8_idx < RegisterCount; u8_idx ++)
	{
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);	
		REG_WRITE_UINT16(NC_AUXREG_DAT, pu8_ReadRetryRegister[u8_idx]);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) | CMD_REG8L);
		REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) |(ACT_RAN_DIN));

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 2);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);

		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 2, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
			NC_ResetFCIE();
			NC_Config();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
		#else
			//nand_stop();
		#endif
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_RST_TIMEOUT; // timeout
		}
		//NC_GetCIFD(&au8_ParamValue[u8_idx], 0, 1);
		//nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "0x%Xh ", au8_ParamValue[u8_idx]);
	}
	
	NC_Config();
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);

	nand_clock_setting(pNandDrv->u32_Clk);
	REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

	return UNFD_ST_SUCCESS;
}

//For 20nm 32Gb MLC

U8 au8_Hynix20nm32GBMLC_ReadRetryRegister[HYNIX20nm32GBMLC_REGISTER_NUMBER] = {
    0xB0,0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7
};

U8 au8_Hynix20nm32GBMLC_ReadRetryValue[HYNIX20nm32GBMLC_READRETRY_TIME][HYNIX20nm32GBMLC_REGISTER_NUMBER];

U8 au8_Hynix20nm32GBMLC_RRIOTPWout[2] = {
	0x00, 0x4D
};

U8	gu8_CurIdx = 0;
U8	gu8_DefaultIdx = 0;

U32 NC_Hynix_20nm32GBGetRegDefaultValue(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32 u32_Row = 0x000200;
	U32 u32_Err;
	U16 u16_i, u16_j;
	U8	*pu8_DataBuf = pNandDrv->pu8_PageDataBuf;
	U8	*pu8_RRTable1, *pu8_RRInvTable1, *pu8_RRTable2, *pu8_RRInvTable2;
	
	//Read R-R Table from OTP BLOCK and Check for inverse version
	// data go through CIFD
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));


	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT,0xAE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_DAT,0xB0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x16<<8 | 0x36);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG9);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x04<<8 | 0x17);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREGA);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x19<<8 | 0x19);

	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB<< 8) | CMD_0xFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( (OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) |CMD_REG8L);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_RAN_DOUT);
	NC_SetCIFD(&au8_Hynix20nm32GBMLC_RRIOTPWout[0], 0, 1);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 1, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
	#if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
	#else
		//nand_stop();
	#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_RST_TIMEOUT; // timeout
	}

	
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_RAN_DOUT << 8) | (OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_1));
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( CMD_REG9L << 8) |CMD_REG8H);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( CMD_REGAL << 8) |CMD_REG9H);

	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_BREAK);

	NC_SetCIFD(&au8_Hynix20nm32GBMLC_RRIOTPWout[1], 0, 1);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 2, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
	#if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
	#else
		//nand_stop();
	#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}
	
	u32_Err = NC_Read_RandomIn(u32_Row, 0, pu8_DataBuf, 260);
	if (u32_Err !=UNFD_ST_SUCCESS)
	{
		return u32_Err;
	}
	
	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H);	
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x38<<8 | 0x38);
	
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB<< 8) | CMD_0xFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB<< 8) | CMD_REG8L);	
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_BREAK);
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 3, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
	#if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
	#else
		//nand_stop();
	#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_RST_TIMEOUT; // timeout
	}
	NC_Config();

//	dump_mem(pu8_DataBuf , 260);
	//parse Read Retry Value
	if(pu8_DataBuf[0] != HYNIX20nm32GBMLC_READRETRY_TIME || pu8_DataBuf[1] != HYNIX20nm32GBMLC_REGISTER_NUMBER)
	{
		 nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Mismatch R-R Count %X:%X or R-R Reg Count %X:%X\n", pu8_DataBuf[0], HYNIX20nm32GBMLC_READRETRY_TIME, 
		 				pu8_DataBuf[1], HYNIX20nm32GBMLC_REGISTER_NUMBER);
		 pNandDrv->u8_RequireReadRetry = 0;
		return UNFD_ST_ERR_SETUP_READ_RETRY_FAIL;
	}
	pu8_RRTable1 = pu8_DataBuf + 2;
	pu8_RRInvTable1 = pu8_RRTable1 + HYNIX20nm32GBMLC_READRETRY_TIME * HYNIX20nm32GBMLC_REGISTER_NUMBER;
	pu8_RRTable2 = pu8_DataBuf + 2 + HYNIX20nm32GBMLC_READRETRY_TIME * HYNIX20nm32GBMLC_REGISTER_NUMBER * 2;
	pu8_RRInvTable2 = pu8_RRTable2 + HYNIX20nm32GBMLC_READRETRY_TIME * HYNIX20nm32GBMLC_REGISTER_NUMBER;

	for(u16_i = 0; u16_i < HYNIX20nm32GBMLC_READRETRY_TIME; u16_i ++)
	{
		for(u16_j = 0; u16_j < HYNIX20nm32GBMLC_REGISTER_NUMBER; u16_j ++)
		{
			if((pu8_RRTable1[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ^
				pu8_RRInvTable1[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j]) ==  0xFF)
			{
				au8_Hynix20nm32GBMLC_ReadRetryValue[u16_i][u16_j] = pu8_RRTable1[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ;
			}
			else
			{
				if((pu8_RRTable2[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ^
				pu8_RRInvTable2[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j]) ==  0xFF)
				{
					au8_Hynix20nm32GBMLC_ReadRetryValue[u16_i][u16_j] = pu8_RRTable2[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ;
				}
				else
				{
					
					 nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Fail Parsing R-R value %X:%X \n", u16_i, u16_j);
					 pNandDrv->u8_RequireReadRetry = 0;
					return UNFD_ST_ERR_SETUP_READ_RETRY_FAIL;
				}
			}
			nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "0x%02X, ", au8_Hynix20nm32GBMLC_ReadRetryValue[u16_i][u16_j] );
		}
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "\n");
	}
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);

	return UNFD_ST_SUCCESS;
}

U32 NC_Hynix_20nm32GBReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8  u8_idx, u8_ValueIdx;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;
	
	NC_Hynix_GetReadRetryParam(au8_Hynix20nm32GBMLC_ReadRetryRegister, HYNIX20nm32GBMLC_REGISTER_NUMBER);
	nand_clock_setting(FCIE3_SW_SLOWEST_CLK);
	REG_WRITE_UINT16(NC_WIDTH, 0);
	REG_CLR_BITS_UINT16(NC_LATCH_DATA, BIT0|BIT1|BIT2);

	if(u8_SetToDefault)
	{
		gu8_CurIdx = u8_ValueIdx = gu8_DefaultIdx;
	}
	else
	{
		if(u8_RetryIndex == 0)
		{
			gu8_DefaultIdx = gu8_CurIdx;
		}
		gu8_CurIdx ++;
		if(gu8_CurIdx == 8)
			gu8_CurIdx = 0;

		u8_ValueIdx = gu8_CurIdx;
	}
	
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
	    	~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x16<<8 | 0x36);

	for(u8_idx = 0 ; u8_idx < HYNIX20nm32GBMLC_REGISTER_NUMBER; u8_idx ++)
	{
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);	
		REG_WRITE_UINT16(NC_AUXREG_DAT,au8_Hynix20nm32GBMLC_ReadRetryRegister[u8_idx]);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) | CMD_REG8L);
		REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_REG8H << 8) |(ACT_RAN_DOUT));
		REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_BREAK);
		NC_SetCIFD(&au8_Hynix20nm32GBMLC_ReadRetryValue[u8_ValueIdx][u8_idx], 0, 1);
		NC_SetCIFD(&au8_Hynix20nm32GBMLC_ReadRetryValue[u8_ValueIdx][u8_idx], 1, 1);
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);

		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 2, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
			NC_ResetFCIE();
			NC_Config();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
		#else
			//nand_stop();
		#endif
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_RST_TIMEOUT; // timeout
		}		
	}
	
	NC_Config();
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);
	nand_clock_setting(pNandDrv->u32_Clk);
	REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

	return UNFD_ST_SUCCESS;
}


//For 20nm 64Gb A die MLC

U8 au8_Hynix20nm64GBMLCA_ReadRetryRegister[HYNIX20nm32GBMLC_REGISTER_NUMBER] = {
    0xCC, 0xBF, 0xAA, 0xAB, 0xCD, 0xAD, 0xAE, 0xAF
};

U8 au8_Hynix20nm64GBMLCA_ReadRetryValue[HYNIX20nm32GBMLC_READRETRY_TIME][HYNIX20nm32GBMLC_REGISTER_NUMBER];

U8 au8_Hynix20nm64GBMLCA_RRIOTPWout[2] = {
	0x40, 0x4D
};

U32 NC_Hynix_20nm64GBAGetRegDefaultValue(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U32 u32_Row = 0x000200;
	U32 u32_Err;
	U16 u16_i, u16_j;
	U8	*pu8_DataBuf = pNandDrv->pu8_PageDataBuf;
	U8	*pu8_RRTable1, *pu8_RRInvTable1, *pu8_RRTable2, *pu8_RRInvTable2;

	//Read R-R Table from OTP BLOCK and Check for inverse version
	// data go through CIFD
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT,0xFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_DAT,0xCC);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x16<<8 | 0x36);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG9);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x04<<8 | 0x17);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREGA);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x19<<8 | 0x19);

	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB<< 8) | CMD_0xFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( (OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) |CMD_REG8L);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_RAN_DOUT);
	NC_SetCIFD(&au8_Hynix20nm32GBMLC_RRIOTPWout[0], 0, 1);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 1, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
	#if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
	#else
		//nand_stop();
	#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_RST_TIMEOUT; // timeout
	}

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_RAN_DOUT << 8) | (OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_1));
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( CMD_REG9L << 8) |CMD_REG8H);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( CMD_REGAL << 8) |CMD_REG9H);

	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_BREAK);

	NC_SetCIFD(&au8_Hynix20nm64GBMLCA_RRIOTPWout[1], 0, 1);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 2, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
	#if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
	#else
		//nand_stop();
	#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}

	u32_Err = NC_Read_RandomIn(u32_Row, 0, pu8_DataBuf, 260);
	if (u32_Err !=UNFD_ST_SUCCESS)
	{
		return u32_Err;
	}
	
	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H);	
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x38<<8 | 0x38);
	
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB<< 8) | CMD_0xFF);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB<< 8) | CMD_REG8L);	
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_BREAK);
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 3, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
	#if 0==IF_IP_VERIFY
		NC_ResetFCIE();
		NC_Config();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
	#else
		//nand_stop();
	#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_RST_TIMEOUT; // timeout
	}
	NC_Config();

//	dump_mem(pu8_DataBuf , 260);
	//parse Read Retry Value
	if(pu8_DataBuf[0] != HYNIX20nm32GBMLC_READRETRY_TIME || pu8_DataBuf[1] != HYNIX20nm32GBMLC_REGISTER_NUMBER)
	{
		 nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Mismatch R-R Count %X:%X or R-R Reg Count %X:%X\n", pu8_DataBuf[0], HYNIX20nm32GBMLC_READRETRY_TIME, 
		 				pu8_DataBuf[1], HYNIX20nm32GBMLC_REGISTER_NUMBER);
		 pNandDrv->u8_RequireReadRetry = 0;
		return UNFD_ST_ERR_SETUP_READ_RETRY_FAIL;
	}
	pu8_RRTable1 = pu8_DataBuf + 2;
	pu8_RRInvTable1 = pu8_RRTable1 + HYNIX20nm32GBMLC_READRETRY_TIME * HYNIX20nm32GBMLC_REGISTER_NUMBER;
	pu8_RRTable2 = pu8_DataBuf + 2 + HYNIX20nm32GBMLC_READRETRY_TIME * HYNIX20nm32GBMLC_REGISTER_NUMBER * 2;
	pu8_RRInvTable2 = pu8_RRTable2 + HYNIX20nm32GBMLC_READRETRY_TIME * HYNIX20nm32GBMLC_REGISTER_NUMBER;

	for(u16_i = 0; u16_i < HYNIX20nm32GBMLC_READRETRY_TIME; u16_i ++)
	{
		for(u16_j = 0; u16_j < HYNIX20nm32GBMLC_REGISTER_NUMBER; u16_j ++)
		{
			if((pu8_RRTable1[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ^
				pu8_RRInvTable1[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j]) ==  0xFF)
			{
				au8_Hynix20nm64GBMLCA_ReadRetryValue[u16_i][u16_j] = pu8_RRTable1[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ;
			}
			else
			{
				if((pu8_RRTable2[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ^
				pu8_RRInvTable2[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j]) ==  0xFF)
				{
					au8_Hynix20nm64GBMLCA_ReadRetryValue[u16_i][u16_j] = pu8_RRTable2[u16_i * HYNIX20nm32GBMLC_REGISTER_NUMBER + u16_j] ;
				}
				else
				{
					nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Fail Parsing R-R value %X:%X \n", u16_i, u16_j);
					pNandDrv->u8_RequireReadRetry = 0;
					return UNFD_ST_ERR_SETUP_READ_RETRY_FAIL;
				}
			}
			nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "0x%02X, ", au8_Hynix20nm64GBMLCA_ReadRetryValue[u16_i][u16_j] );
		}
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "\n");
	}
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);

	return UNFD_ST_SUCCESS;
}


U32 NC_Hynix_20nm64GBAReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8  u8_idx, u8_ValueIdx;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;
	
	NC_Hynix_GetReadRetryParam(au8_Hynix20nm64GBMLCA_ReadRetryRegister, HYNIX20nm32GBMLC_REGISTER_NUMBER);

	nand_clock_setting(FCIE3_SW_SLOWEST_CLK);
	REG_WRITE_UINT16(NC_WIDTH, 0);
	REG_CLR_BITS_UINT16(NC_LATCH_DATA, BIT0|BIT1|BIT2);

	if(u8_SetToDefault)
	{
		gu8_CurIdx = u8_ValueIdx = gu8_DefaultIdx;
	}
	else
	{
		if(u8_RetryIndex == 0)
		{
			gu8_DefaultIdx = gu8_CurIdx;
		}
		gu8_CurIdx ++;
		if(gu8_CurIdx == 8)
			gu8_CurIdx = 0;

		u8_ValueIdx = gu8_CurIdx;
	}

	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H|BIT_NC_WP_AUTO);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x16<<8 | 0x36);
		
	for(u8_idx = 0 ; u8_idx < HYNIX20nm32GBMLC_REGISTER_NUMBER; u8_idx ++)
	{
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);	
		REG_WRITE_UINT16(NC_AUXREG_DAT, au8_Hynix20nm64GBMLCA_ReadRetryRegister[u8_idx]);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) | CMD_REG8L);
		REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_REG8H << 8) |(ACT_RAN_DOUT));
		REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_BREAK);
		NC_SetCIFD(&au8_Hynix20nm64GBMLCA_ReadRetryValue[u8_ValueIdx][u8_idx], 0, 1);
		NC_SetCIFD(&au8_Hynix20nm64GBMLCA_ReadRetryValue[u8_ValueIdx][u8_idx], 1, 1);
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);

		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 2, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
			NC_ResetFCIE();
			NC_Config();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
		#else
			//nand_stop();
		#endif
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_RST_TIMEOUT; // timeout
		}		
	}

	NC_Config();
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);

	nand_clock_setting(pNandDrv->u32_Clk);
	REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

	return UNFD_ST_SUCCESS;
}


U8 gau8_Hynix_16nm64GB_FDie_ReadRetryValue[HYNIX16nm64GBMLC_FDie_READRETRY_TIME][HYNIX16nm64GBMLC_FDie_REGISTER_NUMBER];

U8 gau8_Hynix_16nm64GB_FDie__ReadRetryRegister[HYNIX16nm64GBMLC_FDie_REGISTER_NUMBER] = {
    0x0E, 0x0F, 0x10, 0x11
};
int NumberOfSetBits(U32 i)
{
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}
extern U8 gau8_OneBitCnt[];

U32 NC_Hynix_16nm64GB_FDie_GetRegDefaultValue(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U8 au8_TmpBuf[1];
	U8* pu8_DataBuf = pNandDrv->pu8_PageDataBuf;
	U16 i, j, k, l;
	U8	*pu8_RR, *pu8_InvRR;
	U32 u32_tmp;
	int setbitcount;

	memset(gau8_Hynix_16nm64GB_FDie_ReadRetryValue, 0, sizeof(gau8_Hynix_16nm64GB_FDie_ReadRetryValue));

	NC_ResetFCIE();
	pNandDrv->u16_Reg40_Signal &= ~( BIT_NC_CE_AUTO + BIT_NC_CE_H);
	NC_Config();
	//0xff cmd
	NC_ResetNandFlash();

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x1636);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG9);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x0417);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREGA);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x0019);

//	pNandDrv->u16_Reg48_Spare |= BIT_NC_RANDOM_RW_OP_EN;
	NC_Config();
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	
	//set addr 0xE
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0xE);	
	
	au8_TmpBuf[0] = 0x52;
	NC_SetCIFD(au8_TmpBuf, 0 , 1);

	//cmd 0x36 addr 0xE value 0x52
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ADR_C2T1S0 << 8)|CMD_REG8L);
    REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_RAN_DOUT);
    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
    REG_WRITE_UINT16(NC_AUXREG_DAT, 1);   //cnt
    REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset;
	
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_CIFD_ACCESS | BIT_NC_JOB_START | BIT_NC_IF_DIR_W);
    if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
    {
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "RR OTP command 0x36 fails\n");
        nand_die();
        return UNFD_ST_ERR_RR_INIT_FAIL;
    }

	//cmd 0x16 0x17 0x04 0x19
    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_REG9L << 8)|CMD_REG8H);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_REGAL << 8)|CMD_REG9H);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) | ACT_BREAK);
    REG_WRITE_UINT16(NC_CTRL,  BIT_NC_JOB_START);
    if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
    {
		nand_debug(0, 1, "Error RR OTP command 0x16 0x17 0x04 0x19 fails\n");
        nand_die();
        return UNFD_ST_ERR_RR_INIT_FAIL;
    }
	
//	pNandDrv->u16_Reg48_Spare &=~BIT_NC_RANDOM_RW_OP_EN;
	pNandDrv->u16_Reg48_Spare |=BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
	pNandDrv->u16_Reg50_EccCtrl |= BIT10;
	NC_Config();
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	if(pNandDrv->u8_RequireRandomizer)
		NC_DisableLFSR();

	NC_ReadSectors(0x200, 0, pu8_DataBuf, pNandDrv->pu8_PageSpareBuf,1);
	pNandDrv->u16_Reg48_Spare &=~BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
	pNandDrv->u16_Reg50_EccCtrl &=~BIT10;
	NC_Config();

	NC_ResetNandFlash();

	
//	pNandDrv->u16_Reg48_Spare |= BIT_NC_RANDOM_RW_OP_EN;
	NC_Config();
	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	//set addr 0xE
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0xE);	
	
	au8_TmpBuf[0] = 0x00;
	NC_SetCIFD(au8_TmpBuf, 0 , 1);

	//cmd 0x36 addr 0xE value 0x00
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ADR_C2T1S0 << 8)|CMD_REG8L);
    REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_RAN_DOUT);
    REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
    REG_WRITE_UINT16(NC_AUXREG_DAT, 1);   //cnt
    REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset;
	
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_CIFD_ACCESS | BIT_NC_JOB_START | BIT_NC_IF_DIR_W);
    if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME)
    {
        nand_debug(0, 1, "Error RR OTP command 0x36 fails 2\n");
        nand_die();
        return UNFD_ST_ERR_RR_INIT_FAIL;
    }
	
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (pNandDrv->u8_OpCode_RW_AdrCycle << 8) | CMD_0x00);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_WAITRB << 8) | CMD_0x30);
	REG_WRITE_UINT16(NC_AUXREG_DAT, (ACT_BREAK << 8) | ACT_BREAK);
	REG_WRITE_UINT16(NC_CTRL,  BIT_NC_JOB_START);
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
	{
		nand_debug(0, 1, "Error final read fails\n");
        nand_die();
		return UNFD_ST_ERR_RR_INIT_FAIL;
	}
//	pNandDrv->u16_Reg48_Spare &=~BIT_NC_RANDOM_RW_OP_EN;
	pNandDrv->u16_Reg40_Signal |= ( BIT_NC_CE_AUTO + BIT_NC_CE_H);
	NC_Config();

	NC_ResetFCIE();
	NC_Config();
	NC_ResetNandFlash();

    if (pu8_DataBuf[0] != 8 && pu8_DataBuf[1] != 8)
    {
		nand_debug(0, 1, "Error RR count is not 8 %d, %d\n", pu8_DataBuf[0], pu8_DataBuf[1]);
        nand_die();
    	return UNFD_ST_ERR_RR_INIT_FAIL;
    }

	pu8_RR = &pu8_DataBuf[16];
	pu8_InvRR = &pu8_DataBuf[48];		//64
	for(i = 0; i < 8; i ++)	//loop RR times
    {    
	    for (j = 0; j < 4; j++)		//loop register
    	{
			for(k = 0; k < 8; k ++)	//loop over bits
			{
				u32_tmp = 0;
				for(l = 0 ; l < 8; l ++) //loop over RRT sets
				{
					u32_tmp |= ((pu8_RR[l * 64 + i *4 + j] >> k)&0x1) << l;					
				}
                #if 0
				setbitcount = NumberOfSetBits(u32_tmp);
                #else
                setbitcount = gau8_OneBitCnt[u32_tmp&0xFF];
                #endif
				if(setbitcount > 4)
					gau8_Hynix_16nm64GB_FDie_ReadRetryValue[i][j]|= 1<<k;
				else if(setbitcount == 4)
				{
					u32_tmp = 0;
					for(l = 0 ; l < 8; l ++) //loop over InvRRT sets
					{
						u32_tmp |= ((pu8_InvRR[l * 64 + i *4 + j] >> k)&0x1) << l;
					}
					
                    #if 0
					setbitcount = NumberOfSetBits(u32_tmp);
                    #else
                    setbitcount = gau8_OneBitCnt[u32_tmp&0xFF];
                    #endif					
					if(setbitcount < 4)
						gau8_Hynix_16nm64GB_FDie_ReadRetryValue[i][j]|= 1<<k;
					else if(setbitcount == 4)
					{
						nand_debug(0, 1, "Parsing RR table fail\n");
                        nand_die();
						return UNFD_ST_ERR_RR_INIT_FAIL;	
					}
				}
			}
			//nand_debug(0, 0," %X,", gau8_Hynix_16nm64GB_FDie_ReadRetryValue[i][j]);
    	}
		//nand_debug(0, 0, "\n");
    }
    return UNFD_ST_SUCCESS;
}

U32 NC_Hynix_16nm64GB_FDie_ReadRetrySequence(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	U8  u8_idx, u8_ValueIdx;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;

    if(u8_SetToDefault)
        return UNFD_ST_SUCCESS;
    
	//NC_Hynix_GetReadRetryParam(gau8_Hynix_16nm64GB_FDie__ReadRetryRegister, HYNIX16nm64GBMLC_F_DIE_REGISTER_NUMBER);

	nand_clock_setting(FCIE3_SW_SLOWEST_CLK);
	REG_WRITE_UINT16(NC_WIDTH, 0);
	REG_CLR_BITS_UINT16(NC_LATCH_DATA, BIT0|BIT1|BIT2);

	if(u8_SetToDefault)
	{
		gu8_CurIdx = u8_ValueIdx = gu8_DefaultIdx;
	}
	else
	{
		if(u8_RetryIndex == 0)
		{
			gu8_DefaultIdx = gu8_CurIdx;
		}
		gu8_CurIdx ++;
		if(gu8_CurIdx == 8)
			gu8_CurIdx = 0;

		u8_ValueIdx = gu8_CurIdx;
	}

	if(IF_SPARE_DMA())
		REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare &
		    ~(BIT_NC_SPARE_DEST_MIU|BIT_NC_RANDOM_RW_OP_EN));

	REG_CLR_BITS_UINT16(NC_SIGNAL, BIT_NC_CE_AUTO|BIT_NC_CE_H|BIT_NC_WP_AUTO);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_CMDREG8);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0x16<<8 | 0x36);
		
	for(u8_idx = 0 ; u8_idx < HYNIX16nm64GBMLC_FDie_REGISTER_NUMBER; u8_idx ++)
	{
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);	
		REG_WRITE_UINT16(NC_AUXREG_DAT, gau8_Hynix_16nm64GB_FDie__ReadRetryRegister[u8_idx]);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ((OP_ADR_CYCLE_00|OP_ADR_TYPE_COL|OP_ADR_SET_0) << 8) | CMD_REG8L);
		REG_WRITE_UINT16(NC_AUXREG_DAT, (CMD_REG8H << 8) |(ACT_RAN_DOUT));
		REG_WRITE_UINT16(NC_AUXREG_DAT, ( ACT_BREAK << 8) |ACT_BREAK);
		NC_SetCIFD(&gau8_Hynix_16nm64GB_FDie_ReadRetryValue[u8_ValueIdx][u8_idx], 0, 1);
		NC_SetCIFD(&gau8_Hynix_16nm64GB_FDie_ReadRetryValue[u8_ValueIdx][u8_idx], 1, 1);
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 1);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset
		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_CIFD_ACCESS);

		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout 2, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
			NC_ResetFCIE();
			NC_Config();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
		#else
			//nand_stop();
		#endif
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_RST_TIMEOUT; // timeout
		}		
	}

	NC_Config();
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);

	nand_clock_setting(pNandDrv->u32_Clk);
	REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);
	//for debug
	//NC_Hynix_GetReadRetryParam(gau8_Hynix_16nm64GB_F_Die_ReadRetryRegister, HYNIX16nm64GBMLC_F_DIE_REGISTER_NUMBER);

	return UNFD_ST_SUCCESS;
}



U32 NC_GetReadRetryRegValue(U8** ppu8_RegisterValue)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U16 u16_i, u16_j = 0, u16_k = 0;
	U32 au32_AddrRow[4];
	U32 au32_AddrCol[4];
	U32 u32_Err = 0;
	
	for(u16_i = 0; u16_i < pNandDrv->ReadRetry_t.u8_CustRegNo; u16_i ++)
	{
		u16_k = 0;
		for(u16_j = 0; u16_j < pNandDrv->ReadRetry_t.u8_GetCmdLen; u16_j ++)
			if(pNandDrv->ReadRetry_t.pu8_GetCmdTypeSeq[u16_j] == ADDR_CYCLE_CMD)
			{
				au32_AddrRow[u16_k] = 0;
				au32_AddrCol[u16_k] = pNandDrv->ReadRetry_t.pu8_CustRegTable[u16_i];
				u16_k ++;
			}
		
		u32_Err = NC_SendCustCmd(pNandDrv->ReadRetry_t.pu8_GetCmdTypeSeq, pNandDrv->ReadRetry_t.pu8_GetCmdValueSeq, au32_AddrRow, au32_AddrCol,
			pNandDrv->pu8_PageDataBuf, pNandDrv->pu8_PageSpareBuf, pNandDrv->ReadRetry_t.u8_GetCmdLen, 0, 1, 1);
		
		
		for(u16_j = 0; u16_j < pNandDrv->ReadRetry_t.u8_ByteLenPerCmd; u16_j ++)
			ppu8_RegisterValue[u16_i][u16_j] = pNandDrv->pu8_PageDataBuf[u16_j];
	}
	return u32_Err;
}

U32 NC_SetReadRetryRegValue(U8 u8_RetryIndex, U8 u8_SetToDefault)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	U16 u16_i, u16_j, u16_k;
	U32 u32_Err = 0;
	U32 au32_AddrRow[4];
	U32 au32_AddrCol[4];
	U8** ppu8_RegValue;
	
	if(u8_RetryIndex >= pNandDrv->ReadRetry_t.u8_MaxRetryTime)
	{
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Retry Index should be smaller than MaxRetryTime\n");
		return UNFD_ST_ERR_INVALID_ADDR;
	}
	ppu8_RegValue = (U8**)malloc(pNandDrv->ReadRetry_t.u8_CustRegNo * sizeof(U8*) +
						pNandDrv->ReadRetry_t.u8_CustRegNo * (pNandDrv->ReadRetry_t.u8_ByteLenPerCmd + 1) *sizeof(U8));
	
	if(!ppu8_RegValue)
		return UNFD_ST_ERR_INVALID_ADDR;
	
	for(u16_i = 0; u16_i < pNandDrv->ReadRetry_t.u8_CustRegNo; u16_i ++)
		ppu8_RegValue[u16_i] = ((U8*)(ppu8_RegValue + pNandDrv->ReadRetry_t.u8_CustRegNo)
	    								+ u16_i * (pNandDrv->ReadRetry_t.u8_ByteLenPerCmd + 1) *sizeof(U8));

	if(pNandDrv->ReadRetry_t.u8_DefaultValueOffset)
	{
		NC_GetReadRetryRegValue(ppu8_RegValue);
	}
	for(u16_i = 0; u16_i < pNandDrv->ReadRetry_t.u8_CustRegNo; u16_i ++)
	{
		u16_k = 0;
		for(u16_j = 0; u16_j < pNandDrv->ReadRetry_t.u8_GetCmdLen; u16_j ++)
			if(pNandDrv->ReadRetry_t.pu8_GetCmdTypeSeq[u16_j] == ADDR_CYCLE_CMD)
			{
				au32_AddrRow[u16_k] = 0;
				au32_AddrCol[u16_k] = pNandDrv->ReadRetry_t.pu8_CustRegTable[u16_i];
				u16_k ++;
			}
		u16_j = 0;
		if(u16_k == 0)
		{
			pNandDrv->pu8_PageDataBuf[u16_j] = 0;
			pNandDrv->pu8_PageDataBuf[u16_j + 1] = pNandDrv->ReadRetry_t.pu8_CustRegTable[u16_i];
			u16_j +=2;
		}
		
		for(u16_k = 0; u16_k < pNandDrv->ReadRetry_t.u8_ByteLenPerCmd; u16_k ++)
		{
			if(!u8_SetToDefault)
			{
				if(pNandDrv->ReadRetry_t.u8_DefaultValueOffset)
				{
					if(pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][0] == 0xFF)
					{
						pNandDrv->pu8_PageDataBuf[u16_j] =
							pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][u16_k + 1];
						u16_j ++;
					}
					else if(pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][0] == 0)
					{
						pNandDrv->pu8_PageDataBuf[u16_j] = ppu8_RegValue[u16_i][u16_k] +
							pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][u16_k + 1];
						u16_j ++;
					}
					else if(pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][0] == 1)
					{
						pNandDrv->pu8_PageDataBuf[u16_j] = ppu8_RegValue[u16_i][u16_k] -
						    pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][u16_k + 1];
						u16_j ++;
					}
					else
					{
						free(ppu8_RegValue);
						return UNFD_ST_ERR_INVALID_PARAM;
					}
				}
				else
				{
					if(pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][0] == 0xFF)
					{
						pNandDrv->pu8_PageDataBuf[u16_j] = pNandDrv->ReadRetry_t.pppu8_RetryRegValue[u8_RetryIndex][u16_i][u16_k + 1];
						u16_j ++;
					}
					else
					{
						free(ppu8_RegValue);
						return UNFD_ST_ERR_INVALID_PARAM;
					}
				}
			}
			else
			{
				pNandDrv->pu8_PageDataBuf[u16_j] = pNandDrv->ppu8_ReadRetryDefault[u16_i][u16_k];
				u16_j ++;
			}
		}
		u32_Err = NC_SendCustCmd(pNandDrv->ReadRetry_t.pu8_SetCmdTypeSeq, pNandDrv->ReadRetry_t.pu8_SetCmdValueSeq, au32_AddrRow, au32_AddrCol,
			pNandDrv->pu8_PageDataBuf, pNandDrv->pu8_PageSpareBuf, pNandDrv->ReadRetry_t.u8_SetCmdLen, 0, 1, 1);
	}
	
	free(ppu8_RegValue);
	return u32_Err;
}

#endif // NC_SEL_FCIE3
