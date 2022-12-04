#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "scall.h"
#include "terminal.h"
#include "paging.h"
#include "sched.h"
int32_t n_terminal = 0;
/*reference from https://wiki.osdev.org/Programmable_Interval_Timer*/
void pit_init(){
    
    outb(PIT_MODE,PIT_CMD);
    
    uint8_t data_port =  (uint8_t)PIT_MODE && 0xFF;
    outb(data_port, PIT_DATA);
    
    data_port = (uint8_t)FREQ_DIV;
    outb(data_port,PIT_DATA);
    
    enable_irq(IDT_PIT);
}

void pit_handler()
{
    //send_eoi(IDT_PIT);
    // vedio_mem_swtich
    // process_switch
}

void process_switch()
{
    send_eoi(IDT_PIT);
    // /*get a new vidmemory to switch*/
    // uint8_t* new_screen;
    // uint8_t** ns = &new_screen;
    // vidmap(ns);
    cli();
    /*video mem switch*/
    n_terminal = (current_terminal_id+1)%MAX_TERMINAL_SIZE;
    // screen_x = &terminals[new_terminal].cursor_x;
    // screen_y = &terminals[new_terminal].cursor_y; mei zhao dao duiying de bianliang muqian 
     if (n_terminal == current_terminal_id){
        
        PT[VIDEO_MEM>>12] = VIDEO_MEM;
        PT_VIDMEM[0] = VIDEO_MEM;
        PT_VIDMEM[0] = PT_VIDMEM[0] | PTE_P;

    }else{ 
        PT[VIDEO_MEM>>12] = terminal_array[n_terminal].video_buffer_addr;
        PT_VIDMEM[0] = terminal_array[n_terminal].video_buffer_addr;
        PT_VIDMEM[0] = PT_VIDMEM[0] | PTE_P;
    }
    
    
    /*get the new process*/
    uint32_t stack_esp = 0;
    uint32_t stack_ebp = 0;
    int32_t next_tid = 0;
    uint32_t pd_addr = 0;
    uint32_t pd_index = 0;
    term_info[current_terminal_id].pid = cur_pid;
    term_info[current_terminal_id].pid_t = cur_process_ptr;
    terminal_array[current_terminal_id].current_process_id = cur_pid;
    /*check if any shell exists, if not, create one*/
    if(term_info[current_terminal_id].pid == -1){
        sti();
        next_tid = forward(current_terminal_id);
        execute((uint8_t*)"shell");
    }else{
        /*get the current stack info*/
        asm volatile(
	        "movl %%esp, %%eax;"
	        "movl %%ebp, %%ebx;"
	        :"=a"(stack_esp), "=b"(stack_ebp)
	        :											
	    );
        /*store it*/
        cur_process_ptr->scheduled_esp = stack_esp;
        cur_process_ptr->scheduled_ebp = stack_ebp;
        cur_process_ptr->tss_esp0 = tss.esp0;
        /*switch the terminal*/
        next_tid = forward(current_terminal_id);
        
        /*set user page*/
        pd_index = USER_MEM >> 22; // 22 is to get the offset
        pd_addr = 2 + cur_pid ; // start from 8MB + current pid
        DT[pd_index] = 0x00000000;
        DT[pd_index] = DT[pd_index] | PD_MASK;
        DT[pd_index] = DT[pd_index] | (pd_addr << 22);
        tlb_flash();
        
        /*store stack infomation*/
        stack_esp = cur_process_ptr->scheduled_esp;
        stack_ebp = cur_process_ptr->scheduled_ebp;
        tss.esp0 = cur_process_ptr->tss_esp0;
        tss.ss0 = KERNEL_DS;
        asm volatile(
            "movl %%eax, %%esp;"
	        "movl %%ebx, %%ebp;"
            :
            :"a"(stack_esp), "b"(stack_ebp)
        );
    }
    /*set user page and the file array*/
    pd_index = USER_MEM >> 22; // 22 is to get the offset
    pd_addr = 2 + cur_pid ; // start from 8MB + current pid
    DT[pd_index] = 0x00000000;
    DT[pd_index] = DT[pd_index] | PD_MASK;
    DT[pd_index] = DT[pd_index] | (pd_addr << 22);
    tlb_flash();
    filed_array = cur_process_ptr->fd_array;
    sti();

    /*return*/
    asm volatile(
        "leave ;"
        "ret ;"
    );
}


int32_t forward( int32_t tid)
{
    int32_t terminal_next = (tid+1)%3; // max terminal num
    current_terminal_id = terminal_next;
    cur_pid = terminal_array[terminal_next].current_process_id;
    cur_process_ptr = (PCB_t*)(END_OF_KERNEL - KERNEL_STACK_SIZE * (cur_pid + 1));
    return terminal_next;
}

