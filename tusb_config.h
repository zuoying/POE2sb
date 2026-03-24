#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include <stdint.h>

#define CFG_TUSB_OS               OPT_OS_PICO
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN        __attribute__((aligned(4)))
#define CFG_TUD_ENDPOINT0_SIZE    64

#define CFG_TUD_ENABLED           1
#define CFG_TUD_HID              1
#define CFG_TUD_HID_EP_BUFSIZE    64

#define CFG_TUH_ENABLED           1
#define CFG_TUH_HID               1
#define CFG_TUH_HUB               1
#define CFG_TUH_RPI_PIO_USB       1
#define CFG_TUH_MAX_DEVICE        1
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#endif
