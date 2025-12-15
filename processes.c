#include "sysmonitor.h" 
 
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
                    sscanf(end_name + 2,  "%*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime); 
                     
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