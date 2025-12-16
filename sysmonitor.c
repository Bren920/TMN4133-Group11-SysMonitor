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
volatile sig_atomic_t keep_running = 1;
volatile sig_atomic_t is_monitoring = 0;

// --- Helper Functions ---

void getTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void logEntry(const char *mode, const char *data) {
    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening log file");
        return;
    }

    char timestamp[64];
    getTimestamp(timestamp, sizeof(timestamp));

    char logBuffer[BUFFER_SIZE];
    int len = snprintf(logBuffer, sizeof(logBuffer), "[%s] [%s] - %s\n", timestamp, mode, data);

    if (write(fd, logBuffer, len) == -1) {
        perror("Error writing to log file");
    }
    close(fd);
}

// ---- SIGNAL HANDLER ----
void handleSignal(int sig) {
    // Re-arm signal to ensure it persists
    signal(SIGINT, handleSignal);

    if (sig == SIGINT) {
        if (is_monitoring) {
            // Case 1: Inside Continuous Monitor
            printf("\n[Signal Caught] Stopping monitoring...\n");
            keep_running = 0; 
            fflush(stdout);
        } else {
            // Case 2: In Menu
            printf("\n\nExiting Program... Saving Log.\n");
            logEntry("SIGNAL", "Session ended via Ctrl+C");
            exit(0);
        }
    }
}

