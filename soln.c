#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "utility.h"

int GetParamExec(char *argv[]);
int GetParamRedirect(char *argv[]);
int GetParamPipe(char *argv[]);

int GetParamExec(char *argv[])
{
    /**
     * TODO => Change B:
     *      + Retrieve the parameter from the input argument
     *      + hint: use index 2
     *      + hint: use strtol not atoi, atoi can be unsafe here
     */
    return 0;
}

int GetParamRedirect(char *argv[])
{
    /**
     * TODO => Change B:
     *      + Retrieve the parameter by reading (scanf) from STDIN
     */
    return 0;
}

int GetParamPipe(char *argv[])
{
    /**
     * TODO => Change B:
     *      + Convert the at index 2 into an integer which represents the FD 
     *        of the read end of the pipe
     *      + Read from that pipe... note what you read will depend on what you write
     *        in RunSoln_Pipe
     */

    return 0;
}

// Simple hash function (djb2)
unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}


int main(int argc, char *argv[]) {
    /* Disable buffering to avoid blocking when printing to a file */
    setvbuf(stdout, NULL, _IONBF, 0);


    if(argc < 2){
        printf("Usage: %s <IPC_TYPE> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    int ipc_type_arg = strtol(argv[1], NULL, 10);
    if(ipc_type_arg < 0 || ipc_type_arg >= IPC_TYPE_MAX_LEN)
    {
        fprintf(stderr, "ERROR: Expected argv[1] to be between 0 and %d, but received [%s]\n", IPC_TYPE_MAX_LEN, argv[1]);
        return EXIT_FAILURE;
    }

    IPCTypeT ipc_type = (IPCTypeT)ipc_type_arg;
    int param = 0;
    switch(ipc_type)
    {
        case IPC_TYPE_EXEC:
            param = GetParamExec(argv);
            break;

        case IPC_TYPE_REDIRECT:
            param = GetParamRedirect(argv);
            break;

        case IPC_TYPE_PIPE:
            param = GetParamPipe(argv);
            break;
        
        default:
            perror("Unkown IPC Type");
            return EXIT_FAILURE;
            break;
    }

    /* ---------------------- DO NOT MODIFY CODE BELOW HERE --------------------- */

    /* Seed random with a simple hash of the executable name plus the parameter */
    srandom(hash(argv[0]) + param);
  
    int mode = random() % 4 + 1;
    pid_t pid = getpid(); // Get the current process ID

    int correct_result = 0;
    int incorrect_result = 1;
    int segfault_result = 42;

    switch (mode) {
        case 1:
            
            // Simulate a computation for the correct answer
            for (int i = 0; i < 100000; i++) {
                correct_result += i;  // Example computation
            }
            printf("Program: %s, PID: %d, Mode: 1 - Exiting with CORRECT status\n", argv[0], pid);
            return CORRECT;  // Exit with CORRECT status code

        case 2:
             
            // Simulate a computation for the incorrect answer 
            for (int i = 1; i <= 100000; i++) {
                incorrect_result *= i;
            }
            printf("Program: %s, PID: %d, Mode: 2 - Exiting with INCORRECT status\n", argv[0], pid);
            return INCORRECT;  // Exit with INCORRECT status code

        case 3:
            
            // Simulate computation before triggering a crash
            for (int i = 0; i < 50000; i++) {
                 segfault_result /= (i + 1);
            }
            printf("Program: %s, PID: %d, Mode: 3 - Triggering a segmentation fault\n", argv[0], pid);
            raise(SIGSEGV);  // Trigger a segmentation fault
            break;

        case 4:
            printf("Program: %s, PID: %d, Mode: 4 - Running infinitely\n", argv[0], pid);
            sleep(10);
            break;

        default:
            break;
    }

    return 0;
}
