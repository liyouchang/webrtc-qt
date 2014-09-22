#ifndef __MY_CONFIG_H__
#define __MY_CONFIG_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config_dev/dev.h"
#include "config_net/net.h"
#include "config_media/media.h"
#include "config_input/input.h"
#include "config_output/output.h"
#include "config_jpeg/jpeg.h"
#include "config_name/name.h"
#include "config_serial/serial.h"
#include "config_store/store.h"
#include "config_osd_time/osd_time.h"
#include "config_osd_title/osd_title.h"
#include "config_osd_privacy/osd_privacy.h"
#include "config_sense/sense.h"
#include "config_sensor/sensor.h"
#include "config_overlay/overlay.h"
#include "config_singleLost/singleLost.h"
#include "config_vo/vo.h"
#include "config_wirless/wirless.h"

#define CONFIG_SUCESS		0
#define CONFIG_ALREADY_USED	-1
#define CONFIG_OPEN_FAILD	-2
#define CONFIG_NONE_EXIST	-3

#define CONFIG_MODULE_VERSION	0x000001


#endif

