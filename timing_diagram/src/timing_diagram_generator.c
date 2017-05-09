#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "render.h"
#include "types.h"

void generate_broadcast_signal(uint32_t cur_frame, channel_t channel, void *content);
void mac_scheduler(int frames);

extern channel_t *dl_scheduled_bitmap[10];
extern channel_t *ul_scheduled_bitmap[10];
char output_file_name[] = "../out/output.html";
char template_file_name[] = "../ref/template.html";

int main(int argc, char **argv){
	char str[20];
	FILE *fo, *fi;
	uint32_t i;
	uint32_t num_total_frame = 10;	//	for DL/UL
	
	render_t render;
	render.printer[0] = (printer_t *)0;
	render.printer[1] = (printer_t *)0;
	render.printer[2] = (printer_t *)0;
	render.printer[3] = (printer_t *)0;
	
	if( (FILE *)0 == (fo = fopen(output_file_name, "w"))){
		printf("[0x1]failed to open $s!\n", output_file_name);
		exit(1);
	};
	if( (FILE *)0 == (fi = fopen(template_file_name, "r"))){
		printf("[0x2]failed to open $s!\n", template_file_name);
		exit(1);
	};
	for(i=0;i<num_subframe_per_frame;++i){
		//	calloc to 0 -> channel_t.NA
		dl_scheduled_bitmap[i] = (channel_t *)calloc(num_total_frame, sizeof(channel_t));
		ul_scheduled_bitmap[i] = (channel_t *)calloc(num_total_frame, sizeof(channel_t));
	}
	
	mac_scheduler(num_total_frame);
	
	sprintf(str, "%d\0", 40*num_total_frame*num_subframe_per_frame);
	
	render_html(&render, PRINTER_1, str);
	
	output_html(&render, num_total_frame, fi, fo);//, dl_scheduled_bitmap, ul_scheduled_bitmap);
	
	fclose(fo);
	fclose(fi);
	
	system("pause");
	return 0;
} 



void generate_broadcast_signal(uint32_t cur_frame, channel_t channel, void *content){
	//assert(channel <= NPBCH);
	switch(channel){
		case NPSS:
			dl_scheduled_bitmap[5][cur_frame] = channel;
			break;
		case NSSS:
			dl_scheduled_bitmap[9][cur_frame] = channel;
			break;
		case NPBCH:
			dl_scheduled_bitmap[0][cur_frame] = channel;
			break;
		default:
			break;
	}
}

void mac_scheduler(int frames){
	uint32_t cur_frame;
	uint32_t cur_subframe;
	//assert(scheduled_bitmap);
	for(cur_frame=0; cur_frame<frames; ++cur_frame){
		//	downlink 
		generate_broadcast_signal(cur_frame, NPBCH, (void *)0);
		
		generate_broadcast_signal(cur_frame, NPSS, (void *)0);
		
		if(cur_frame&0x1);
		else
			generate_broadcast_signal(cur_frame, NSSS, (void *)0);
			
		//	uplink
	}
}
