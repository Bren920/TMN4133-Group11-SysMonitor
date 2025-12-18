# SysMonitor++ - Linux System Resource Monitoring Tool

**Course:** TMN4133 System Programming  
**Group:** 11  
**Semester:** 1 2025/2026

## 📋 Project Overview
SysMonitor++ is a lightweight, command-line system monitoring tool developed in C. It uses standard Linux system calls and the `/proc` filesystem to track real-time system performance without relying on heavy external libraries like `htop`.

This project was developed as part of the TMN4133 System Programming course to demonstrate proficiency in:
* File handling (reading `/proc/stat`, `/proc/meminfo`).
* Process management.
* Signal handling (SIGINT).
* Command-line argument parsing.

## 👥 Group Members

| Name | Matrix No. | Role | Task Description |
|------|------------|------|------------------|
| **Brendan Chan Kah Le** | 83403 | Developer | Implemented Memory logic & Logging. |
| **Chong Ming Zin** | 83489 | Coordinator | Coordinated the Documentation and Created slides for video presentation. |
| **Ng Clarence Chuan Hann** | 84832 | Tester | Conducted CLI tests & Documentation. |
| **Xavier Liong Zhi Hao** | 86079 | Developer | Implemented CPU logic & Signal Handling. |

## ✨ Features
1.  **CPU Usage Monitoring:** Calculates real-time CPU utilization percentage by reading `/proc/stat`.
2.  **Memory Usage Analysis:** Displays Total, Used, and Free memory by parsing `/proc/meminfo`.
3.  **Process Monitoring:** Lists the Top 5 active processes sorted by CPU usage.
4. **Dual Modes:**
    * **Interactive Menu Mode:** User-friendly menu-driven interface.
    * **Command-Line (CLI) Mode:** Direct access to monitoring features with command-line arguments.
5.  **Continuous Monitoring:** Live dashboard mode with default 2 seconds refresh interval and customizable refresh interval.
6.  **Event Logging:** Automatically saves system snapshots to `syslog.txt` with timestamps.
7.  **Signal Handling:** Gracefully handles`SIGINT`(Ctrl+C) to save logs and exit safely.

## ⚙️ Prerequisites
* **Operating System:** Linux (Ubuntu, Debian, or WSL on Windows).
* **Compiler:** GCC (GNU Compiler Collection).
* **Tools:** VS Code, Git.

## 🚀 Installation & Compilation
1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/Bren920/TMN4133-Group11-SysMonitor.git](https://github.com/Bren920/TMN4133-Group11-SysMonitor.git)
    cd TMN4133-Group11-SysMonitor
    ```

2.  **Compile the program:**
    ```bash
    gcc sysmonitor.c -o sysmonitor
    ```

## 📖 Usage
You can run the program in two modes: **Interactive Menu** or **Command-Line (CLI)**.

### 1. Interactive Menu Mode
Simply run the executable without arguments:
```bash
./sysmonitor

Menu Options:
1. CPU Usage: View current CPU load.
2. Memory Usage: View memory statistics.
3. Top 5 Processes: View Top 5 active processes sorted by CPU usage.
4. Continuous Monitoring:Enter a loop to update stats every N seconds.
5. Exit: Close the program.

```
### 2. Command-Line (CLI) Mode
You can also use the program in CLI mode by providing command-line arguments.

#### Basic Commands
* **CPU Usage:**
    ```bash
    ./sysmonitor -m cpu
    ```
* **Memory Usage:**
    ```bash
    ./sysmonitor -m mem
    ```
* **Top 5 Processes:**
    ```bash
    ./sysmonitor -m proc
    ```
* **Continuous Monitoring:**
    ```bash
    ./sysmonitor -c N
    ```
    where `N` is the refresh interval in seconds.


## 📂 Logging
The program automatically generates a log file named **`syslog.txt`** in the execution directory. 

It records the following events:
* **Timestamps** for all checks.
* The specific **mode** accessed (e.g., `CPU`, `MEM`, `PROC`).
* **Start/Stop** events for continuous monitoring.

### Example Output inside `syslog.txt`:
```log
[2024-12-19 10:00:01] [CPU] - CPU Usage: 12.50%
[2024-12-19 10:00:05] [MEM] - Total: 16300000 kB, Used: 8000000 kB
[2024-12-19 10:00:10] [EXIT] - User selected Exit. Logging stopped.

