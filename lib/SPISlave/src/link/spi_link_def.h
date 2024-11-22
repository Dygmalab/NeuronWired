
/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2022  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __SPI_LINK_DEF_H_
#define __SPI_LINK_DEF_H_

#include "dl_middleware.h"

/* SPI link layer message types */
typedef uint8_t spil_mess_type_t;

#define SPIL_MESS_TYPE_MASTER_DATA_SEND_START      0x01
#define SPIL_MESS_TYPE_MASTER_DATA_RECV_START      0x02

#define SPIL_MESS_TYPE_DATA                 0x03

#define SPIL_MESS_TYPE_RESULT_OK            0x80
#define SPIL_MESS_TYPE_RESULT_ERR           0x81
#define SPIL_MESS_TYPE_RESULT_READY         0x82
#define SPIL_MESS_TYPE_RESULT_OK_BUSY       0x83    /* The command has succeeded, the line is busy now. */
#define SPIL_MESS_TYPE_RESULT_BUSY          0x84
#define SPIL_MESS_TYPE_RESULT_DATA_READY    0x85

#define SPIL_MESS_TYPE_RESULT_IGNORED_FF    0xFF    /* The Slave was (probably) busy on the SPI line, thus the last message was not received and was ignored */
#define SPIL_MESS_TYPE_RESULT_IGNORED_00    0x00    /* The Slave was (probably) disconnected on the SPI line, thus the last message was not received and was ignored */


typedef struct
{
    uint8_t len;
    spil_mess_type_t type;
} PACK spil_mess_header_t;

typedef struct
{
    spil_mess_header_t head;
} PACK spil_mess_master_data_send_start_t;

typedef struct
{
    spil_mess_header_t head;
} PACK spil_mess_master_data_recv_start_t;

typedef struct
{
    spil_mess_header_t head;
    uint8_t data[];
} PACK spil_mess_data_t;

typedef struct
{
    spil_mess_header_t head;
} PACK spil_mess_result_t;



#endif /* __SPI_LINK_DEF_H_ */
