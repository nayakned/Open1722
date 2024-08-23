/*
 * Copyright (c) 2024, COVESA
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of COVESA nor the names of its contributors may be 
 *      used to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>
#include "avtp/CommonHeader.h"
#include "avtp/Utils.h" 
#include "avtp/Defines.h"

/**
 * This table maps all IEEE 1722 common header fields to a descriptor.
 */
static const Avtp_FieldDescriptor_t Avtp_CommonHeaderFieldDesc[AVTP_COMMON_HEADER_FIELD_MAX] = {
    /* Common AVTP header */
    [AVTP_COMMON_HEADER_FIELD_SUBTYPE]            = { .quadlet = 0, .offset = 0, .bits = 8 },
    [AVTP_COMMON_HEADER_FIELD_H]                  = { .quadlet = 0, .offset = 8, .bits = 1 },
    [AVTP_COMMON_HEADER_FIELD_VERSION]            = { .quadlet = 0, .offset = 9, .bits = 3 },
};

uint64_t Avtp_CommonHeader_GetField(Avtp_CommonHeader_t* avtp_pdu,
        Avtp_CommonHeaderField_t field)
{
    return Avtp_GetField(
            Avtp_CommonHeaderFieldDesc,
            AVTP_COMMON_HEADER_FIELD_MAX,
            (uint8_t*)avtp_pdu,
            (uint8_t)field);
}

uint8_t Avtp_CommonHeader_GetSubtype(Avtp_CommonHeader_t* avtp_pdu)
{
    return Avtp_GetField(
            Avtp_CommonHeaderFieldDesc,
            AVTP_COMMON_HEADER_FIELD_MAX,
            (uint8_t*)avtp_pdu,
            AVTP_COMMON_HEADER_FIELD_SUBTYPE);
}

uint8_t Avtp_CommonHeader_GetH(Avtp_CommonHeader_t* avtp_pdu)
{
    return Avtp_GetField(
            Avtp_CommonHeaderFieldDesc,
            AVTP_COMMON_HEADER_FIELD_MAX,
            (uint8_t*)avtp_pdu,
            AVTP_COMMON_HEADER_FIELD_H);
}

uint8_t Avtp_CommonHeader_GetVersion(Avtp_CommonHeader_t* avtp_pdu)
{
    return Avtp_GetField(
            Avtp_CommonHeaderFieldDesc,
            AVTP_COMMON_HEADER_FIELD_MAX,
            (uint8_t*)avtp_pdu,
            AVTP_COMMON_HEADER_FIELD_VERSION);
}

int Avtp_CommonHeader_SetField(Avtp_CommonHeader_t* avtp_pdu,
        Avtp_CommonHeaderField_t field, uint64_t value)
{
    return Avtp_SetField(
            Avtp_CommonHeaderFieldDesc,
            AVTP_COMMON_HEADER_FIELD_MAX,
            (uint8_t*)avtp_pdu,
            (uint8_t)field,
            value);        
}

/******************************************************************************
 * Legacy API
 *****************************************************************************/
int avtp_pdu_get(const struct avtp_common_pdu *pdu, Avtp_CommonHeaderField_t field,
                                uint32_t *val)
{
    if (val == NULL) {
        return -EINVAL;
    } else {
        uint64_t temp = Avtp_CommonHeader_GetField((Avtp_CommonHeader_t*) pdu, field);
        *val = (uint32_t)temp;
        return 0;
    }
}

int avtp_pdu_set(struct avtp_common_pdu *pdu, Avtp_CommonHeaderField_t field,
                                uint32_t value)
{
    return Avtp_CommonHeader_SetField((Avtp_CommonHeader_t*) pdu, field, value);
}
