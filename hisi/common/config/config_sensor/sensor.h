#ifndef __MY_SENSOR_CONFIG_H__
#define __MY_SENSOR_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int sensorFileRead(struct DEVHARDPARAM * dev);
extern int sensorFileGet(struct SENSORPARAM *sensor);
extern int sensorFileSet(struct SENSORPARAM *sensor);
extern CONFIG_CALLBACK sensorCallBack;

#endif	//__MY_SENSOR_CONFIG_H__
