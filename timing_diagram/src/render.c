#include "render.h"

//#define UL_ALL_3750HZ

channel_t *dl_scheduled_bitmap[10];
channel_t *ul_scheduled_bitmap[48][10];
uint8_t carrier_has_nprach[12] = {0};

uint8_t channel_name[channels_length][12] = {
	"NA",
	//DL
	"NPSS",
	"NSSS",
	"NPBCH",
	"NPDCCH",
	"NPDSCH",
	"NPDSCH_SIB",
	//UL
	"NPRACH",
	"NPUSCH"
};

uint8_t printer_char[num_printer][4] = {
	"$DL",	//	reserve for DL
	"$UL",	//	reserve for UL
	"$RF",
	"$SF",
	"$F1",
	"$1"
};

void enqueue(printer_t **head, uint8_t *value){
	printer_t *new_node, *iterator;
	new_node = (printer_t *)malloc(sizeof(printer_t));
	new_node->next = (printer_t *)0;
	//	memcpy from input to queue!
	memcpy(new_node->str, value, strlen(value)+1);
	
	if((printer_t *)0 == *head){
	    *head = new_node;
    }else{
        iterator = *head;
        printf("%d %d", iterator, iterator->next);
        while((printer_t *)0 != iterator->next){
            iterator = iterator->next;
        }
        iterator->next = new_node;
    }
}

uint32_t dequeue(printer_t **head, uint8_t *o_str){
	printer_t *del;
	
	del = *head;
	if((printer_t *)0 == del) return 0;
	memcpy(o_str, del->str, strlen(del->str));
	*head = (*head)->next;
	free(del);
	return 1;
}

void load_frames_info(uint32_t frames, FILE *fp){
    uint32_t frame;
    
	for(frame=0;frame<frames;++frame){
		fprintf(fp, "<td colspan=\"10\" class=\"FRAME\">%d</td>\n", frame);
	}
}

void load_subframes_info(uint32_t frames, FILE *fp){
    uint32_t frame;
	uint32_t subframe;
	
	for(frame=0;frame<frames;++frame)
	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
		fprintf(fp, "<td class=\"SUBFRAME\">%d</td>\n", subframe);
	}
}

void load_dl_frames(uint32_t frames, FILE *fp){//, channel_t *dl_scheduled_bitmap[10]){//need to confirm this method
	uint32_t frame;
	uint32_t subframe;
	for(frame=0;frame<frames;++frame)
	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
		fprintf(fp, "<td class=%s></td>\n", channel_name[dl_scheduled_bitmap[subframe][frame]]);
	}
}

void load_ul_frames(uint32_t frames, FILE *fp){//, channel_t *ul_scheduled_bitmap[10]){
    uint32_t frame;
	uint32_t subframe;
	int8_t subcarrier, subcarrier_15k;
	
	for(subcarrier=47;subcarrier>=0;--subcarrier){
	    subcarrier_15k=subcarrier/4;
        
        #ifdef UL_ALL_3750HZ
        fprintf(fp, "<tr>\n");
        if(0==((subcarrier+1)%4))
            fprintf(fp, "<td rowspan=\"4\">%d-%d</td>\n", subcarrier-3, subcarrier);
        #else
        if(carrier_has_nprach[subcarrier_15k]){
            fprintf(fp, "<tr>\n");
            if(0==((subcarrier+1)%4))
                fprintf(fp, "<td rowspan=\"4\">%d-%d</td>\n", subcarrier-3, subcarrier);
        }else{
            if(0==((subcarrier+1)%4))
                fprintf(fp, "<tr>\n<td>%d-%d</td>\n", subcarrier-3, subcarrier);
        }
        #endif
    	for(frame=0;frame<frames;++frame)
    	for(subframe=0;subframe<num_subframe_per_frame;++subframe){
    	    #ifdef UL_ALL_3750HZ
    	    fprintf(fp, "<td class=%s></td>\n", channel_name[ul_scheduled_bitmap[subcarrier][subframe][frame]]);
            #else
            if( carrier_has_nprach[subcarrier_15k] ){
    	        if(NPRACH==ul_scheduled_bitmap[subcarrier][subframe][frame]){
    	            fprintf(fp, "<td class=NPRACH></td>\n");
                }else{
                    if(0==((subcarrier+1)%4))
                        fprintf(fp, "<td rowspan=\"4\" class=%s></td>\n", channel_name[ul_scheduled_bitmap[subcarrier][subframe][frame]]);
                }
            }else{
                if(0==((subcarrier+1)%4))
                    fprintf(fp, "<td class=%s></td>\n", channel_name[ul_scheduled_bitmap[subcarrier][subframe][frame]]);
            } 
    	    #endif
        }
        
        #ifdef UL_ALL_3750HZ
        fprintf(fp, "</tr>\n");
        #else
        if(carrier_has_nprach[subcarrier_15k]){
            fprintf(fp, "</tr>\n");
        }else{
            if(0==((subcarrier+1)%4))
                fprintf(fp, "</tr>\n");
        }
        #endif
    }
}

