# 🚀 Simulated Parallel Processing

This project demonstrates a simulated parallel processing system where multiple child processes are dynamically assigned tasks by the parent process. The child processes simulate work by performing a sleep operation and communicate their task completion back to the parent process.

## 🛠️ Key Features
- **Dynamic Task Assignment**: The parent process forks child processes in a loop, assigning unique tasks to each one based on its index.
- **Process Management**: The system ensures that processes are properly reaped, pipes are closed, and the parent waits for child termination.
- **Two-way Communication**: Pipes are set up for two-way communication between the parent and child processes.
- **Signal Handling**: The project uses signals for task completion notification from child to parent.
- **Error Handling**: All system calls such as `read`, `write`, `fork`, and `pipe` are error-handled for robustness.

## 💻 Project Overview

### Parent Process:
- Dynamically assigns tasks to child processes.
- Communicates through pipes to send task assignments and receive results.
- Waits for child processes to terminate before exiting.

### Child Processes:
- Each child process performs a "work" simulation by sleeping for a randomized time.
- After completing the task, the child notifies the parent via a signal and writes the result back through pipes.

### Communication & Synchronization:
- **Pipes**: Two sets of pipes are created for each child—one for sending data from the parent to the child and one for sending data from the child to the parent.
- **Signals**: Custom signals are used to notify the parent when a child completes its task, ensuring proper synchronization without busy-waiting.

### Error Handling:
- All system calls (`fork()`, `pipe()`, `read()`, `write()`) are wrapped in error checks to handle failures gracefully.
- Pipe file descriptors are safely closed after use to prevent leaks.

## ⚙️ System Calls & Functions

- `fork()`: Used to create child processes.
- `pipe()`: Sets up communication channels between the parent and child processes.
- `read()`: Retrieves task data and results from pipes.
- `write()`: Sends task assignments and results back to the parent.
- `kill()`: Used to send signals to notify the parent of task completion.
- `wait()`: Ensures the parent waits for the termination of all child processes.

## 🛠️ How It Works

1. **Forking Child Processes**:  
   The main process forks child processes using a loop.  
   Each child is assigned a task based on its index.

2. **Pipes Setup**:  
   Each child gets a dedicated set of pipes for two-way communication.

3. **Signal Registration**:  
   Each child registers a unique signal (`SIGRTMIN + i`) to notify the parent when the task is completed.

4. **Task Execution & Communication**:  
   The parent sends tasks to the children via the pipes.  
   Once a task is completed, the child sends the result back to the parent through the pipes.

5. **Process Reaping**:  
   The parent process waits for the termination of all child processes using `wait()`.

## ⚙️ Error Handling
The project includes robust error handling throughout:
- **Pipe Management**: All pipes are closed in the cleanup phase.
- **Signal Handling**: Proper signal registration and handling to ensure correct synchronization between parent and child.
- **System Call Error Checking**: Every system call (e.g., `fork()`, `pipe()`, `read()`, `write()`) is checked for failure, and proper actions are taken to handle errors.

## 🏗️ Installation

To build and run the project, follow these steps:

1. Clone this repository:

    ```bash
    git clone https://github.com/yourusername/SimulatedParallelProcessing.git
    ```

2. Navigate to the project directory:

    ```bash
    cd SimulatedParallelProcessing
    ```

3. Compile the program:

    ```bash
    make
    ```

4. Run the program:

    ```bash
    ./simulated_parallel_processing
    ```

## 📬 Contact
Feel free to reach out via [LinkedIn](https://linkedin.com/in/adriana-caraeni)

---

Made with ❤️ and 🖥️ by Adriana

