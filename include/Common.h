#ifndef __COMMON__
#define __COMMON__

#include <chrono>
#include <string>
#include <iostream>


#ifdef DEBUG
#define LOG(info) \
    std::cout << __FILE__ << ":" \
    << __LINE__ << " (" << __TIMESTAMP__ \
    << "): " << info << std::endl;
#else
#define LOG(info)
#endif

#endif


