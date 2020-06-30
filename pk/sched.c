#include <list.h>
//s#include <sync.h>
#include "proc.h"
#include "sched.h"
#include "atomic.h"
#include "mmap.h"

static spinlock_t vm_lock = SPINLOCK_INIT;

void
wakeup_proc(struct proc_struct *proc) {
    assert(proc->state != PROC_ZOMBIE && proc->state != PROC_RUNNABLE);
    proc->state = PROC_RUNNABLE;
}

void
schedule(void) {
    list_entry_t *le, *last;
    struct proc_struct *next = NULL;
    {
        currentproc->need_resched = 0;
        last = (currentproc == idleproc) ? &proc_list : &(currentproc->list_link);
        le = last;
        do {
            if ((le = list_next(le)) != &proc_list) {
                next = le2proc(le, list_link);
                if (next->state == PROC_RUNNABLE) {
                    break;
                }
            }
        } while (le != last);
        if (next == NULL || next->state != PROC_RUNNABLE) {
            next = idleproc;
        }
        next->runs ++;
        if (next != currentproc) {
            proc_run(next);
        }
    }
}

