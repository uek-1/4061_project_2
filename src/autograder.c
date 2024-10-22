#include "utility.h"

/* ------------------------------- PROTOTYPES ------------------------------- */

/* Batch Execution Prototypes */
static int GetBatchSize(void);
static void BatchRun(int BatchSize, IPCTypeT IPCType);
static void StartBatch(IPCTypeT IPCType);
static void WaitBatch(void);

/* Timer Handling Prototypes */
static void StuckProcessTimerHandler(int signum);
static void StartStuckProcessTimer(void);
static void CancelStuckProcessTimer(void);

/* Solution Execution Prototypes */
static void RedirectSolnOutput(SolnDataT *SolnData);
static void NO_RETURN RunSoln_Exec(SolnDataT *SolnData);
static void NO_RETURN RunSoln_Redirect(SolnDataT *SolnData);
static void NO_RETURN RunSoln_Pipe(SolnDataT *SolnData);

/* ---------------------------- STATIC VARIABLES ---------------------------- */

static SolnDataArrT SolnDataArr;
static BatchDataT CurrentBatchData;

/* ---------------------------- STATIC FUNCTIONS ---------------------------- */

static void RedirectSolnOutput(SolnDataT *SolnData) {
  /**
   * TODO => Change C:
   *      + Open the output file and redirect STDOUT for the child process using
   * dup2
   *      + NOTE: This function is ONLY called by a child process
   *      + Flag Hints: O_WRONLY | O_CREAT | O_TRUNC
   *      + Permission Hints: 0644
   *      + Hint: You can use SolnData->OutputFilePath as the file path to open
   */

  int output_fd =
      open(SolnData->OutputFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (output_fd < 0) {
    perror("Couldn't open output file for solution!");
  }
  if (dup2(output_fd, STDOUT_FILENO) < 0) {
    perror("Couldn't redirect STDOUT!");
  };

  if (close(output_fd) < 0) {
    perror("Error closing inital output file");
  }

  return;
}

static void NO_RETURN RunSoln_Exec(SolnDataT *SolnData) {
  /**
   * TODO => Change B:
   *      + Nothing to change in this function, if RedirectSolnOutput is
   * implemented correctly, the remaining code is provided for you... This is
   * essentially project #1
   */

  /* Convert the IPC Type enumeration as a string to pass into the soln
   * executable */
  char ipc_type_param[MAX_PARAM_LEN];
  IntToParamStr(IPC_TYPE_EXEC, ipc_type_param);

  /* Convert the input parameter integer as a string to pass into the soln
   * executable */
  char input_param[MAX_PARAM_LEN];
  IntToParamStr(SolnData->InputParam, input_param);

  /* Redirect the solution output prior to exec'ing */
  RedirectSolnOutput(SolnData);

  /* Exec the solution and pass the input parameter in */
  execl(SolnData->ExePath, SolnData->ExeName, ipc_type_param, input_param,
        NULL);

  fprintf(stderr,
          "ERROR: Execl failed... Path [%s] Exe [%s] IpcType [%s] Param [%s]\n",
          SolnData->ExePath, SolnData->ExeName, ipc_type_param,
          SolnData->InputParamStr);
  exit(EXIT_FAILURE);
}

static void NO_RETURN RunSoln_Redirect(SolnDataT *SolnData) {
  /**
   * TODO => Change B:
   *      + Derive the input file path, NOTE this is created in main via
   * GenerateInputFiles
   *      + Open that file and redirect STDIN to use that file
   */

  char input_file_path[50];
  sprintf(input_file_path, "%s/%d.in", INPUT_DIRECTORY_PATH,
          SolnData->InputParam);

  int input_fd = open(input_file_path, O_RDONLY, 0644);
  if (input_fd < 0) {
    perror("Couldn't open fd!");
  }

  if (dup2(input_fd, STDIN_FILENO) < 0) {
    perror("Error redirecting STDIN!");
  }

  if (close(input_fd) < 0) {
    perror("Error closing inital fd");
  }

  /* ------------------------------ PROVIDED CODE -----------------------------
   */

  /* Convert the IPC Type enumeration as a string to pass into the soln
   * executable */
  char ipc_type_param[MAX_PARAM_LEN];
  IntToParamStr(IPC_TYPE_REDIRECT, ipc_type_param);

  /* Redirect the solution output prior to exec'ing */
  RedirectSolnOutput(SolnData);

  /* Exec the solution without a parameter as this will be derived from STDIN */
  execl(SolnData->ExePath, SolnData->ExeName, ipc_type_param, NULL);

  /* Error check execl call */
  fprintf(stderr, "ERROR: Execl failed... Path [%s] Exe [%s] IpcType [%s]\n",
          SolnData->ExePath, SolnData->ExeName, ipc_type_param);
  exit(EXIT_FAILURE);
}

static void NO_RETURN RunSoln_Pipe(SolnDataT *SolnData) {
  int pipefds[2] = {0, 0};

  /**
   * TODO => Change B:
   *      + Open a pipe, write the input paramater to the write end of the pipe
   * and close it
   *      + Allow the provided code to pass the file descriptor of the pipe read
   * end to the soln proc
   */

  if (pipe(pipefds) < 0) {
    perror("error creating pipe!");
  }

  if (write(pipefds[PIPE_WRITE_END], &SolnData->InputParam, 1) < 0) {
    perror("Error writing to pipe!");
  }

  if (close(pipefds[PIPE_WRITE_END]) < 0) {
    perror("Error closing pipe");
  }

  /* ------------------------------ PROVIDED CODE -----------------------------
   */

  /* Convert the IPC Type enumeration as a string to pass into the soln
   * executable */
  char ipc_type_param[MAX_PARAM_LEN];
  IntToParamStr(IPC_TYPE_PIPE, ipc_type_param);

  /* Convert the read end of the pipe file descriptor as a string to pass into
   * the soln executable */
  char pipe_read_end_param[MAX_PARAM_LEN];
  IntToParamStr(pipefds[PIPE_READ_END], pipe_read_end_param);

  /* Redirect the solution output prior to exec'ing */
  RedirectSolnOutput(SolnData);

  /* Exec the solution with the parameter set to the file descriptor of the read
   * end of the pipe */
  execl(SolnData->ExePath, SolnData->ExeName, ipc_type_param,
        pipe_read_end_param, NULL);

  /* Error check execl call */
  fprintf(stderr,
          "ERROR: Execl failed... Path [%s] Exe [%s] IpcType [%s] Param [%s]\n",
          SolnData->ExePath, SolnData->ExeName, ipc_type_param,
          pipe_read_end_param);
  exit(EXIT_FAILURE);
}

/* ----------------------------- TIMER FUNCTIONS ---------------------------- */

static void StuckProcessTimerHandler(int signum) {
  /**
   * TODO => Change D:
   *      + This is the timer handler function... once it expires you should
   * loop the SolnDataArr.Array
   *      + Any process that still has a WaitStatus of IN_PROGRESS_WAIT_STATUS
   * should be killed with the kill system call
   *      + Hint for looping the SolnDataArr you can re-use some of the code in
   * WaitBatch. Specifically the for loop.
   */

  return;
}

static void StartStuckProcessTimer(void) {
  /**
   * TODO => Change D:
   *      + Start the stuck process timer...
   *      + Use STUCK_PROC_TIMER_VALUE_SEC and STUCK_PROC_TIMER_VALUE_USEC for
   * the timer values
   *
   * TODO IMPORTANT =>
   *      + Where should StartStuckProcessTimer be called????
   */

  return;
}

static void CancelStuckProcessTimer(void) {
  /**
   * TODO => Change D:
   *      + Cancel the stuck process timer
   *
   * TODO IMPORTANT =>
   *      + Where should CancelStuckProcessTimer be called???
   */

  return;
}

/* ----------------------------- BATCH FUNCTIONS ---------------------------- */

static int GetBatchSize(void) {
  /* Change A --> Code provided for you */
  int cpu_count;
  FILE *fp;
  char line[256];

  /* Open the cpuinfo file */
  if ((fp = fopen("/proc/cpuinfo", "r")) == NULL) {
    perror("ERROR: Failed to open /proc/cpuinfo\n");
    exit(EXIT_FAILURE);
  }

  /* Iterate over the lines of the file, if the string "processor" is contained
   * this is a CPU */
  cpu_count = 0;
  while (fgets(line, sizeof(line), fp)) {
    if (strstr(line, "processor") != NULL) {
      cpu_count++;
    }
  }

  /* Close the file and return the cpu count */
  fclose(fp);

  return cpu_count;
}

/**
 * Function:    StartBatch
 *
 * Description: StartBatch will create a child process for each solution to run
 * in the batch. It will then use the IPCType to call a particular starting
 * function for that solution.
 *
 * Parameters:
 *              IPCType     --> Type of IPC to use to start the solution
 *
 * Returns:     NONE
 *
 */
static void StartBatch(IPCTypeT IPCType) {

  for (int i = 0; i < CurrentBatchData.Count; i++) {
    SolnDataT *cur_soln_dat =
        &SolnDataArr.Array[CurrentBatchData.StartIndex + i];

    /* Set the solutions wait status to in-progress prior to forking and
     * exec'ing */
    cur_soln_dat->WaitStatus = IN_PROGRESS_WAIT_STATUS;

    /* FORK */
    pid_t pid = fork();

    /* ERROR CHECK */
    if (pid == -1) {
      perror("ERROR: Fork system call failed\n");
      exit(EXIT_FAILURE);
    }

    /* CHILD CODE */
    if (pid == 0) {
      switch (IPCType) {
      case IPC_TYPE_EXEC:
        RunSoln_Exec(cur_soln_dat);
        /* Unreachable - Never Returns */
        break;

      case IPC_TYPE_REDIRECT:
        RunSoln_Redirect(cur_soln_dat);
        /* Unreachable - Never Returns */
        break;

      case IPC_TYPE_PIPE:
        RunSoln_Pipe(cur_soln_dat);
        /* Unreachable - Never Returns */
        break;

      default:
        fprintf(stderr, "ERROR: Invalid IPC Type [%d]\n", IPCType);
        exit(EXIT_FAILURE);
      }
    }

    /* PARENT CODE */

    /* Save the PID and continue forking */
    cur_soln_dat->pid = pid;
  }

  /* Kick off the stuck process timer AFTER the child processes are forked */
  StartStuckProcessTimer();

  return;
}

/**
 * Function:    WaitBatch
 *
 * Description: WaitBatch will wait on all processes started in a batch
 *              using non-blocking wait. It will store the exit status
 *              of each process in the SolnDataArr
 *
 * Parameters:
 *
 * Returns:     NONE
 *
 */
static void WaitBatch(void) {
  int exited_count;
  int waitpid_result;
  int waitpid_status;

  exited_count = 0;
  while (exited_count < CurrentBatchData.Count) {
    for (int i = 0; i < CurrentBatchData.Count; i++) {
      SolnDataT *soln_data =
          &SolnDataArr.Array[CurrentBatchData.StartIndex + i];

      if (soln_data->WaitStatus == IN_PROGRESS_WAIT_STATUS) {
        waitpid_result = 0;
        waitpid_status = 0;
        waitpid_result = waitpid(soln_data->pid, &waitpid_status, WNOHANG);

        /* ERROR */
        if (waitpid_result < 0) {
          fprintf(stderr, "ERROR: waitpid system call failed on pid [%d]\n",
                  soln_data->pid);
          exit(EXIT_FAILURE);
        }

        /* CHILD STILL RUNNING */
        if (waitpid_result == 0) {
          continue;
        }

        /* CHILD EXITED --> store status and increment exited_count */
        soln_data->WaitStatus = waitpid_status;
        exited_count++;
      }
    }
  }

  /* Cancel the stuck process timer... */
  CancelStuckProcessTimer();

  return;
}

/**
 * Function:    BatchRun
 *
 * Description: BatchRun will split the solution data array into batch size
 * chunks and call StartBatch then WaitBatch on each batch started
 *
 * Parameters:
 *              SolnDataArr --> Solution data array to run
 *              BatchSize   --> Batch size to split into
 *              IPCType     --> Type of IPC to pass to StartBatch
 *
 * Returns:     NONE
 *
 */
static void BatchRun(int BatchSize, IPCTypeT IPCType) {
  int cur_soln_index;

  cur_soln_index = 0;
  while (cur_soln_index < SolnDataArr.Count) {
    /* Determine how many executions remain */
    int remaining_executions = SolnDataArr.Count - cur_soln_index;

    /* Update the static CurrentBatchData */
    CurrentBatchData.StartIndex = cur_soln_index;
    CurrentBatchData.Count = min(BatchSize, remaining_executions);

    /* Start the batch... This will fork a process for solution it needs to run
     */
    StartBatch(IPCType);

    /* Wait on the batch... This will non-blocking wait for each process to exit
     */
    WaitBatch();

    /* Increment the current solution index by the number of solutions just run
     */
    cur_soln_index += CurrentBatchData.Count;
  }

  return;
}

/* ---------------------------------- MAIN ---------------------------------- */

int main(int argc, char *argv[]) {
  printf("=== autograder v%d starting ===\n", TEMPLATE_VERSION);

  /**
   * (a)  Validate and extract the arguments passed in
   *      See utility.h for a description of ArgumentsT
   *
   *      ** CODE PROVIDED FOR YOU ALREADY **
   */
  ArgumentsT args;
  args = ValidateArguments(argc, argv);

  /* (b) Determine the batch size based on the CPU count */
  int batch_size;
  batch_size = GetBatchSize();
  printf("Batch Size Extracted: [%d]\n", batch_size);

  /* (c) If the IPC type is redirect... generate the input files */
  if (args.IPCType == IPC_TYPE_REDIRECT) {
    GenerateInputFiles(args.SolnParams);
  }

  /* (d) Determine the number of solutions we are running */
  int num_solutions;
  num_solutions = GetNumSolutions(SOLUTION_DIRECTORY_PATH);

  /**
   * (e) Determine the total number of executions that will be performed
   *     and allocate space in a solution data array for each execution.
   *
   *        Number of Solutions
   *      X Number of Parameters for each Solution
   *      -----------------------------------------
   *        Total Num Executions
   *
   *      Clear the allocated memory w/ a memset to safely initialize
   *
   */
  SolnDataArr.Count = (num_solutions * args.SolnParams.NumParams);
  SolnDataArr.Array = malloc(SolnDataArr.Count * sizeof(SolnDataT));
  memset(SolnDataArr.Array, 0x0, (SolnDataArr.Count * sizeof(SolnDataT)));

  /* (f) Use the provided utility function to populate the solution data array
   */
  PopulateSolnDataArr(
      SOLUTION_DIRECTORY_PATH, /* Directory where solution executables are */
      SolnDataArr.Array, /* Allocated array where the solution data should be
                            populated */
      args.SolnParams    /* Parameters to run each solution with */
  );

  /* (g) Call BatchRun to run the solution's in batches */
  BatchRun(batch_size, args.IPCType);

  /* (h) Write the results of the batch run to the output file*/
  WriteSolnResultsToFile(&SolnDataArr);

  /* (i) Cleanup any allocated memory */
  free(SolnDataArr.Array);

  printf("=== autograder exiting ===\n");
  return 0;
}
