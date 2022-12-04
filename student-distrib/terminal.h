#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#define VEDIO_BUFFER_1 0xB9000
#define VEDIO_BUFFER_2 0xBA000
#define VEDIO_BUFFER_3 0xBB000
#define MAX_TERMINAL_SIZE 3
#define MAX_TERMINAL_BUFFER_SIZE 128
#define FOUR_KB 4096

typedef struct terminal_t{
    int32_t screen_cursor_x;
    int32_t screen_cursor_y;
    int32_t terminal_id;
    unsigned char terminal_buffer[MAX_TERMINAL_BUFFER_SIZE];
    int32_t video_buffer_addr;
    // PCB_t* current_process_ptr;
    int32_t buf_pos;
    int32_t current_process_id;
    /*To be added*/
}terminal_t;

terminal_t terminal_array[MAX_TERMINAL_SIZE];
int32_t current_terminal_id;

int32_t terminal_open(const char* filename);
int32_t terminal_close(const char* fname);
int32_t terminal_write(int32_t fd, const void* input_buf, int32_t nbytes);
int32_t terminal_read(const char* fname,uint32_t fd, void* buf, int32_t nbytes);
int32_t terminal_switch(int32_t new_terminal_id);
int32_t terminal_init(void);
void clean_terminal_buffer();
void read_to_cur_terminal_buffer(int32_t index,unsigned char input);
int32_t restore_terminal(int32_t tid);
int32_t save_terminal(int32_t tid);
void enable_video_buf_page(void);
#endif
