#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <csignal>
#include <iostream>

// Global flag for graceful shutdown
extern volatile bool g_shutdown;

void signal_handler(int sig);
void setupSignalHandlers();

#endif