// Universal wait function
void waitForInput() {
    printf("\nPress Enter to Continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF); 
}

void printError(const char *msg) {
    perror(msg);
}

void clearScreen() {
    printf("\033[H\033[J");
}

// --- Core Features ---

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

    //sleep(1);
    usleep(200000);

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

struct Process {
    int pid;
    char name[256];
    unsigned long time;
};

int compareProcesses(const void *a, const void *b) {
    struct Process *p1 = (struct Process *)a;
    struct Process *p2 = (struct Process *)b;
    if (p2->time > p1->time) return 1;
    if (p2->time < p1->time) return -1;
    return 0;
}

void listTopProcesses() {
    DIR *d;
    struct dirent *dir;
    struct Process procs[1024]; 
    int count = 0;

    d = opendir("/proc");
    if (!d) { printError("Cannot open /proc"); return; }

    while ((dir = readdir(d)) != NULL) {
        if (isdigit(dir->d_name[0])) {
            int pid = atoi(dir->d_name);
            char path[512]; 
            char buffer[BUFFER_SIZE];
            
            snprintf(path, sizeof(path), "/proc/%s/stat", dir->d_name);
            int fd = open(path, O_RDONLY);
            if (fd != -1) {
                int len = read(fd, buffer, sizeof(buffer) - 1);
                buffer[len] = '\0';
                close(fd);

                char *start_name = strchr(buffer, '(');
                char *end_name = strrchr(buffer, ')');
                
                if (start_name && end_name) {
                    memset(procs[count].name, 0, sizeof(procs[count].name));
                    strncpy(procs[count].name, start_name + 1, end_name - start_name - 1);
                    procs[count].pid = pid;

                    unsigned long utime = 0, stime = 0;
                    sscanf(end_name + 2, "%*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
                    
                    procs[count].time = utime + stime;
                    count++;
                    if (count >= 1024) break;
                }
            }
        }
    }
    closedir(d);

    qsort(procs, count, sizeof(struct Process), compareProcesses);

    printf("\n------------------------------------------------\n");
    printf(" Top 5 Processes (by CPU Time Ticks)\n");
    printf("------------------------------------------------\n");
    printf(" %-8s %-20s %-10s\n", "PID", "Name", "CPU Ticks");
    printf("------------------------------------------------\n");
    char logBuffer[2048] = "Top 5: "; 
    
    for (int i = 0; i < 5 && i < count; i++) {
        printf(" %-8d %-20s %-10lu\n", procs[i].pid, procs[i].name, procs[i].time);
        
        char tmp[512];
        snprintf(tmp, sizeof(tmp), "[%d:%.50s] ", procs[i].pid, procs[i].name);
        
        if (strlen(logBuffer) + strlen(tmp) < sizeof(logBuffer)) {
            strcat(logBuffer, tmp);
        }
    }
    logEntry("PROC", logBuffer);
}

// --- FIXED CONTINUOUS MONITOR ---
void continuousMonitor(int interval) {
    logEntry("MODE", "Started continuous monitoring");
    
    keep_running = 1;
    is_monitoring = 1; 

    while (keep_running) {
        clearScreen(); 
        printf("=== SysMonitor++ (Ctrl+C to Return to Menu) ===\n");
        printf("Starting Continuous Monitoring (Interval: %ds).\n", interval);
        getCPUUsage();
        printf("\n");
        getMemoryUsage();
        printf("\n");
        listTopProcesses();
        
        if (!keep_running) break;

        // Adjust sleep
        if (interval > 1) {
            sleep(interval - 1);
        } else {
            usleep(100000); 
        }
    }

    // Reset Monitoring Flag
    is_monitoring = 0;
    
    printf("\n>> Monitoring Stopped. Press Enter to return to menu...");
    
    // Clear buffer and wait for Enter
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- Main Program ---
int main(int argc, char *argv[]) {
    signal(SIGINT, handleSignal);

    // Command Line Mode
    if (argc > 1) {
        if (strcmp(argv[1], "-m") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Error: missing parameter. Use -m [cpu/mem/proc]\n");
                return 1;
            }
            if (strcmp(argv[2], "cpu") == 0) getCPUUsage();
            else if (strcmp(argv[2], "mem") == 0) getMemoryUsage();
            else if (strcmp(argv[2], "proc") == 0) listTopProcesses();
            else fprintf(stderr, "Invalid option. Use -h for help.\n");
        } else if (strcmp(argv[1], "-c") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Error: missing interval. Use -c [seconds]\n");
                return 1;
            }
            int interval = atoi(argv[2]);
            if (interval > 0) continuousMonitor(interval);
            else fprintf(stderr, "Invalid interval.\n");
        } else if (strcmp(argv[1], "-h") == 0) {
            printf("Usage: ./sysmonitor [flags]\n");
            printf("  -m cpu   : Show CPU usage\n");
            printf("  -m mem   : Show Memory usage\n");
            printf("  -m proc  : Show Top 5 processes\n");
            printf("  -c <sec> : Continuous monitoring\n");
        } else {
            fprintf(stderr, "Invalid option. Use -h for help.\n");
        }
        return 0;
    }

    // Interactive Menu Mode
    int choice;
    do {
        clearScreen();
        is_monitoring = 0;
        keep_running = 1;

        printf("\n==================================\n");
        printf("    SysMonitor++ Main Menu\n");
        printf("==================================\n");
        printf("1. CPU Usage\n");
        printf("2. Memory Usage\n");
        printf("3. Top 5 Processes\n");
        printf("4. Continuous Monitoring\n");
        printf("5. Exit\n");
        printf("Select an option >> ");
        
        // Read the integer
        int result = scanf("%d", &choice);

        // --- FIX START: Clear the buffer globally here ---
        // This eats the newline (\n) left by scanf so it doesn't 
        // trigger waitForInput() or other scans accidentally.
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        // --- FIX END ---
        
        if (result == EOF) continue; 

        if (result != 1) {
            printf("\nInvalid choice. Please enter a number (1-5).\n");
            sleep(1); 
            continue; 
        }

        switch (choice) {
            case 1: getCPUUsage(); waitForInput(); break;
            case 2: getMemoryUsage(); waitForInput(); break;
            case 3: listTopProcesses(); waitForInput(); break;
            case 4: {
                int interval = 2; // Default
                int valid = 0;
                char inputBuffer[64];

                // NOTE: We do NOT need to clear buffer here anymore
                // because we did it globally after scanf above.

                while (!valid) {
                    printf("Enter update interval in seconds (Press Enter for 2s): ");

                    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL) continue; 

                    if (inputBuffer[0] == '\n') {
                        interval = 2; 
                        valid = 1;
                        printf(">> Using default: 2 seconds.\n");
                        sleep(1); 
                    } else {
                        int parsed;
                        if (sscanf(inputBuffer, "%d", &parsed) == 1) {
                            if (parsed > 0) {
                                interval = parsed;
                                valid = 1;
                            } else {
                                printf("Invalid input. Please enter a positive number.\n");
                            }
                        } else {
                            printf("Invalid input. Please enter a number.\n");
                        }
                    }
                }
                continuousMonitor(interval);
                break;
            }
            case 5: 
                printf("Exiting... Saving log.\n");
                logEntry("EXIT", "User selected Exit");
                break;
            default: 
                printf("Invalid choice. Try again.\n");
                sleep(1);
        }
    } while (choice != 5);

    return 0;
}