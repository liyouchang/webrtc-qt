#ifndef ZMQPEERTEMINAL_H
#define ZMQPEERTEMINAL_H




/*  Handle DSO symbol visibility   */
#if defined _WIN32
#   if defined TERMINAL_STATIC
#       define TERMINAL_EXPORT
#   elif defined DLL_EXPORT
#       define TERMINAL_EXPORT __declspec(dllexport)
#   else
#       define TERMINAL_EXPORT __declspec(dllimport)
#   endif
#else
#   if defined __SUNPRO_C  || defined __SUNPRO_CC
#       define TERMINAL_EXPORT __global
#   elif (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#       define TERMINAL_EXPORT __attribute__ ((visibility("default")))
#   else
#       define TERMINAL_EXPORT
#   endif
#endif


extern "C"{

TERMINAL_EXPORT int Initialize();
TERMINAL_EXPORT int Connect();
TERMINAL_EXPORT int SendByTunnel(const char *data,int len);

}


#endif // ZMQPEERTEMINAL_H
