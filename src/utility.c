#include "utility.h"
#include <stdbool.h>

static void print_usage_error(void);

const char *const IPC_TYPE_STR_MAP[IPC_TYPE_MAX_LEN] = {
    "exec",
    "redirect",
    "pipe"
};

const char *const SOLN_EXIT_STATUS_STR_MAP[SOLN_EXIT_STATUS_MAX_LEN] = {
    "invalid",
    "correct",
    "incorrect",
    "crash",
    "stuck"
};

int GetNumSolutions(const char* SolnDirectory)
{
    int soln_count;
    DIR *dir;
    struct dirent *entry;

    if((dir = opendir(SolnDirectory)) == NULL)
    {
        fprintf(stderr, "ERROR: Unable to open solution directory [%s]\n", SolnDirectory);
        exit(EXIT_FAILURE);
    }

    soln_count = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".gitkeep") == 0)
            continue;

        soln_count++;
    }

    closedir(dir);

    return soln_count;
}

ArgumentsT ValidateArguments(int argc, char *argv[])
{
    ArgumentsT args_out;
    memset(&args_out, 0x0, sizeof(ArgumentsT));

    /* (a) Validate the number of arguments else print usage */
    if(argc < 3)
    {
        print_usage_error();
        exit(EXIT_FAILURE);
    }

    /* (b.1) Extract the IPC Type from the arguments */
    bool ipc_type_found = false;
    for(int i = 0; i < IPC_TYPE_MAX_LEN; i++)
    {
        if(strcmp(argv[1], IPC_TYPE_STR_MAP[i]) == 0){
            args_out.IPCType = (IPCTypeT)i;
            ipc_type_found = true;
            break;
        }
    }

    /* (b.2) Print an error if the IPC type is incorrectly extracted */
    if(ipc_type_found == false)
    {
        fprintf(stderr, "\nERROR: Invalid IPC type provided [%s]\n\n", argv[1]);
        print_usage_error();
        exit(EXIT_FAILURE);
    }

    /* (c.1) Validate that the number of parameters does not exceed our max (subtract 2 for the first 2 arguments) */
    args_out.SolnParams.NumParams = argc - 2;
    if(args_out.SolnParams.NumParams > NUM_PARAMS_MAX)
    {
        fprintf(stderr, "\nERROR: Too many solution parameters were provided... [%d] provided. Maximum allowed [%d]\n\n", args_out.SolnParams.NumParams, NUM_PARAMS_MAX);
        print_usage_error();
        exit(EXIT_FAILURE);
    }

    /* (c.2) Extract the input parameters */
    for(int i = 0; i < args_out.SolnParams.NumParams; i++)
    {
        args_out.SolnParams.Params[i] = atoi(argv[i + 2]);
    }

    /* (d) Print the arguments parsed */
    printf("Autograder Starting Arguments:\n");
    printf("IPC Type:        [%s]\n", IPC_TYPE_STR_MAP[args_out.IPCType]);
    printf("Solution Params:\n");
    for(int i = 0; i < args_out.SolnParams.NumParams; i++){
        printf("    Param #%d -> [%d]\n", i, args_out.SolnParams.Params[i]);
    }


    return args_out;
}

static void print_usage_error(void)
{
    fprintf(stderr, "USAGE: ./autograder <ipc_type> <p1> <p2> ... <pn>\n");
    fprintf(stderr, "    ipc_type   --> A string representing the IPC to use. Options are:\n");
    fprintf(stderr, "        exec\n");
    fprintf(stderr, "        redirect\n");
    fprintf(stderr, "        pipe\n");
    fprintf(stderr, "    p1-pn      --> parameters to run the solutions with (max 4)\n");
    return;
}

