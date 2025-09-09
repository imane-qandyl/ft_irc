#include "signals.hpp"

// Global flag for graceful shutdown
volatile bool g_shutdown = false;

void signal_handler(int sig) {
    switch (sig) {
        case SIGINT:
            std::cout << "\n[INFO] Received SIGINT (Ctrl+C) - Shutting down gracefully..." << std::endl;
            g_shutdown = true;
            break;
        case SIGTERM:
            std::cout << "\n[INFO] Received SIGTERM - Shutting down gracefully..." << std::endl;
            g_shutdown = true;
            break;
        case SIGQUIT:
            std::cout << "\n[INFO] Received SIGQUIT (Ctrl+\\) - Shutting down gracefully..." << std::endl;
            g_shutdown = true;
            break;
        case SIGTSTP:
            std::cout << "\n[INFO] Received SIGTSTP (Ctrl+Z) - Suspending process..." << std::endl;
            // Standard behavior: suspend the process (not terminate)
            signal(SIGTSTP, SIG_DFL); // Restore default behavior
            raise(SIGTSTP); // Re-raise the signal to actually suspend
            signal(SIGTSTP, signal_handler); // Re-install our handler for next time
            std::cout << "\n[INFO] Process resumed" << std::endl;
            break;
        default:
            std::cout << "\n[INFO] Received signal " << sig << " - Shutting down gracefully..." << std::endl;
            g_shutdown = true;
            break;
    }
}

void setupSignalHandlers() {
    signal(SIGINT, signal_handler);   // Ctrl+C - terminate
    // signal(SIGTERM, signal_handler);  // Termination request - terminate
    signal(SIGQUIT, signal_handler);  // Quit signal (Ctrl+\) - terminate
    signal(SIGTSTP, signal_handler);  // Ctrl+Z - suspend process
}
