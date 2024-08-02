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

#include "avtp/acf/Most.h"
#include "avtp/Utils.h" 

/**
 * This table describes all the offsets of the ACF Most header fields.
 */
static const Avtp_FieldDescriptor_t Avtp_MostFieldDesc[AVTP_MOST_FIELD_MAX] =
{
    /* ACF common header fields */
    [AVTP_MOST_FIELD_ACF_MSG_TYPE]      = { .quadlet = 0, .offset = 0, .bits = 7 },
    [AVTP_MOST_FIELD_ACF_MSG_LENGTH]    = { .quadlet = 0, .offset = 7, .bits = 9 },  
    /* ACF CAN header fields */
    [AVTP_MOST_FIELD_PAD]               = { .quadlet = 0, .offset = 16, .bits = 2 },
    [AVTP_MOST_FIELD_MTV]               = { .quadlet = 0, .offset = 18, .bits = 1 },
    [AVTP_MOST_FIELD_MOST_NET_ID]       = { .quadlet = 0, .offset = 19, .bits = 5 },
    [AVTP_MOST_FIELD_RESERVED]          = { .quadlet = 0, .offset = 24, .bits = 8 },
    [AVTP_MOST_FIELD_MESSAGE_TIMESTAMP] = { .quadlet = 1, .offset = 0, .bits = 64 },
    [AVTP_MOST_FIELD_DEVICE_ID]         = { .quadlet = 3, .offset = 0, .bits = 16 },
    [AVTP_MOST_FIELD_FBLOCK_ID]         = { .quadlet = 3, .offset = 16, .bits = 8 },
    [AVTP_MOST_FIELD_INST_ID]           = { .quadlet = 3, .offset = 24, .bits = 8 },
    [AVTP_MOST_FIELD_FUNC_ID]           = { .quadlet = 4, .offset = 0, .bits = 12 },
    [AVTP_MOST_FIELD_OP_TYPE]           = { .quadlet = 4, .offset = 12, .bits = 4 },
    [AVTP_MOST_FIELD_RESERVED_2]        = { .quadlet = 4, .offset = 16, .bits = 16 },
};

int Avtp_Most_Init(Avtp_Most_t* pdu)
{
    if(!pdu) {
        return -EINVAL;
    }

    memset(pdu, 0, sizeof(Avtp_Most_t));  
    Avtp_Most_SetField(pdu, AVTP_MOST_FIELD_ACF_MSG_TYPE, AVTP_ACF_TYPE_MOST);
    
    return 0;
}

int Avtp_Most_GetField(Avtp_Most_t* pdu, Avtp_MostFields_t field, uint64_t* value)
{
    return Avtp_GetField(Avtp_MostFieldDesc, AVTP_MOST_FIELD_MAX, (uint8_t*)pdu, (uint8_t)field, value);
}

int Avtp_Most_SetField(Avtp_Most_t* pdu, Avtp_MostFields_t field, uint64_t value)
{
    return Avtp_SetField(Avtp_MostFieldDesc, AVTP_MOST_FIELD_MAX, (uint8_t*)pdu, (uint8_t)field, value);
}
