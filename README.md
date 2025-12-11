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
| Name | Matrix No. | Role |
|------|------------|------|
| **Brendan Chan Kah Le** | 83403 | [e.g., Skeleton & CLI] |
| **Chong Ming Zin** | 83489 | [e.g., CPU Monitor] |
| **Ng Clarence Chuan Hann** | 84832 | [e.g., Memory Monitor] |
| **Xavier Liong Zhi Hao** | 86079 | [e.g., Process Monitor] |

## ✨ Features
1.  **CPU Usage:** Calculates real-time CPU utilization percentage by reading `/proc/stat`.
2.  **Memory Usage:** Displays Total, Used, and Free memory by parsing `/proc/meminfo`.
3.  **Process Monitoring:** Lists the Top 5 active processes sorted by CPU usage.
4.  **Continuous Mode:** Refreshes data automatically every *N* seconds.
5.  **Logging:** Automatically saves system snapshots to `syslog.txt` with timestamps.
6.  **Signal Handling:** Gracefully catches `Ctrl+C` to save logs and exit safely.

## ⚙️ Prerequisites
* **OS:** Linux (Ubuntu, Debian, or WSL on Windows).
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
