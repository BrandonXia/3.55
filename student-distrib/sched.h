#ifndef _SCHED_H
#define _SCHED_H

#define PIT_CMD     0x43
#define PIT_DATA    0x40
#define IDT_PIT     0x0
#define PIT_MODE    0x37     /*we use C3, M0*/
#define FREQ_DIV    46      /*the value of the low 8 bit of the division*/
typedef struct terminal_info{
    int pid;
    struct PCB_t* pid_t;
}terminal_info;
terminal_info term_info[3];
void pit_init(void);
void pit_handler(void);
void process_switch(void);
int32_t forward(int32_t);
#endif