void init_render(render_t *render){
    uint32_t i;
    for(i=0;i<num_PRINTER;++i)
        render->printer[i] = (printer_t *)0;
}

void render_html(render_t *render, printer_e target, uint8_t *str){
	enqueue(&render->printer[target], str);
}

void output_html(render_t *render, uint32_t num_total_frame, FILE *fi, FILE *fo){//, channel_t **dl_scheduled_bitmap, channel_t **ul_scheduled_bitmap){
	uint8_t str[100], str1[100], *p, *p2, *p3, *p4;
	uint8_t i;
	printer_t *iterator;
	uint8_t len;
	while(!feof(fi)){
		fscanf(fi, "%s\n", str);
		len=strlen(str);
		
		p = strstr(str, printer_char[PRINTER_DL]);
		if((uint8_t *)0 != p){
			fwrite(str, 1, p-str, fo);
			load_dl_frames(num_total_frame, fo);//, dl_scheduled_bitmap);
			fprintf(fo, "%s\n", p+3);
			continue;
		}
		p = strstr(str, printer_char[PRINTER_UL]);
		if((uint8_t *)0 != p){
			fwrite(str, 1, p-str, fo);
			load_ul_frames(num_total_frame, fo);//, ul_scheduled_bitmap);
			fprintf(fo, "%s\n", p+3);
			continue;
		}
		
		p = strstr(str, printer_char[PRINTER_FRAME_INFO]);
		if((uint8_t *)0 != p){
			fwrite(str, 1, p-str, fo);
			load_frames_info(num_total_frame, fo);//, ul_scheduled_bitmap);
			fprintf(fo, "%s\n", p+3);
			continue;
		}
		p = strstr(str, printer_char[PRINTER_SUBFRAME_INFO]);
		if((uint8_t *)0 != p){
			fwrite(str, 1, p-str, fo);
			load_subframes_info(num_total_frame, fo);//, ul_scheduled_bitmap);
			fprintf(fo, "%s\n", p+3);
			continue;
		}
		for(i=PRINTER_1;i<num_PRINTER;i++){
			p = strstr(str, printer_char[i]);
			if((uint8_t *)0 != p){
				fwrite(str, 1, p-str, fo);
				iterator = render->printer[i];
				
				while((printer_t *)0 != iterator){
					fprintf(fo, "%s", iterator->str);
					iterator=iterator->next;
				}
				fprintf(fo, "%s\n", p+2);
				break;
			}
		}
		if((uint8_t *)0 == p)
			fprintf(fo, "%s\n", str);
	}
}

void dl_scheduled(uint32_t frame, uint32_t subframe, channel_t channel, uint16_t rnti, uint8_t *string){
	dl_scheduled_bitmap[subframe][frame] = channel;
	
	switch(channel){
		case NPSS:
			break;
		case NSSS:
			break;
		case NPBCH:
			break;
		case NPDCCH:
			break;
		case NPDSCH:
			if(SI_RNTI == rnti){
				dl_scheduled_bitmap[subframe][frame] = NPDSCH_SIB;
			}else{
			    dl_scheduled_bitmap[subframe][frame] = NPDSCH;
            }
			break;

		default:
			break;
	}
}

void ul_scheduled(uint32_t frame, uint32_t subframe, uint32_t carrier, channel_t channel){
	ul_scheduled_bitmap[carrier][subframe][frame] = channel;
	switch(channel){
		case NPRACH:
		    carrier_has_nprach[carrier/4] = 1;
			break;
		case NPUSCH:
			break;
		default:
			break;
	}
}


