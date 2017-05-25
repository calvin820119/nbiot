#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "render.h"
#include "types.h"

#define BV(X) (0x1<<X)

void generate_broadcast_signal(uint32_t cur_frame, channel_t channel, void *content);
void mac_scheduler(uint32_t frames);
uint8_t is_sibs_frame(uint32_t cur_frame);
uint8_t is_sib1_frame(uint32_t cur_frame);
void init_SI();

extern resource_t *dl_scheduled_bitmap[10];
extern resource_t *ul_scheduled_bitmap[48][10];
uint8_t output_file_name[] = "../../../out/output.html";
uint8_t template_file_name[] = "../ref/template.html";

typedef struct sib1_nb_sched_s{
    uint8_t repetitions;    //  4, 8, 16
    uint8_t starting_rf;
}sib1_nb_sched_t;

typedef enum si_window_length_e{
    ms160=16,
    ms320=32,
    ms480=48,
    ms640=64,
    ms960=96,
    ms1280=128,
    ms1600=160
}si_window_length_t;



typedef enum si_periodicity_e{
    rf64=64, 
    rf128=128, 
    rf256=256, 
    rf512=512,
	rf1024=1024, 
    rf2048=2048, 
    rf4096=4096
}si_periodicity_t;

typedef enum si_repetition_pattern_e{
    every2ndRF=2, 
    every4thRF=4,  
	every8thRF=8,  
    every16thRF=16
}si_repetition_pattern_t;

typedef enum si_tb_e{
    b56=2, 
    b120=2, 
    b208=8, 
    b256=8, 
    b328=8, 
    b440=8, 
    b552=8, 
    b680=8
}si_tb_t;

typedef enum sibs_e{
    sib2_v=0x1,
    sib3_v=0x2,
    sib4_v=0x4,
    sib5_v=0x8,
    sib14_v=0x10,
    sib16_v=0x20
}sibs_t;

typedef struct sibs_nb_sched_s{
    si_periodicity_t si_periodicity;
    si_repetition_pattern_t si_repetition_pattern;
    sibs_t sib_mapping_info;   //bit flag
    si_tb_t si_tb;
    
    int16_t num_sf_tx;
    int16_t repetition_pattern_counter;
}sibs_nb_sched_t;

typedef struct sib1_nb_s{
    sibs_nb_sched_t scheduling_info[6];
    si_window_length_t si_window_length;
    uint8_t si_radio_frame_offset;
    uint8_t sizeof_scheduling_info;
}sib1_nb_t;

sib1_nb_sched_t sib1_sched;
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
		dl_scheduled_bitmap[i] = (resource_t *)calloc(num_total_frame, sizeof(resource_t));
		for(ii=0;ii<48;++ii)
		ul_scheduled_bitmap[ii][i] = (resource_t *)calloc(num_total_frame, sizeof(resource_t));
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

void init_SI(){
    uint32_t n_cell_id = 0x1;
    
    //SIB1-NB
    //  period: 2560ms 256rf
    
    //sib1.repetitions = 4;
    //sib1.repetitions = 8;
    sib1_sched.repetitions = 4;
    
    switch(sib1_sched.repetitions){
        case 4:
            switch(n_cell_id & 0x3){
                case 0:
                    sib1_sched.starting_rf = 0;    
                    break;
                case 1:
                    sib1_sched.starting_rf = 16;     
                    break;   
                case 2:
                    sib1_sched.starting_rf = 32;       
                    break; 
                case 3:
                    sib1_sched.starting_rf = 48;       
                    break; 
                default: break;
            }
            break;
        case 8:
            if(n_cell_id & 0x1){
                sib1_sched.starting_rf = 16;    
            }else{
                sib1_sched.starting_rf = 0;
            }
            break;
        case 16:
            if(n_cell_id & 0x1){
                sib1_sched.starting_rf = 1;    
            }else{
                sib1_sched.starting_rf = 0;
            }
            break;
        default: break;
    }
    
    sib1.si_radio_frame_offset = 0;
    sib1.si_window_length = ms160;
    sib1.sizeof_scheduling_info = 0;
    
    sib1.scheduling_info[0].sib_mapping_info = sib2_v | sib3_v;
    sib1.scheduling_info[0].si_periodicity = rf64;
    sib1.scheduling_info[0].si_repetition_pattern = every4thRF;
    sib1.scheduling_info[0].si_tb = b680;
    ++sib1.sizeof_scheduling_info;
    
    sib1.scheduling_info[1].sib_mapping_info = sib4_v | sib5_v;
    sib1.scheduling_info[1].si_periodicity = rf128;
    sib1.scheduling_info[1].si_repetition_pattern = every2ndRF;
    sib1.scheduling_info[1].si_tb = b680;
    ++sib1.sizeof_scheduling_info;
    
    sib1.scheduling_info[2].sib_mapping_info = sib14_v | sib16_v;
    sib1.scheduling_info[2].si_periodicity = rf256;
    sib1.scheduling_info[2].si_repetition_pattern = every8thRF;
    sib1.scheduling_info[2].si_tb = b680;
    ++sib1.sizeof_scheduling_info;
}

