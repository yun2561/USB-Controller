#ifndef USB_CONF_H
#define USB_CONF_H

#include <stdlib.h>
#include "gd32f3x0.h"

//#include "gd32f350r_eval.h"
//#include "gd32f350r_audio_codec.h"

/* USB FIFO size config */
#define RX_FIFO_FS_SIZE             128U
#define TX0_FIFO_FS_SIZE            64U
#define TX1_FIFO_FS_SIZE            64U
#define TX2_FIFO_FS_SIZE            64U
#define TX3_FIFO_FS_SIZE            0U

#define USB_SOF_OUTPUT              0
#define USB_LOW_POWER               0

//#define VBUS_SENSING_ENABLED

//#define USE_HOST_MODE
#define USE_DEVICE_MODE
//#define USE_OTG_MODE

#ifndef USE_DEVICE_MODE
    #ifndef USE_HOST_MODE
        #error  "USE_DEVICE_MODE or USE_HOST_MODE should be defined!"
    #endif
#endif

/* __packed keyword used to decrease the data type alignment to 1-byte */
#if defined (__GNUC__)         /* GNU Compiler */
    #ifndef __packed
        //#define __packed __unaligned
        #define __packed __attribute__ ((__packed__))
    #endif
#elif defined (__TASKING__)    /* TASKING Compiler */
    #define __packed __unaligned
#endif /* __GNUC__ */

#endif /* __USB_CONF_H__ */

