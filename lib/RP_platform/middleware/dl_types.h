#ifndef __DL_TYPES_H_
#define __DL_TYPES_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//#include <stdlib.h>

#define INLINE inline
#define VOLATILE volatile
#define PACK __attribute__((__packed__))

typedef bool bool_t;
typedef char char_t;

typedef enum
{
    RESULT_OK = 0,
    RESULT_ERR,
    RESULT_BUSY,
    RESULT_INCOMPLETE,
} result_t;

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

typedef enum
{
    SCANNER_STATE_INITIALIZING = 0x01,
    SCANNER_STATE_RUNNING = 0x02,
    SCANNER_STATE_SLEEPING = 0x03,
    SCANNER_STATE_POWER_SAVING = 0x04,
    SCANNER_STATE_FIRMWARE_UPGRADE = 0x05,

    SCANNER_STATE_FAILURE = 0xFF,
} scanner_state_t;

typedef enum
{
    SCANNER_SIDE_LEFT = 1,
    SCANNER_SIDE_RIGHT,
} scanner_side_t;

typedef struct
{
    uint8_t led_id;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} PACK drwp_led_setup_t;

typedef struct
{
    uint8_t bitmap[5];
} PACK buttons_status_t;

typedef struct deviceid64 { uint8_t id[8]; }PACK deviceid64_t;
typedef struct ble_deviceaddr { uint8_t addr[6]; }PACK ble_deviceaddr_t;

#endif /* __DL_TYPES_H_ */
