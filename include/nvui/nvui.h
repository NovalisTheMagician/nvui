#pragma once

#define VERSION "0.1"

#ifdef _WIN32
    #ifdef NVEXPORT
        #define NVAPI __attribute__((dllexport))
    #else
        #define NVAPI __attribute__((dllimport))
    #endif
#else
    #ifdef NVEXPORT
        #define NVAPI __attribute__((visibility(default)))
    #else
        #define NVAPI
    #endif
#endif
