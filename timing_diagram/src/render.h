#ifndef _RENDER_H_
#define _RENDER_H_

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "types.h"

#define num_printer 6


typedef struct printer_s{
	char str[20];
	struct printer_s *next;
}printer_t;

typedef struct render_s{
	printer_t *printer[num_printer];
	
}render_t;

typedef enum printer_ee{
	PRINTER_DL=0,
	PRINTER_UL,
	PRINTER_FRAME_INFO,
	PRINTER_SUBFRAME_INFO,
	PRINTER_FRAME_T1,
	PRINTER_1,
	num_PRINTER
}printer_e;

void init_render(render_t *head);
void enqueue(printer_t **head, char *value);
int dequeue(printer_t **head, char *o_str);
void load_dl_frames(int frames, FILE *fp);//, channel_t **dl_scheduled_bitmap);
void load_ul_frames(int frames, FILE *fp);//, channel_t **ul_scheduled_bitmap);
void load_subframes_info(int frames, FILE *fp);
void load_frames_info(int frames, FILE *fp);
void load_html_table_group1(int frames, FILE *fp);
void render_html(render_t *render, printer_e target, char *str);
void output_html(render_t *render, int num_total_frame, FILE *fi, FILE *fo);//, channel_t **dl_scheduled_bitmap, channel_t **ul_scheduled_bitmap);


#endif
