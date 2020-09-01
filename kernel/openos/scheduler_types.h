#ifndef OPENWSN_SCHEDULER_TYPES_H
#define OPENWSN_SCHEDULER_TYPES_H

/**
\addtogroup kernel
\{
\addtogroup Scheduler
\{
*/

#include "opendefs.h"

typedef struct task_llist_t {
   task_cbt                       cb;
   task_prio_t                    prio;
   void*                          next;
} taskList_item_t;

typedef struct {
   taskList_item_t                taskBuf[TASK_LIST_DEPTH];
   taskList_item_t*               task_list;
   uint8_t                        numTasksCur;
   uint8_t                        numTasksMax;
} scheduler_vars_t;

typedef struct {
   uint8_t                        numTasksCur;
   uint8_t                        numTasksMax;
} scheduler_dbg_t;

/**
\}
\}
*/

#endif /* OPENWSN_SCHEDULER_TYPES_H */
