#pragma once

#include <assert.h>
#include <stddef.h> /* size_t */

#include "String.h"

/**
 *  \brief Dynamic process arguments list builder used by Process_Launch
 *  \note All argvs elements are (de-)allocate using malloc/realloc/free
 */
struct atb_ProcessArgs {
  char **list; /*!< NULL terminated list of argv */
  size_t size; /*!< Size of the list (does not include the last NULL) */
};

/**
 *  \brief Statically initialize a atb_ProcessArgs data structure
 */
#define atb_ProcessArgs_INITIALIZER()                                          \
  { NULL, 0 }

/**
 *  \brief Initialize a atb_ProcessArgs data structure
 */
static inline void atb_ProcessArgs_Init(struct atb_ProcessArgs *const pargs) {
  assert(pargs != NULL);

  pargs->list = NULL;
  pargs->size = 0;
}

/**
 *  \brief Appends count copy of null-terminated str form arg_list into pargs
 *
 *  \param[in] arg_list List of null-terminated c-strings
 *  \param[in] count Size of arg_list
 *
 *  \note Performs allocation using malloc/realloc
 */
void atb_ProcessArgs_AppendCStrList(struct atb_ProcessArgs *const pargs,
                                    char const *const *const arg_list,
                                    size_t count);

/**
 *  \brief Appends other, by copying into pargs
 *
 *  \param[in] other The processargs to copy from
 *
 *  \note Performs allocation using malloc/realloc
 */
void atb_ProcessArgs_AppendCopy(struct atb_ProcessArgs *const pargs,
                                const struct atb_ProcessArgs *const other);

/**
 *  \brief Free all ressources allocated by pargs
 */
void atb_ProcessArgs_Delete(struct atb_ProcessArgs *const pargs);

/**
 *  \brief Represents an UNIX process through its path, and its PID
 */
struct atb_Process {
  struct atb_String path; /*!< Executable path used to launch the process */
  int pid;                /*!< UNIX unique PID of the process, when alive */
};

/**
 *  \brief Statically initialize a atb_Process data structure
 */
#define atb_Process_INITIALIZER()                                              \
  { atb_String_INITIALIZER(), 0 }

/**
 *  \brief Initialize a atb_Process data structure
 */
static inline void atb_Process_Init(struct atb_Process *const proc) {
  assert(proc != NULL);

  atb_String_Init(&proc->path);
  proc->pid = 0;
}

/**
 *  \returns True if the process is considered ALIVE (zombie included)
 */
bool atb_Process_IsAlive(struct atb_Process const *const proc);

/**
 *  \brief Holds the process status informations
 */
struct atb_ProcessStatus {
  enum {
    ATB_PROCESS_EXITED,  /*!< Process exited (returned using exit()) */
    ATB_PROCESS_KILLED,  /*!< Process killed by signal (did not exit()) */
    ATB_PROCESS_RUNNING, /*!< Process still alive and running */
  } state;               /*!< Process states (RUNNING/EXITED/KILLED) */

  union {
    int sig_id;    /*!< When state == KILLED, holds the signal ID used */
    int exit_code; /*!< When state == EXITED, holds the exit code */
  } info;          /*!< Additionnal info when the process is not RUNNING */
};

/**
 *  \brief Retreive the process status of proc
 *
 *  \param[out] status Data filled with the status info of the process
 *
 *  \returns True if succeeded, false otherwise (waitid failed).
 */
bool atb_Process_GetStatus(struct atb_Process const *const proc,
                           struct atb_ProcessStatus *const status);

/**
 *  \brief Return value of atb_Process_Launch
 */
typedef enum {
  ATB_PROCESS_LAUNCH_SUCCESS,         /*!< Operation succeeded */
  ATB_PROCESS_LAUNCH_FAILURE,         /*!< Unknown failure */
  ATB_PROCESS_LAUNCH_INVALID_PATH,    /*!< The path given is invalid */
  ATB_PROCESS_LAUNCH_ALREADY_RUNNING, /*!< The process is already running */
} atb_Process_LaunchResult;

/**
 *  \brief Launch a process using path with args as command
 *
 *  \note On success, this function takes ownership of the content of the path
 *        string, resetting its content
 *
 *  \param[inout] path The location fo the process/executable to launch
 *  \param[in] args The args pass to the command when launching path
 *
 *  \return atb_Process_LaunchResult According to the launch success or not
 */
atb_Process_LaunchResult
atb_Process_Launch(struct atb_Process *const proc,
                   struct atb_String *const path,
                   struct atb_ProcessArgs const *const args);

/**
 *  \brief Return value of atb_Process_Kill
 */
typedef enum {
  ATB_PROCESS_KILL_SUCCESS,           /*!< Operation succeeded */
  ATB_PROCESS_KILL_FAILURE,           /*!< Unknown failure */
  ATB_PROCESS_KILL_GET_STATUS_FAILED, /*!< Retreiving the status failed */
} atb_Process_KillResult;

/**
 *  \brief Stop a running process optionnally return the exit status of it
 *
 *  \note Send the SIGKILL signal to the process
 *  \note Free all ressources allocated by the process
 *
 *  \param[inout] status If non-null, will be set to the status of the process
 *                       after sending the SIGKILL signal
 *
 *  \return atb_Process_KillResult According to the kill success or not
 */
atb_Process_KillResult atb_Process_Kill(struct atb_Process *const proc,
                                        struct atb_ProcessStatus *const status);
