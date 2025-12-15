Part 4 - Main Controller (main.c)
Contributor 4: Main Menu, Input Validation, and Monitoring Loop.
#include "sysmonitor.h"
void continuousMonitor(int interval) {
 printf("Starting Continuous Monitoring (Interval: %ds).\n", interval);
 printf(">> Press Ctrl+C to return to Main Menu <<\n");
 logEntry("MODE", "Started continuous monitoring");

 keep_running = 1;
 is_monitoring = 1;
 while (keep_running) {
 clearScreen();
 printf("=== SysMonitor++ (Ctrl+C to Return to Menu) ===\n");
 // ADDED: Show current interval
 printf(" Refresh Interval: %d seconds\n", interval);

 getCPUUsage();
 printf("\n");
 getMemoryUsage();
 printf("\n");
 listTopProcesses();

 if (!keep_running) break;
 sleep(interval);
 }
 is_monitoring = 0;
 printf("\n----------------------------------\n");
}
int main(int argc, char *argv[]) {
 signal(SIGINT, handleSignal);
 // Command Line Mode
 if (argc > 1) {
 if (strcmp(argv[1], "-m") == 0) {
 if (argc < 3) return 1;
 if (strcmp(argv[2], "cpu") == 0) getCPUUsage();
 else if (strcmp(argv[2], "mem") == 0) getMemoryUsage();
 else if (strcmp(argv[2], "proc") == 0) listTopProcesses();
 } else if (strcmp(argv[1], "-c") == 0) {
 if (argc < 3) return 1;
 int interval = atoi(argv[2]);
 if (interval > 0) continuousMonitor(interval);
 } else if (strcmp(argv[1], "-h") == 0) {
 printf("Usage: ./sysmonitor [flags]\n");
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
 printf(" SysMonitor++ Main Menu\n");
 printf("==================================\n");
 printf("1. CPU Usage\n");
 printf("2. Memory Usage\n");
 printf("3. Top 5 Processes\n");
 printf("4. Continuous Monitoring\n");
 printf("5. Exit\n");
 printf("Select an option: ");

 int result = scanf("%d", &choice);

 // Handle EOF or Stream Errors
 if (result == EOF) continue;
 // UPDATED: Handle Non-Numeric Input
 if (result != 1) {
 while(getchar() != '\n'); // Clear the input buffer
 printf("\nInvalid choice. Please enter a number (1-5).\n");
 sleep(1); // Wait so user sees the message
 continue; // Restart loop
 }
 switch (choice) {
 case 1: getCPUUsage(); waitForInput(); break;
 case 2: getMemoryUsage(); waitForInput(); break;
 case 3: listTopProcesses(); waitForInput(); break;
 case 4: {
 int interval = 0;
 int valid = 0;

 // UPDATED: Loop until valid number
 while (!valid) {
 printf("Enter refresh interval (seconds): ");
 int scanRes = scanf("%d", &interval);

 if (scanRes == 1 && interval > 0) {
 valid = 1;
 } else {
 while(getchar() != '\n'); // Clear bad input
 printf("Invalid input. Please enter a positive number.\n");
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