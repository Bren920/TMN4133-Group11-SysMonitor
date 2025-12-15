#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
#define LOG_FILE "syslog.txt"

// Function Prototypes
void getCPUUsage();
void getMemoryUsage();
void listTopProcesses();
void continuousMonitor(int interval);
void handleSignal(int sig);
void logEntry(const char *mode, const char *data);
void printError(const char *msg);
void clearScreen();
void waitForInput();
void getTimestamp(char *buffer, size_t size);

// --- Global Flags ---
// keep_running: Controls the while loop in continuous mode
volatile sig_atomic_t keep_running = 1;
// is_monitoring: Tells the signal handler if we are in the loop or not
volatile sig_atomic_t is_monitoring = 0;

// --- Helper Functions ---

// Get current timestamp string
void getTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Log to file using system calls
void logEntry(const char *mode, const char *data) {
    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening log file");
        return;
    }

    char timestamp[64];
    getTimestamp(timestamp, sizeof(timestamp));

    char logBuffer[BUFFER_SIZE];
    // Format: [Timestamp] [Mode] - Data
    int len = snprintf(logBuffer, sizeof(logBuffer), "[%s] [%s] - %s\n", timestamp, mode, data);

    if (write(fd, logBuffer, len) == -1) {
        perror("Error writing to log file");
    }
    close(fd);
}

// Signal Handler
void handleSignal(int sig) {
    if (sig == SIGINT) {
        if (is_monitoring) {
            // Case 1: We are inside the continuous monitor loop.
            // Just stop the loop, do not exit the program.
            printf("\nStopping monitoring... returning to menu.\n");
            keep_running = 0;
        } else {
            // Case 2: We are at the menu or elsewhere.
            // Exit the program.
            printf("\n\nExiting Program... Saving log.\n");
            logEntry("SIGNAL", "Session ended via Ctrl+C");
            exit(0);
        }
    }
}

// Wait for user to press Enter
void waitForInput() {
    printf("\nPress Enter to continue...");
    // Clear input buffer (catch the newline from previous scanf)
    while(getchar() != '\n'); 
    // Wait for actual input
    getchar(); 
}

void printError(const char *msg) {
    perror(msg);
}

void clearScreen() {
    // using clear for Linux
    system("clear");
}