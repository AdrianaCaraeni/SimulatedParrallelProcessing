
# Simulated Parallel Processing

## Overview

This project simulates parallel processing using child processes. The parent process dynamically assigns tasks to child processes, each of which performs a simulated workload. After completing their tasks, the child processes communicate the results back to the parent using pipes and signals. The program includes error handling, sleep time randomization, and ensures proper synchronization between processes.

## Features

- **Dynamic Task Assignment**: The parent process forks child processes in a loop and assigns each process a unique task based on its index.
  
- **Two-Way Communication**: Pipes are used for communication in both directions:
  - The parent-to-child pipe sends task assignments to the children.
  - The child-to-parent pipe allows the child to send results back to the parent.

- **Signal Handling**: Each child process is assigned a unique signal. The child signals the parent when it completes its task. This ensures the parent is notified when each child finishes.

- **Error Handling**: The program includes robust error handling for system calls such as `fork()`, `pipe()`, `read()`, `write()`, and `close()` to ensure the program runs smoothly even in the case of failure.

- **Memory Management**: Proper memory management is ensured by closing pipes and reaping child processes once they have completed their tasks, avoiding any resource leakage.

## Detailed Functionality

### 1. **Creating Child Processes**
The program uses `fork()` in the `main()` function to create child processes. Each child is assigned a unique task based on its index in the loop. The task involves performing a simulated workload (e.g., a sleep operation).

### 2. **Reaping Child Processes**
Once there are no more tasks to assign, the parent waits for all child processes to terminate. This is achieved using the `wait(NULL)` system call. The parent then closes the pipes and handles any remaining cleanup.

### 3. **Setting Up Communication**
The program creates pipes for communication between the parent and child processes:
- One pipe is used for sending tasks from the parent to the child.
- Another pipe is used for sending task results from the child to the parent.

After the `fork()`, the unused pipe ends are closed on both the parent and child sides to ensure proper communication.

### 4. **Handling Signals**
Each child process is assigned a unique signal (e.g., `SIGRTMIN + i`). The child sends a signal to the parent after completing its task. The parent handles these signals using the `sigaction()` system call, which ensures that the parent is notified when a child has finished.

### 5. **Reading and Writing Data**
The program uses `write()` and `read()` system calls to communicate between the parent and child:
- The parent writes task data to the child’s pipe.
- The child reads the task data, performs its simulated task, and then writes the result back to the parent.

### 6. **Signal-Driven Synchronization**
The parent registers a signal handler to handle the completion of tasks. The signal handler is designed to update a global variable atomically, ensuring synchronization between the signal handler and the main execution flow.

### 7. **Graceful Exit and Cleanup**
Child processes monitor the return value of `read()`. If it returns zero (indicating that the pipe is closed), the child safely closes its file descriptors and exits the loop. All resources, including pipes, are cleaned up to prevent memory leaks.

### 8. **Error Handling**
Every system call (`fork()`, `pipe()`, `read()`, `write()`, `close()`) is wrapped in error handling. If a system call fails, the program prints an error message and exits gracefully. A helper function, `safe_close()`, is used to ensure that pipes are always closed correctly.

### 9. **Command-Line Arguments**
The program accepts command-line arguments to specify the number of tasks to be processed. This allows for flexibility in testing and running the program with different workloads.

### 10. **Memory Management**
No dynamic memory allocation is used in this program, so memory leaks are not a concern. All pipes are closed when they are no longer needed, and child processes are reaped to prevent resource leakage.

## Getting Started

### Prerequisites
- A Unix-based operating system (Linux, macOS).
- A C compiler (e.g., `gcc`).

### Running the Program

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/SimulatedParallelProcessing.git
   ```

2. Compile the program:

   ```bash
   cd SimulatedParallelProcessing
   gcc -o parallel_processing main.c
   ```

3. Run the program with the desired number of tasks:

   ```bash
   ./parallel_processing <number_of_tasks>
   ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
