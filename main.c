#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

// Define the child states
enum child_state {IDLE, BUSY, DONE};

#define NUMBER_OF_CHILDREN 3
pid_t child_pids[NUMBER_OF_CHILDREN];
volatile sig_atomic_t child_done[NUMBER_OF_CHILDREN] = {IDLE};

int pipes_parent_to_child[NUMBER_OF_CHILDREN][2];
int pipes_child_to_parent[NUMBER_OF_CHILDREN][2];
void no_interrupt_sleep(int sec){
    // advanced sleep which will not be interfered by signals
    struct timespec req, rem;

    req.tv_sec = sec;  // The time to sleep in seconds
    req.tv_nsec = 0; // Additional time to sleep in nanoseconds

    while(nanosleep(&req, &rem) == -1){
        if(errno == EINTR){
            req = rem;
        }
    }
}

void signal_handler(int signal_value){
    // Validate the signal is within the expected range
    printf("Signal handler has been triggered\n");
    if (signal_value < SIGRTMIN || signal_value >= SIGRTMIN + NUMBER_OF_CHILDREN) {
        fprintf(stderr, "Unexpected signal received: %d\n", signal_value);
        return;
    }
    // Signal handler to mark a child process as done.  
    // Updates the corresponding index in child_done[] based on the signal received.  
    child_done[signal_value - SIGRTMIN] = DONE;  // Mark child i as done
    printf("Child %d marked as DONE.\n", signal_value - SIGRTMIN);
}

// Function to handle closing of pipes with error checking
void safe_close(int pipe) {
    printf("Attempting to close a pipe: %d\n", pipe);
    if (close(pipe) == -1) {
        perror("Error closing the pipe");
        // Decide whether to terminate or log and continue
        exit(EXIT_FAILURE); // Terminate program on critical error
    }
    printf("Successfully closed pipe: %d\n", pipe);
}

void child_code(int child_id, int max_sleep_time){
printf("Child %d starting execution.\n", child_id);
// Tear down pipes that it should not have access to 
for (int j = 0; j < NUMBER_OF_CHILDREN; j++) {
            if (j == child_id) {
                safe_close(pipes_parent_to_child[j][1]); // Close write end of parent-to-child pipe
                safe_close(pipes_child_to_parent[j][0]); // Close read end of child-to-parent pipe
            } else {
                // Close all other pipe ends
                safe_close(pipes_parent_to_child[j][0]);  // Close read end of prnt_chld for other children
                safe_close(pipes_parent_to_child[j][1]);  // Close write end of prnt_chld for other children
                safe_close(pipes_child_to_parent[j][0]);  // Close read end of chld_prnt for other children
                safe_close(pipes_child_to_parent[j][1]);  // Close write end of chld_prnt for other children
                //ERROR HANDLE 
            }
    }

// Child's code
int counter = 0;
// Infinite loop for child processes to handle tasks
while(1){
    int task_id;
    printf("Child %d waiting for a task.\n", child_id);
    // Within the loop, the child should try to read from the read pipe to see if the parent has sent a task over
    // Attempt to read task id from the parent (reading from read pipe)
    int bytes_read = read(pipes_parent_to_child[child_id][0], &task_id, sizeof(task_id));
    // ERROR HANDLE below
    if(bytes_read == -1){
        perror("Error reading from parent");
        exit(EXIT_FAILURE);
    }
    // check if read returns zero, we must break since the parent can no longer send us any task to execute
    if(bytes_read == 0){
        printf("Child %d: Parent closed pipe. Exiting.\n", child_id);
        // Close the pipes that the child has been using
        safe_close(pipes_parent_to_child[child_id][0]);  // Close the read pipe from parent to child
        safe_close(pipes_child_to_parent[child_id][1]);  // Close the write pipe from child to parent
        break;  // Exit the loop since the parent has closed the pipe
    }
    printf("Child %d received task: %d\n", child_id, task_id);
    // Simulate performing a task by sleeping for a random amount of time
    // Random sleep duration based on maximum sleep time (provided as argument)
    int sleep_time = rand() % max_sleep_time + 1;  // max_sleep_time comes from the parent
    printf("Child %d sleeping for %d second(s).\n", child_id, sleep_time);
    no_interrupt_sleep(sleep_time); // sleeping, simulating running processes
    counter++; // increment counter
    // Send the task id it received back to the parent using the write pipe
    if (write(pipes_child_to_parent[child_id][1], &task_id, sizeof(task_id)) == -1) {
        perror("Error writing to parent");
        exit(EXIT_FAILURE);
    }
    printf("Child %d completed task %d. Notifying parent.\n", child_id, task_id);
    // Send signal to the parent, notifying the parent to check this child’s pipe for data
    // Notify the parent using a signal (child's specific signal id)
    if (kill(getppid(), SIGRTMIN + child_id) == -1) {
        perror("Error sending signal to parent");
         exit(EXIT_FAILURE);
        }
    }
}


