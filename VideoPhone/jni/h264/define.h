#ifndef DEFINE_H
#define DEFINE_H
#define ENABLE_GRAY 0

#define INT_MAX (1<<30)
#define INT64_MAX (1<<30)
#define INT_MIN (-1<<30)
#define ENABLE_THREADS 0

#define restrict 
#define ENABLE_SMALL 0
#define INT64_C __int64

#define snprintf sprintf


enum
{
	//ENOMEM = 0,
//	EINVAL   = 1,
	START_ENCODE  = 2,
    IN_ENCODE     = 3,
	END_ENCODE    = 4
};

#define ENABLE_H264_DECODER 1
#endif