void PopulateSolnDataArr(const char* SolnDirectory, SolnDataT *SolnDataArr, SolnParamsT SolnParams)
{
    int cur_soln_count;
    DIR *dir;
    struct dirent *entry;

    if((dir = opendir(SolnDirectory)) == NULL)
    {
        fprintf(stderr, "ERROR: Unable to open solution directory [%s]\n", SolnDirectory);
        exit(EXIT_FAILURE);
    }

    cur_soln_count = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        /* Skip "." and ".." directories */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".gitkeep") == 0)
            continue;

        /* Iterate each parameter that we want to run this solution with and create a solution data array for it */
        for(int i = 0; i < SolnParams.NumParams; i++)
        {
            /* String copy the solution name to the executable name */
            memset(SolnDataArr[cur_soln_count].ExeName, '\0', sizeof(SolnDataArr[cur_soln_count].ExeName));
            strcpy(
                SolnDataArr[cur_soln_count].ExeName,        /* Destination */
                entry->d_name                               /* Source */
            );

            /* Sprintf the executable path to the executable path */
            memset(SolnDataArr[cur_soln_count].ExePath, '\0', sizeof(SolnDataArr[cur_soln_count].ExePath));
            sprintf(
                SolnDataArr[cur_soln_count].ExePath,        /* Destination */
                "%s/%s",                                    /* Format */
                SolnDirectory,                              /* String #1 */
                entry->d_name                               /* String #2 */
            );

            /* Set the paramater from the ith index */
            SolnDataArr[cur_soln_count].InputParam = SolnParams.Params[i];

            /* Set the input paramater to a string */
            memset(SolnDataArr[cur_soln_count].InputParamStr, '\0', (sizeof(char) * MAX_PARAM_LEN));
            sprintf(SolnDataArr[cur_soln_count].InputParamStr, "%d", SolnParams.Params[i]);

            /* Set the output file path */
            memset(SolnDataArr[cur_soln_count].OutputFilePath, '\0', (sizeof(char) * MAX_LEN_OUTPUT_PATH));
            sprintf(SolnDataArr[cur_soln_count].OutputFilePath, "%s/%s.%d", OUTPUT_DIRECTORY_PATH, SolnDataArr[cur_soln_count].ExeName, SolnParams.Params[i]);

            /* Increment the current solution count */
            cur_soln_count++;
        }
    }

    closedir(dir);
}

void PrintSolnToFile(FILE* Output, SolnDataT* SolnData)
{
    SolnExitStatusT soln_exit_status;
    soln_exit_status = INVALID_STATUS;

    if(WIFEXITED(SolnData->WaitStatus))
    {
        soln_exit_status = (SolnExitStatusT)WEXITSTATUS(SolnData->WaitStatus);
    }
    else if (WIFSIGNALED(SolnData->WaitStatus))
    {
        if(WTERMSIG(SolnData->WaitStatus) == SIGKILL)
        {
            soln_exit_status = STUCK;
        }
        else if(WTERMSIG(SolnData->WaitStatus) == SIGSEGV)
        {
            soln_exit_status = CRASH;
        }
    }

    fprintf(Output, "%s %d(%s)\n", SolnData->ExeName, SolnData->InputParam, SOLN_EXIT_STATUS_STR_MAP[soln_exit_status]);
}

void GenerateInputFiles(SolnParamsT SolnParams)
{
    char file_path[50];

    for(int i = 0; i < SolnParams.NumParams; i++)
    {
        memset(file_path, '\0', sizeof(file_path));
        sprintf(file_path, "%s/%d.in", INPUT_DIRECTORY_PATH, SolnParams.Params[i]);
        printf("File path printing to: [%s]\n", file_path);
        FILE *f = fopen(file_path, "w");
        fprintf(f, "%d", SolnParams.Params[i]);
        fflush(f);
        fclose(f);
    }
}

void WriteSolnResultsToFile(const SolnDataArrT *SolnDataArr)
{
    FILE* output;
    output = fopen(FULL_OUTPUT_PATH, "w");
    if(output == NULL)
    {
        perror("ERROR: fopen failed\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < SolnDataArr->Count; i++)
    {
        PrintSolnToFile(output, &SolnDataArr->Array[i]);
    }
}

const char* IpcTypeToStr(IPCTypeT IPCType)
{
    return IPC_TYPE_STR_MAP[IPCType];
}

IPCTypeT IpcStrToType(const char* IPCTypeStr)
{
    for(IPCTypeT ipc_type = 0; ipc_type < IPC_TYPE_MAX_LEN; ipc_type++)
    {
        if(strstr(IPCTypeStr, IPC_TYPE_STR_MAP[ipc_type]))
        {
            return ipc_type;
        }
    }

    fprintf(stderr, "ERROR: Invalid IPC Type string: [%s]\n", IPCTypeStr);
    exit(EXIT_FAILURE);
}

void IntToParamStr(int IntIn, char ParamStr[MAX_PARAM_LEN])
{
    memset(ParamStr, '\0', (sizeof(char) * MAX_PARAM_LEN));
    sprintf(ParamStr, "%d", IntIn);
}