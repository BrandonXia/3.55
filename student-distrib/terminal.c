#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "types.h"
#include "scall.h"
#include "paging.h"
#include "cursor.h"


/* 
 * terminal_open
 *   DESCRIPTION: initializing the terminal for further use.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void 
 *   SIDE EFFECTS: none
 */

int32_t terminal_open(const char* filename)
{
    return 1 ;
}

/* 
 * terminal_close
 *   DESCRIPTION: cleaning terminal for further use.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void 
 *   SIDE EFFECTS: none
 */

int32_t terminal_close(const char* fname)
{
    clean_buffer();
    clean_terminal_buffer();
    return 1;
}

/* 
 * terminal_read
 *   DESCRIPTION: read from keyboard buffer to terminal buffer
 *   INPUTS: const char* fname,uint32_t fd, void* buf, int32_t nbytes)
 *   OUTPUTS: none
 *   RETURN VALUE: i - the length read
 *   SIDE EFFECTS: none
 */
int32_t terminal_read(const char* fname,uint32_t fd, void* buf, int32_t nbytes)
{
    char* t_buf = buf ;
    if (t_buf==NULL){return -1;}
    if (nbytes < 0){return -1;}
    // we have 1024 as the max buf size
    if (nbytes >= 1024){
        return -1;
    }   
    int i=0;
    while((check_enter() == 0)){}
    while(terminal_array[current_terminal_id].terminal_buffer[i] != '\n'){
        t_buf[i] = terminal_array[current_terminal_id].terminal_buffer[i];
        i++;
    }
    clean_buffer();
    terminal_array[current_terminal_id].buf_pos = 0;
    return i;
}

/* 
 * terminal_write
 *   DESCRIPTION: write from terminal to screen and clean terminal buffer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: i - write length
 *   SIDE EFFECTS: none
 */

int32_t terminal_write(int32_t fd, const void* input_buf, int32_t nbytes)
{
    const char* buf = input_buf;
    if(buf == NULL){
        return -1;
    }
    int32_t count;
    for(count = 0; count < nbytes; count++){
        putc(buf[count]);
    }
    return count;
}

/* 
 * terminal_write
 *   DESCRIPTION: clean the terminal buffer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void 
 *   SIDE EFFECTS: none
 */
void clean_terminal_buffer()
{
    int i;
    /*Clean the keyboard buffer*/
    for (i=0;i<MAX_TERMINAL_BUFFER_SIZE;i++){
        terminal_array[current_terminal_id].terminal_buffer[i] = '\0' ;
    }
    return ;
}
/* 
 * terminal_switch
 *   DESCRIPTION: switch terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void 
 *   SIDE EFFECTS: none
 */
int32_t terminal_switch(int32_t new_terminal_id)
{
    if (new_terminal_id == current_terminal_id)
        return 0;
    if (terminal_array[new_terminal_id].current_process_id == -1){
        /*switch to this new terminal and open shell with process parent pid -1*/
        save_terminal(current_terminal_id);
        restore_terminal(new_terminal_id);
        current_terminal_id = new_terminal_id;
        execute((uint8_t*) "shell");
    }
    else{
        save_terminal(current_terminal_id);
        restore_terminal(new_terminal_id);
        current_terminal_id = new_terminal_id;
    }
    return 0;
}

/* 
 * terminal_init
 *   DESCRIPTION: init terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void 
 *   SIDE EFFECTS: none
 */
int32_t terminal_init(void)
{
    int i,j;
    for (i=0;i<MAX_TERMINAL_SIZE;i++){
        terminal_array[i].terminal_id = i;
        terminal_array[i].current_process_id = -1;
        // terminal_array[i].current_process_ptr = NULL;
        terminal_array[i].screen_cursor_x = 0;
        terminal_array[i].screen_cursor_y = 0;
        terminal_array[i].buf_pos = 0;
        for (j=0;j<MAX_TERMINAL_BUFFER_SIZE;j++){
            terminal_array[i].terminal_buffer[j] = '\0';
        }
    }
    current_terminal_id = 0;
    terminal_array[0].video_buffer_addr = VEDIO_BUFFER_1;
    terminal_array[1].video_buffer_addr = VEDIO_BUFFER_2;
    terminal_array[2].video_buffer_addr = VEDIO_BUFFER_3;
    PT[(VEDIO_BUFFER_1>>12)] = PT[(VEDIO_BUFFER_1>>12)] | PTE_P;
    PT[(VEDIO_BUFFER_2>>12)] = PT[(VEDIO_BUFFER_2>>12)] | PTE_P;
    PT[(VEDIO_BUFFER_3>>12)] = PT[(VEDIO_BUFFER_3>>12)] | PTE_P;
    tlb_flash();
    return 0;
}

/* 
 *  read to cur terminal buffer
 *   DESCRIPTION: clean the terminal buffer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void 
 *   SIDE EFFECTS: none
 */
void read_to_cur_terminal_buffer(int32_t index,unsigned char input)
{
    terminal_array[current_terminal_id].terminal_buffer[index] = input;
    return ;
}


/* 
 *   save current terminal information into current terminal structure
 *   DESCRIPTION: save current terminal information into current terminal structure
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void 
 *   SIDE EFFECTS: none
 */
int32_t save_terminal(int32_t tid)
{
    // save cursor state
    terminal_array[tid].screen_cursor_x = screen_x;
    terminal_array[tid].screen_cursor_y = screen_y;

    // save buffer position state 
    terminal_array[tid].buf_pos = save_buf_pos();

    // save the video memory
    memcpy((uint8_t*)terminal_array[tid].video_buffer_addr, (uint8_t*)VIDEO_MEM, FOUR_KB);

    // save the current pid state
    terminal_array[tid].current_process_id = get_cur_pid();

    // clean keyboard buffer without touching the terminal buffer
    clean_keyboard_buffer();

    return 0;
}


int32_t restore_terminal(int32_t tid)
{
    // clean keyboard buffer without touching the terminal buffer
    clean_keyboard_buffer();

    // restore the cursor
    screen_x = terminal_array[tid].screen_cursor_x;
    screen_y = terminal_array[tid].screen_cursor_y;
    update_cursor(screen_x,screen_y);

    // restore buffer postion
    give_buf_pos(terminal_array[tid].buf_pos);

    // restore cuurent process id
    int32_t cur_pid = terminal_array[tid].current_process_id;
    set_cur_pid(cur_pid);
    PCB_t* cur_process_ptr = (PCB_t*)(END_OF_KERNEL - KERNEL_STACK_SIZE * (cur_pid + 1));
    set_cur_process_ptr(cur_process_ptr);

    //restore the video memory
    memcpy((uint8_t*) VIDEO_MEM, (uint8_t*)terminal_array[tid].video_buffer_addr, FOUR_KB);

    return 0;
}
