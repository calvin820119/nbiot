#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "render.h"
#include "types.h"

void generate_broadcast_signal(uint32_t cur_frame, channel_t channel, void *content);
void mac_scheduler(uint32_t frames);
uint8_t is_sib1_frame(uint32_t cur_frame);
void init_SI();

extern channel_t *dl_scheduled_bitmap[10];
extern channel_t *ul_scheduled_bitmap[48][10];
uint8_t output_file_name[] = "../out/output.html";
uint8_t template_file_name[] = "../ref/template.html";

typedef struct sib1_nb_s{
    uint8_t repetitions;    //  4, 8, 16
    uint8_t starting_rf;
}sib1_nb_t;

sib1_nb_t sib1;

int main(int argc, char **argv){
	uint8_t str[20];
	FILE *fo, *fi;
	uint32_t i, ii;
	uint32_t num_total_frame = 256;	//	for DL/UL
	
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
		for(ii=0;ii<48;++ii)
		ul_scheduled_bitmap[ii][i] = (channel_t *)calloc(num_total_frame, sizeof(channel_t));
	}
	
	init_SI();
	
	for(i=0;i<41;i++){
	    ul_scheduled(0, 1, i, NPRACH);
    }
    for(;i<48;i++){
	    ul_scheduled(0, 3, i, NPRACH);
    }
	
	mac_scheduler(num_total_frame);
	
	sprintf(str, "%d\0", 40*num_total_frame*num_subframe_per_frame);
	
	init_render(&render);
	
	render_html(&render, PRINTER_1, str);
	
	output_html(&render, num_total_frame, fi, fo);//, dl_scheduled_bitmap, ul_scheduled_bitmap);
	
	fclose(fo);
	fclose(fi);
	
	//system("pause");
	return 0;
} 

/*
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
}*/

void init_SI(){
    uint32_t n_cell_id = 0x1;
    //SIB1-NB
    //  period: 2560ms 256rf
    
    //sib1.repetitions = 4;
    //sib1.repetitions = 8;
    sib1.repetitions = 4;
    
    switch(sib1.repetitions){
        case 4:
            switch(n_cell_id & 0x3){
                case 0:
                    sib1.starting_rf = 0;    
                    break;
                case 1:
                    sib1.starting_rf = 16;     
                    break;   
                case 2:
                    sib1.starting_rf = 32;       
                    break; 
                case 3:
                    sib1.starting_rf = 48;       
                    break; 
                default: break;
            }
            break;
        case 8:
            if(n_cell_id & 0x1){
                sib1.starting_rf = 16;    
            }else{
                sib1.starting_rf = 0;
            }
            break;
        case 16:
            if(n_cell_id & 0x1){
                sib1.starting_rf = 1;    
            }else{
                sib1.starting_rf = 0;
            }
            break;
        default: break;
    }
}

void mac_scheduler(uint32_t frames){
	uint32_t cur_frame;
	uint32_t cur_subframe;
	//assert(scheduled_bitmap);
	for(cur_frame=0; cur_frame<frames; ++cur_frame){
		//	downlink 
		//#0
		dl_scheduled(cur_frame, 0, NPBCH, 0);//generate_broadcast_signal(cur_frame, NPBCH, (void *)0);
		
		//#4
		if(is_sib1_frame(cur_frame))
		    dl_scheduled(cur_frame, 4, NPDSCH, SI_RNTI);
		//#5
		dl_scheduled(cur_frame, 5, NPSS, 0);//generate_broadcast_signal(cur_frame, NPSS, (void *)0);
		
		
		//#9
		if(cur_frame&0x1);
		else{
			dl_scheduled(cur_frame, 9, NSSS, 0);//generate_broadcast_signal(cur_frame, NSSS, (void *)0);
		}
			
			
		//	uplink
	}
}

uint8_t is_sib1_frame(uint32_t cur_frame){
    sib1_nb_t s;
    s = sib1;
    
    int32_t nr_mod_256 = cur_frame & 0xff;
    
    int32_t temp1 = (nr_mod_256 % (256/s.repetitions)) - s.starting_rf;
    
    if(temp1 & 0x1 || temp1 < 0){
        return 0x0;
    }else{
        return 0x1;
    }
    
    /*
    switch(s.repetitions){
        case 4:
            if(((nr_mod_256 % 64) - s.starting_rf) >= 0){
                if(((nr_mod_256 % 64) - s.starting_rf) & 0x1){
                    return 0x0;
                }else{
                    return 0x1;
                }
            }else{
                return 0x0;   
            }
            break;
        case 8:
            break;
        case 16:
            break;
        default: break;
    }*/
}