// Processing the arguments
// argc - int how many arguments the program has received
// argv - string array that stores the content of these arguments
int main(int argc, char **argv) {
    if(argc != 3){
    	fprintf(stderr, "Usage: %s <N> <dt>\n", argv[0]);
	exit(EXIT_FAILURE);
    }
    // Declare a character pointer to store the pointer where conversion fails
    char *endptr;
    // Set errno to 0 to check for errors during conversion
    errno = 0;
    // Convert the first argument (argv[1]) from string to a long integer (base 10)
    long value = strtol(argv[1], &endptr, 10);
    // Check if errno is set to a non-zero value indicating a conversion error
    if (errno != 0) {
        perror("Conversion error, argv could not be converted to an integer");
        exit(EXIT_FAILURE);
    }
    // If no digits were found in the argument, endptr will point to the start of the string
    // This means no number was provided by the user, and we display an error message
    if (endptr == argv[1]) {
        fprintf(stderr, "Error: No digits found in argument '%s'\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    // Check if the conversion is valid, ensuring that the string contains only valid integer characters
    // If there are any extra non-numeric characters after the number, endptr will not be pointing to the null-terminator
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid, non-integer, characters in argument '%s'\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    // Check if the value is positive since the argument must be a positive integer
    if (value <= 0) {
        fprintf(stderr, "Error: Argument '%s' must be a positive integer\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Set errno to 0 to check for errors during conversion
    errno = 0;
    long sleepTime = strtol(argv[2], &endptr, 10);
    // Check if errno is set to a non-zero value indicating a conversion error
    if (errno != 0) {
        perror("Conversion error, argv could not be converted to an integer");
        exit(EXIT_FAILURE);
    }
    // If no digits were found in the argument, endptr will point to the start of the string
    // This means no number was provided by the user, and we display an error message
    if (endptr == argv[2]) {
        fprintf(stderr, "Error: No digits found in argument '%s'\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    // Check if the conversion is valid, ensuring that the string contains only valid integer characters
    // If there are any extra non-numeric characters after the number, endptr will not be pointing to the null-terminator
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid, non-integer, characters in argument '%s'\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    // Check if the value is positive since the argument must be a positive integer
    if (value <= 0) {
        fprintf(stderr, "Error: Argument '%s' must be a positive integer\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    // Set up a signal handler for the SIGRTMIN signal range (real-time signals)
    struct sigaction sa;
    sa.sa_handler = signal_handler;  // Assign our custom signal handler function
    // sigfillset(&sa.sa_mask); function is to block all signals while in the handler to prevent interruption
    //ERROR HANDLE 
    if(sigfillset(&sa.sa_mask) == -1) {
    perror("sigfillset failed");
    exit(EXIT_FAILURE);
    }
    sa.sa_flags = SA_RESTART;  // Ensure interrupted system calls are restarted automatically

    // Register the signal handlers for each child signal (SIGRTMIN + i for each child process)
    for (int i = 0; i < NUMBER_OF_CHILDREN; i++) {
        if (sigaction(SIGRTMIN + i, &sa, NULL) == -1) {
            perror("sigaction");  // Handle any errors while registering the signal handler
            exit(EXIT_FAILURE);
        }
    }

    // Create pipes for communication between the parent and child processes
    // Two pipes are needed per child: one for sending tasks to the child and one for receiving results
    for (int i = 0; i < NUMBER_OF_CHILDREN; i++) {
        if (pipe(pipes_parent_to_child[i]) == -1) {  // Pipe for sending tasks to children
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(pipes_child_to_parent[i]) == -1) {  // Pipe for receiving results from children
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Fork the processes to create child processes
    for (int child_id = 0; child_id < NUMBER_OF_CHILDREN; child_id++) {
        pid_t forking = fork();  // Create a new child process
        if (forking == -1) {
            perror("Failed to fork properly");  // Error handling for unsuccessful fork
            exit(EXIT_FAILURE);
        }
        if (forking == 0) {  // Child process code block
            printf("Child %d created!\n", child_id);  // Indicate that the child has been created
            child_code(child_id, sleepTime);  // Call the child code function for the specific child
            exit(0);  // Exit the child process
        }
    }
    for (int child_id = 0; child_id < NUMBER_OF_CHILDREN; child_id++) {
            // Close the unnecessary pipe ends in the parent process
            safe_close(pipes_parent_to_child[child_id][0]);  // Parent doesn't need the read end of this pipe
            safe_close(pipes_child_to_parent[child_id][1]);  // Parent doesn't need the write end of this pipe
    }

    // Parent code to send tasks to children
    // Parent process logic
    int childTotals[3];
    childTotals[0] = 0;
    childTotals[1] = 0;
    childTotals[2] = 0;
    int num_tasks = value;
    int terminated = 0;  // counter for the number of taks that have terminated
    while (1) {
        for (int i = 0; i < NUMBER_OF_CHILDREN; i++) {
            if(child_done[i] == IDLE){
                if(num_tasks > 0){  // if there are tasks remaining
                    printf("Assigning task %d to child %d.\n", num_tasks, i);
                    // Write the number of tasks to the write end of the pipe, sending it to the child process
                    // The value of num_tasks is sent to the child, and the size of num_tasks is 
                    // specified to ensure the correct amount of data is written
                    if (write(pipes_parent_to_child[i][1], &num_tasks, sizeof(num_tasks)) == -1) {
                        perror("Error writing to child");
                        exit(EXIT_FAILURE);
                    }
                   num_tasks--;
                } else {  
                    printf("No more tasks. Closing pipes for child %d.\n", i);
                    safe_close(pipes_child_to_parent[i][0]);  // Parent doesn't need the read end of this pipe
                    safe_close(pipes_parent_to_child[i][1]);  // Parent doesn't need the write end of this pipe
                    // Wait for the child process to terminate
                    if (wait(NULL) == -1) {
                        perror("Error waiting for child process");
                        exit(EXIT_FAILURE);
                    }
                    terminated ++; // increment terminated
                }
                child_done[i] = BUSY; // make child as busy since it is doing a task
            }
            if(child_done[i]== DONE){  // close the pipes
                int task_id;
                int bytes_read = read(pipes_child_to_parent[i][0], &task_id, sizeof(task_id));
                // ERROR HANDLED below
                if(bytes_read == -1){
                    perror("Error reading from parent");
                    exit(EXIT_FAILURE); 
                }
                printf("Child %d completed task %d.\n", i, task_id);
                childTotals[i]++; // keeping track of how many tasks each child dones
                child_done[i] = IDLE;
            }
        }
        if (terminated == NUMBER_OF_CHILDREN){ // if we have terminated all children
            printf("All tasks completed. Cleaning up.\n");
            break;  // we can break, since we have already close all the children's pipes
        } 
    }
    // AGGREGATE THE RESULTS!!
    printf("Parent process exiting.\n");
    printf("Child 1 did %d tasks\n", childTotals[0]);
    printf("Child 2 did %d tasks\n", childTotals[1]);
    printf("Child 3 did %d tasks\n", childTotals[2]);
    return 0;
}
