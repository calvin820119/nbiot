#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define num_subframe_per_frame 10
#define SIZE 20

//todo modify dequeue, partition all files

char output_file_name[] = "output.html";
char template_file_name[] = "template.html";
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

#define num_printer 4

typedef struct printer_s{
	char str[20];
	struct printer_s *next;
}printer_t;

typedef struct render_s{
	printer_t *printer[num_printer];
	
}render_t;

typedef enum printer_ee{
	PRINTER_1=0,
	PRINTER_2,
	PRINTER_3,
	PRINTER_4,
	num_PRINTER
}printer_e;

char printer_char[num_printer][3] = {
	"$1",	//	reserve for DL
	"$2",	//	reserve for UL
	"$3",
	"$4"
};

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



void enqueue(printer_t **head, char *value){
	printer_t *new_node, *iterator;
	new_node = (printer_t *)malloc(sizeof(printer_t));
	new_node->next = (printer_t *)0;
	//	memcpy from input to queue!
	memcpy(new_node->str, value, strlen(value)+1);
	if((printer_t *)0 == *head){
        *head = new_node;
    }else{
        iterator = *head;
        while((printer_t *)0 != iterator->next){
            iterator = iterator->next;
        }
        iterator->next = new_node;
    }
}

int dequeue(printer_t **head, char *o_str){
	printer_t *del;
	
	del = *head;
	if((printer_t *)0 == del) return 0;
	memcpy(o_str, del->str, strlen(del->str));
	*head = (*head)->next;
	free(del);
	return 1;
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
		generate_broadcast_signal(cur_frame, NPBCH, (void *)0);
		
		generate_broadcast_signal(cur_frame, NPSS, (void *)0);
		
		if(cur_frame&0x1)
			generate_broadcast_signal(cur_frame, NSSS, (void *)0);
	}
}

void load_dl_frames(int frames, FILE *fp){
	uint32_t frame;
	uint32_t subframe;
	for(frame=0;frame<frames;++frame)
	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
		fprintf(fp, "<td class=%s>%d</td>\n", channel_name[dl_scheduled_bitmap[subframe][frame]], subframe);
	}
}

void load_ul_frames(int frames, FILE *fp){
	uint32_t frame;
	uint32_t subframe;
	for(frame=0;frame<frames;++frame)
	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
		fprintf(fp, "<td class=%s>%d</td>\n", channel_name[ul_scheduled_bitmap[subframe][frame]], subframe);
	}
}

void render_html(render_t *render, printer_e target, char *str){
	enqueue(&render->printer[target], str);
}

void output_html(render_t *render, int num_total_frame, FILE *fi, FILE *fo){
	char str[100], str1[100], *p1, *p2, *p3, *p4;
	uint8_t len;
	while(!feof(fi)){
		fscanf(fi, "%s\n", str);
		len=strlen(str);
		p1 = strstr(str, printer_char[0]);
		p2 = strstr(str, printer_char[1]);
		p3 = strstr(str, printer_char[2]);
		p4 = strstr(str, printer_char[3]);
		
		if((char *)0 != p1){
			printf("p1\n");
			fwrite(str, 1, p1-str, fo);
			load_dl_frames(num_total_frame, fo);
			fprintf(fo, "%s\n", p1+2);
			printf("p1\n");
			continue;
		}
		if((char *)0 != p2){
			printf("p2\n");
			fwrite(str, 1, p2-str, fo);
			load_ul_frames(num_total_frame, fo);
			fprintf(fo, "%s\n", p2+2);
			printf("p2\n");
			continue;
		}
		if((char *)0 != p3){
			printf("p3\n");
			fwrite(str, 1, p3-str, fo);
			while(dequeue(&render->printer[2], str1)){
				fprintf(fo, "%s", str1);
			}
			fprintf(fo, "%s\n", p3+2);
			printf("p3\n");
			continue;
		}
		if((char *)0 != p4){
			printf("p4\n");
			fwrite(str, 1, p4-str, fo);
			while(dequeue(&render->printer[3], str1)){
				fprintf(fo, "%s", str1);
			}
			fprintf(fo, "%s\n", p4+2);
			printf("p4\n");
			continue;
		}
		fprintf(fo, "%s\n", str);
	}
}

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
	
	render_html(&render, PRINTER_3, str);
	render_html(&render, PRINTER_4, str);
	
	output_html(&render, num_total_frame, fi, fo);
	
	fclose(fo);
	fclose(fi);
	
	return 0;
} 
