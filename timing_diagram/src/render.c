#include "render.h"

channel_t *dl_scheduled_bitmap[10];
channel_t *ul_scheduled_bitmap[10];

char channel_name[channels_length][10] = {
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

char printer_char[num_printer][4] = {
	"$DL",	//	reserve for DL
	"$UL",	//	reserve for UL
	"$1"
};

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

void load_dl_frames(int frames, FILE *fp){//, channel_t *dl_scheduled_bitmap[10]){//need to confirm this method
	uint32_t frame;
	uint32_t subframe;
	for(frame=0;frame<frames;++frame)
	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
		fprintf(fp, "<td class=%s>%d</td>\n", channel_name[dl_scheduled_bitmap[subframe][frame]], subframe);
	}
}

void load_ul_frames(int frames, FILE *fp){//, channel_t *ul_scheduled_bitmap[10]){
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

void output_html(render_t *render, int num_total_frame, FILE *fi, FILE *fo){//, channel_t **dl_scheduled_bitmap, channel_t **ul_scheduled_bitmap){
	char str[100], str1[100], *p, *p2, *p3, *p4;
	uint8_t i;
	printer_t *iterator;
	uint8_t len;
	while(!feof(fi)){
		fscanf(fi, "%s\n", str);
		len=strlen(str);
		
		p = strstr(str, printer_char[PRINTER_DL]);
		if((char *)0 != p){
			fwrite(str, 1, p-str, fo);
			load_dl_frames(num_total_frame, fo);//, dl_scheduled_bitmap);
			fprintf(fo, "%s\n", p+2);
			continue;
		}
		p = strstr(str, printer_char[PRINTER_UL]);
		if((char *)0 != p){
			fwrite(str, 1, p-str, fo);
			load_ul_frames(num_total_frame, fo);//, ul_scheduled_bitmap);
			fprintf(fo, "%s\n", p+2);
			continue;
		}
		for(i=PRINTER_1;i<num_PRINTER;i++){
			p = strstr(str, printer_char[i]);
			if((char *)0 != p){
				fwrite(str, 1, p-str, fo);
				iterator = render->printer[2];
				
				while((printer_t *)0 != iterator){
					fprintf(fo, "%s", iterator->str);
					iterator=iterator->next;
				}
				fprintf(fo, "%s\n", p+2);
				break;
			}
		}
		if((char *)0 == p)
			fprintf(fo, "%s\n", str);
	}
}
