// Logging.h
#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>

#ifdef DEBUG
#define LOG_FUNCTION() std::cout << "Entering: " << __PRETTY_FUNCTION__ << " [" << __FILE__ << ":" << __LINE__ << "]" << std::endl;
#else
#define LOG_FUNCTION()
#endif

#endif