void mac_scheduler(uint32_t frames){
	uint32_t cur_frame;
	uint32_t cur_subframe;
	uint16_t downlink_bitmap;
	int32_t sf;
	uint8_t ret;
	uint8_t sibs;
	//assert(scheduled_bitmap);
	for(cur_frame=0; cur_frame<frames; ++cur_frame){
		downlink_bitmap = 0x000;
        //	downlink 
		//#0
		dl_scheduled(cur_frame, 0, NPBCH, 0, (uint8_t *)0);//generate_broadcast_signal(cur_frame, NPBCH, (void *)0);
		downlink_bitmap |= BV(0);
		
		//#4
		if(is_sib1_frame(cur_frame)){
		    dl_scheduled(cur_frame, 4, NPDSCH, SI_RNTI, "1");
		    downlink_bitmap |= BV(4);
        }
		    
		//#5
		dl_scheduled(cur_frame, 5, NPSS, 0, (uint8_t *)0);//generate_broadcast_signal(cur_frame, NPSS, (void *)0);
		downlink_bitmap |= BV(5);
		
		//#9
		if(cur_frame&0x1);
		else{
		    dl_scheduled(cur_frame, 9, NSSS, 0, (uint8_t *)0);//generate_broadcast_signal(cur_frame, NSSS, (void *)0);
	        downlink_bitmap |= BV(9);
        }
		
		if((sibs=is_sibs_frame(cur_frame))!=0x0){     //  SIBs every frame only search one first available subframe in this version, TODO modified according to the si-TB
		    sf=0;
		    //printf("sibs %d\n", sibs-1);
		    //printf("num_sf_tx %d\n", sib1.scheduling_info[sibs-1].num_sf_tx);
		    while(sib1.scheduling_info[sibs-1].num_sf_tx > 0 && sf>=0){
		        while(downlink_bitmap&BV(sf)){
                    ++sf;
                    if(sf==10){
                        sf=-1;
                        break;
                    }
                }
                if(sf>=0){
                    //printf("valid sf %d\n", i);
                    dl_scheduled(cur_frame, sf, NPDSCH, SI_RNTI, "SI-RNTI");
    		        downlink_bitmap |= BV(sf);
    		        --sib1.scheduling_info[sibs-1].num_sf_tx;
    		        ++sf;
                }
            }
        }
        
		//	uplink
		
		
	}
}

uint8_t is_sib1_frame(uint32_t cur_frame){
    sib1_nb_sched_t s;
    s = sib1_sched;
    
    int32_t nr_mod_256 = cur_frame & 0xff;
    
    int32_t temp1 = (nr_mod_256 % (256/s.repetitions)) - s.starting_rf;
    //printf("%d\n", 256/s.repetitions);
    if(temp1 & 0x1 || temp1 < 0 || temp1 >= 16 ){
        return 0x0;
    }else{
        return 0x1;
    }
}

uint8_t is_sibs_frame(uint32_t cur_frame){
    uint8_t i;
    uint32_t x;
    
   // printf("%d\n", cur_frame);
    
    for(i=0;i<sib1.sizeof_scheduling_info;++i){
        
        x = i*sib1.si_window_length;
     //   printf("\t%d\n", sib1.scheduling_info[i].si_periodicity);
        if( cur_frame % sib1.scheduling_info[i].si_periodicity == (x+sib1.si_radio_frame_offset) ){
            //printf("frame[%d] sibs[%d] started \n", cur_frame, i); system("pause");
            sib1.scheduling_info[i].repetition_pattern_counter = sib1.si_window_length / sib1.scheduling_info[i].si_repetition_pattern;
            sib1.scheduling_info[i].num_sf_tx = sib1.scheduling_info[i].si_tb;
        }
        
        if( sib1.scheduling_info[i].repetition_pattern_counter > 0 ){
           // printf("frame[%d] sibs[%d] %d \n", cur_frame, i, sib1.scheduling_info[i].budget); system("pause");
            if( (cur_frame % sib1.scheduling_info[i].si_periodicity) % sib1.scheduling_info[i].si_repetition_pattern == 0x0){
                --sib1.scheduling_info[i].repetition_pattern_counter;
                return i+1;
            }
        }
    }
    
    return 0x0;
}

