#ifndef SYSMONITOR_H 
#define SYSMONITOR_H 
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
// Global flags (defined in utils.c) 
extern volatile sig_atomic_t keep_running; 
extern volatile sig_atomic_t is_monitoring; 
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
#endif

#include "sysmonitor.h" 
 
void getCPUUsage() { 
    unsigned long long a[4], b[4]; 
    double loadavg; 
    int fd; 
    char buffer[BUFFER_SIZE]; 
     
    // First reading 
    fd = open("/proc/stat", O_RDONLY); 
    if (fd == -1) { printError("Failed to read /proc/stat"); return; } 
    read(fd, buffer, sizeof(buffer) - 1); 
    close(fd); 
    sscanf(buffer, "%*s %llu %llu %llu %llu", &a[0], &a[1], &a[2], &a[3]); 
 
    sleep(1); 
 
    // Second reading 
    fd = open("/proc/stat", O_RDONLY); 
    if (fd == -1) { printError("Failed to read /proc/stat"); return; } 
    memset(buffer, 0, sizeof(buffer));  
    read(fd, buffer, sizeof(buffer) - 1); 
    close(fd); 
    sscanf(buffer, "%*s %llu %llu %llu %llu", &b[0], &b[1], &b[2], &b[3]); 
 
    unsigned long long load1 = a[0] + a[1] + a[2]; 
    unsigned long long load2 = b[0] + b[1] + b[2]; 
    unsigned long long total1 = load1 + a[3]; 
    unsigned long long total2 = load2 + b[3]; 
 
    if (total2 == total1) loadavg = 0.0; 
    else loadavg = (double)(load2 - load1) / (double)(total2 - total1) * 100.0; 
 
    printf("\n--------------------------------\n"); 
    printf(" CPU Usage: %.2f%%\n", loadavg); 
    printf("--------------------------------\n"); 
 
    char logMsg[64]; 
    snprintf(logMsg, sizeof(logMsg), "CPU Usage: %.2f%%", loadavg); 
    logEntry("CPU", logMsg); 
} 
 
void getMemoryUsage() { 
    int fd = open("/proc/meminfo", O_RDONLY); 
    if (fd == -1) { printError("Failed to read /proc/meminfo"); return; } 
 
    char buffer[BUFFER_SIZE]; 
    int bytesRead = read(fd, buffer, sizeof(buffer) - 1); 
    buffer[bytesRead] = '\0'; 
    close(fd); 
 
    long total_mem = 0, free_mem = 0, available_mem = 0; 
     
    char *p = buffer; 
    char *memTotalPtr = strstr(p, "MemTotal:"); 
    if (memTotalPtr) sscanf(memTotalPtr, "MemTotal: %ld kB", &total_mem); 
     
    char *memFreePtr = strstr(p, "MemFree:"); 
    if (memFreePtr) sscanf(memFreePtr, "MemFree: %ld kB", &free_mem); 
     
    char *avail_ptr = strstr(p, "MemAvailable:"); 
    if (avail_ptr) sscanf(avail_ptr, "MemAvailable: %ld kB", &available_mem); 
    else available_mem = free_mem;  
 
    long used_mem = total_mem - available_mem; 
    double used_percent = 0.0; 
    if (total_mem > 0) used_percent = (double)used_mem / total_mem * 100.0; 
 
    printf("\n--------------------------------\n"); 
    printf(" Memory Information\n"); 
    printf("--------------------------------\n"); 
    printf(" Total Memory : %ld kB\n", total_mem); 
    printf(" Used Memory  : %ld kB (%.2f%%)\n", used_mem, used_percent); 
    printf(" Free Memory  : %ld kB\n", available_mem); 
    printf("--------------------------------\n"); 
 
    char logMsg[128]; 
    snprintf(logMsg, sizeof(logMsg), "Total: %ld kB, Used: %ld kB, Free: %ld kB", total_mem, used_mem, available_mem); 
    logEntry("MEM", logMsg); 
} 
