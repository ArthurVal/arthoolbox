#include "Process.h"
#include "String.h"

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>   /* kill */
#include <sys/wait.h> /* waitpid */
#include <unistd.h>   /* fork, exec...,  */

void atb_ProcessArgs_AppendCStrList(struct atb_ProcessArgs *const pargs,
                                    char const *const *const arg_list,
                                    size_t count) {
  assert(pargs != NULL);
  assert(arg_list != NULL);

  if (count == 0)
    return;

  if (pargs->list != NULL && pargs->size > 0) {
    pargs->list =
        realloc(pargs->list, (pargs->size + (count + 1)) * sizeof(char *));
  } else {
    pargs->list = malloc((count + 1) * sizeof(char *));
  }

  if (pargs->list == NULL) {
    perror("ProcessArgs_IncreaseCapacity - malloc/realloc");
    exit(EXIT_FAILURE);
  }

  char **args_begin = pargs->list + pargs->size;
  for (size_t i = 0; i < count; ++i) {
    if ((args_begin[i] = strdup(arg_list[i])) == NULL) {
      perror("atb_ProcessArgs_AppendCStrList - strdup");
      exit(EXIT_FAILURE);
    }
  }

  pargs->size += count;
  pargs->list[pargs->size] = NULL;
}

void atb_ProcessArgs_AppendCopy(struct atb_ProcessArgs *const pargs,
                                const struct atb_ProcessArgs *const other) {
  assert(pargs != NULL);
  assert(other != NULL);
  atb_ProcessArgs_AppendCStrList(pargs, (const char *const *const)other->list,
                                 other->size);
}

void atb_ProcessArgs_Delete(struct atb_ProcessArgs *const pargs) {
  assert(pargs != NULL);

  if (pargs->list != NULL) {
    for (size_t i = 0u; i != pargs->size; ++i) {
      if (pargs->list[i] != NULL) {
        free(pargs->list[i]);
        pargs->list[i] = NULL;
      }
    }

    free(pargs->list);
  }
  atb_ProcessArgs_Init(pargs);
}

bool atb_Process_IsAlive(struct atb_Process const *const proc) {
  assert(proc != NULL);

  /* NOTE: sending kill with signal == 0 just tests if sending the signal is
   * possible, hence tests if the process exists. kill() returns -1 and errno is
   * set to ESRCH if the process doesn't exists */

  /* IMPORTANT: process exists != process is running, it could be a zombie
   * process. Only waitpid() can be used to check if the process stopped or not
   */
  return (proc->path.size > 0) && (proc->pid > 0) && (kill(proc->pid, 0) != -1);
}

static bool GetStatus(pid_t pid, bool join,
                      struct atb_ProcessStatus *const status) {
  assert(pid > 0);
  assert(status != NULL);

  siginfo_t info = {.si_pid = 0};
  int options = WEXITED;

  if (!join) {
    options |= (WNOHANG | WNOWAIT);
  }

  if (waitid(P_PID, (id_t)pid, &info, options) == -1) {
    perror("atb_Process - GetStatus - waitid");
    return false;
  }

  if (info.si_pid == 0) {
    status->state = ATB_PROCESS_RUNNING;
  } else {
    /* This should always be true if looking at the docs ? */
    assert(info.si_signo == SIGCHLD);

    /* Since waitid was called with WEXITED, si_code shouldn't be STOPPED,
     * CONTUNUED, ... */
    if (info.si_code == CLD_EXITED) {
      status->state = ATB_PROCESS_EXITED;
      status->info.exit_code = info.si_status;
    } else {
      status->state = ATB_PROCESS_KILLED;
      status->info.sig_id = info.si_status;
    }
  }

  return true;
}

bool atb_Process_GetStatus(struct atb_Process const *const proc,
                           struct atb_ProcessStatus *const status) {
  return GetStatus(proc->pid, false, status);
}

atb_Process_LaunchResult
atb_Process_Launch(struct atb_Process *const proc,
                   struct atb_String *const path,
                   const struct atb_ProcessArgs *const args) {
  assert(proc != NULL);
  assert(path != NULL);

  assert(args != NULL);
  assert(args->list != NULL);

  if (path->size == 0) {
    return ATB_PROCESS_LAUNCH_INVALID_PATH;
  }

  if (atb_Process_IsAlive(proc)) {
    return ATB_PROCESS_LAUNCH_ALREADY_RUNNING;
  }

  struct atb_ProcessArgs process_args;
  atb_ProcessArgs_Init(&process_args);
  atb_ProcessArgs_AppendCStrList(&process_args,
                                 (char const *const *)&(path->data), 1);
  atb_ProcessArgs_AppendCopy(&process_args, args);

  atb_Process_LaunchResult result = ATB_PROCESS_LAUNCH_SUCCESS;
  if ((proc->pid = fork()) == -1) {
    perror("atb_Process_Launch - fork");

    atb_ProcessArgs_Delete(&process_args);
    return ATB_PROCESS_LAUNCH_FAILURE;

  } else if (proc->pid == 0) {
    /* child  context */
    /* TODO: Rediriger stdout et stderr
       int stdout = open("app_stdout.log", O_CREAT | O_WRONLY);
       dup2(stdout, 1);
       close(stdout);

       int stderr = open("app_stderr.log", O_CREAT | O_WRONLY);
       dup2(stderr, 2);
       close(stderr);
     */
    if (execv(path->data, process_args.list) == -1) {
      perror("atb_Process_Launch - execv");
      exit(EXIT_FAILURE);
    }
  }

  atb_String_Delete(&(proc->path));
  proc->path = atb_String_MakeByMoving(path);

  atb_ProcessArgs_Delete(&process_args);
  return result;
}

atb_Process_KillResult
atb_Process_Kill(struct atb_Process *const proc,
                 struct atb_ProcessStatus *const status) {
  assert(proc != NULL);

  if (proc->pid > 0) {

    /* FIXME: Maybe use SIGTERM and then SIGKILL if still alive instead ? */
    if (kill(proc->pid, SIGKILL) == -1) {
      perror("process kill(.., SIGKILL)");
      return ATB_PROCESS_KILL_FAILURE;
    }

    struct atb_ProcessStatus internal_status;
    if (!GetStatus(proc->pid, true, &internal_status)) {
      return ATB_PROCESS_KILL_GET_STATUS_FAILED;
    } else if (status != NULL) {
      *status = internal_status;
    }

    proc->pid = -1;
  }

  if (proc->path.data != NULL) {
    atb_String_Delete(&proc->path);
  }

  return ATB_PROCESS_KILL_SUCCESS;
}
