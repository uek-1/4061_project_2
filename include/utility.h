#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <linux/limits.h>
#include <fcntl.h>

#define TEMPLATE_VERSION (3)

#define NUM_PARAMS_MIN (1)
#define NUM_PARAMS_MAX (4)

#define NUM_EXE_MAX (20)
#define NUM_EXE_MIN (1)

#define MAX_LEN_EXE_NAME (50)
#define MAX_PARAM_LEN (50)
#define MAX_LEN_EXE_PATH (500)

#define MAX_LEN_OUTPUT_PATH (500)

#define IN_PROGRESS_WAIT_STATUS (-1)

#define STUCK_PROC_TIMER_VALUE_SEC (5)
#define STUCK_PROC_TIMER_VALUE_USEC (0)

#define SOLUTION_DIRECTORY_PATH ("solutions")
#define OUTPUT_DIRECTORY_PATH ("output")
#define FULL_OUTPUT_PATH ("output/full_output.txt")
#define INPUT_DIRECTORY_PATH ("input")

#define min(a, b) ((a) < (b) ? (a) : (b))
#define NO_RETURN __attribute__((noreturn))

#define PIPE_READ_END (0)
#define PIPE_WRITE_END (1)

/* ---------------------------- MACROS & TYPEDEFS --------------------------- */

typedef enum
{
    IPC_TYPE_EXEC = 0,
    IPC_TYPE_REDIRECT = 1,
    IPC_TYPE_PIPE = 2
} IPCTypeT;

#define IPC_TYPE_MAX_LEN (3)

typedef struct
{
    int NumParams;                      /* Indicates the number of parameters in the subsequent array */
    int Params[NUM_PARAMS_MAX];         /* Variable sized array of the parameters passed in by the user */
} SolnParamsT;

typedef struct
{
    IPCTypeT IPCType;                   /* Indicates the IPC type to use to communicate with the solution */
    SolnParamsT SolnParams;             /* Solution parameters to run against the solutions */
} ArgumentsT;

typedef enum
{
    INVALID_STATUS = 0,                 /* Value 0 is reserved for invalid status, indicative of a solution that has not complete */
    CORRECT = 1,
    INCORRECT = 2,
    CRASH = 3,
    STUCK = 4
} SolnExitStatusT;
#define SOLN_EXIT_STATUS_MAX_LEN (5)

typedef struct
{
    char ExePath[MAX_LEN_EXE_PATH];
    char ExeName[MAX_LEN_EXE_NAME];

    int InputParam;
    char InputParamStr[MAX_PARAM_LEN];

    char OutputFilePath[MAX_LEN_OUTPUT_PATH];

    pid_t pid;
    int WaitStatus;
} SolnDataT;

typedef struct
{
    int Count;
    SolnDataT *Array;
} SolnDataArrT;

typedef struct
{
    int StartIndex;
    int Count;
} BatchDataT;





/* ------------------------------- PROTOTYPES ------------------------------- */

ArgumentsT ValidateArguments(int argc, char *argv[]);

int GetNumSolutions(const char* SolnDirectory);

void PopulateSolnDataArr(const char* SolnDirectory, SolnDataT *SolnDataArr, SolnParamsT SolnParams);

void PrintSolnToFile(FILE* Output, SolnDataT* SolnData);

void GenerateInputFiles(SolnParamsT SolnParams);

void WriteSolnResultsToFile(const SolnDataArrT *SolnDataArr);

const char* IpcTypeToStr(IPCTypeT IPCType);

IPCTypeT IpcStrToType(const char* IPCTypeStr);

void IntToParamStr(int IntIn, char ParamStr[MAX_PARAM_LEN]);

#endif 