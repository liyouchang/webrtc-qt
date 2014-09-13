#ifndef __MY_OSD_PRIVACY_CONFIG_H__
#define __MY_OSD_PRIVACY_CONFIG_H__

#include "../../common_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern int osdPrivacyFileRead(struct DEVHARDPARAM * dev);
extern int osdPrivacyFileGet(struct PRITOSD *osdosdPrivacy);
extern int osdPrivacyFileSet(struct PRITOSD *osdosdPrivacy);
extern CONFIG_CALLBACK osdPrivacyCallBack;

#endif
