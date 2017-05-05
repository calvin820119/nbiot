#include <stdlib.h>
#include <stdio.h>

#define num_subframe_per_frame 10
#define SIZE 20
const char html_p0[] = "<html><head><link rel=\"stylesheet\" href=\"style.css\"><script tpye=\"text/javascript\" ""src=\"script.js\"></script></head><body><h1>Downlink</h1><table class=dl style=\"width:";
const char html_p1[] = "px;\"""><tbody><tr>";
const char html_p2[] = "</tr></tbody></table><h1>Uplink</h1><table class=ul style=\"width:";
const char html_p3[] = "px;\"""><tbody><tr>";
const char html_p4[] = "</tr></tbody></table><form id=\"loadconfigform\">Shift to(subframe):<input type=\"text\" name=\"configname\" /><input type=\"button\" value=\"Go\" onclick=\"scrollWin(document.getElementsByName('configname')[0].value)\" /></form></body></html>";

char str_file_name[] = "output.html";
typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned long uint32_t;
typedef long int32_t;

typedef enum channel_e{
	NA=0,
	//DL
	NPSS,
	NSSS,
	NPBCH,
	NPDCCH,
	NPDSCH,
	//UL
	NPRACH,
	NPUSCH,
		
	channels_length
}channel_t;

char channel_name[channels_length][SIZE] = {
	"NA",
	//DL
	"NPSS",
	"NSSS",
	"NPBCH",
	"NPDCCH",
	"NPDSCH",
	//UL
	"NPRACH",
	"NPUSCH"
};

channel_t *dl_scheduled_bitmap[10];
channel_t *ul_scheduled_bitmap[10];
FILE *fp;





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
		generate_broadcast_signal(cur_frame, NPBCH, (void *)0);
		
		generate_broadcast_signal(cur_frame, NPSS, (void *)0);
		
		if(cur_frame&0x1)
			generate_broadcast_signal(cur_frame, NSSS, (void *)0);
	}
}

void load_dl_frames(int frames){
	uint32_t frame;
	uint32_t subframe;
	for(frame=0;frame<frames;++frame)
	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
		fprintf(fp, "<td class=%s>%d</td>\n", channel_name[dl_scheduled_bitmap[subframe][frame]], subframe);
	}
}

void load_ul_frames(int frames){
	uint32_t frame;
	uint32_t subframe;
	for(frame=0;frame<frames;++frame)
	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
		fprintf(fp, "<td class=%s>%d</td>\n", channel_name[ul_scheduled_bitmap[subframe][frame]], subframe);
	}
}

int main(int argc, char **argv){
	
	
	uint32_t i;
	uint32_t num_total_frame = 10;	//	for DL/UL
	
	if( (FILE *)0 == (fp = fopen(str_file_name, "w")));
	for(i=0;i<num_subframe_per_frame;++i){
		//	calloc to 0 -> channel_t.NA
		dl_scheduled_bitmap[i] = (channel_t *)calloc(num_total_frame, sizeof(channel_t));
		ul_scheduled_bitmap[i] = (channel_t *)calloc(num_total_frame, sizeof(channel_t));
	}
	
	mac_scheduler(num_total_frame);
	
	fprintf(fp, "%s%d%s", html_p0, 40*num_total_frame*num_subframe_per_frame, html_p1);
	load_dl_frames(num_total_frame);
	fprintf(fp, "%s%d%s", html_p2, 40*num_total_frame*num_subframe_per_frame, html_p3);
	load_ul_frames(num_total_frame);
	fprintf(fp, "%s", html_p4);
	
	fclose(fp);
	
	return 0;
} 
