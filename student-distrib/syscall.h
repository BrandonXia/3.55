#ifndef SYS_CALL_H
#define SYS_CALL_H
#include    "scall.h"
#include    "sched.h"
#ifndef ASM
extern void systemcall();
extern void pit_interrupt_handler();
#endif
#endif
