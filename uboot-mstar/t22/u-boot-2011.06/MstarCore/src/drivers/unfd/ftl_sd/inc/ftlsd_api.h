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
#ifndef _FTLSd_API_H_
#define _FTLSd_API_H_

#include "../../inc/common/drvNAND.h"

#include "ftlsd_cfg.h"
#include "ftlsd_utl.h"
#include "ftlsd_dbg.h"
#include "ftlsd_hal.h"
#include "ftlsd_dev.h"
#include "ftlsd_err_codes.h"
#include "ftlsd_core.h"


// ===================================
#define  ERASED_MEM_CONT       0xFF // eMMC ECSD[181]
#define  ERASED_MEM_CONT_U16   (U16)(ERASED_MEM_CONT<<8|ERASED_MEM_CONT)
#define  ERASED_MEM_CONT_U32   (U32)(ERASED_MEM_CONT_U16<<16|ERASED_MEM_CONT_U16)


// ===================================
extern U32  FTLSd_Init(U16 u16_StartPBA, U16 u16_PBACnt);
extern U32  FTLSd_CheckIfReady(void);
extern void FTLSd_ResetReadyFlag(void);
extern U32  FTLSd_GetUID(U8 *pBuf);
extern U32  FTLSd_WriteData(U8* pu8_DataBuf, U32 u32_ByteCnt);
extern U32  FTLSd_ReadData(U8* pu8_DataBuf, U32 u32_ByteCnt);
extern U32  FTLSd_GetDataCapacity(void); 


// ===================================
extern void FTLSd_free(void); // free allocated mem
#endif // _FTLSd_API_H_

