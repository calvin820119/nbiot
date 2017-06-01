#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "render.h"
#include "si.h"


#define BV(X) (0x1<<X)

uint8_t number_char[17] = "0123456789ABCDEF";
//	user define sibs message for UI
uint8_t sibs_str[6][20] = { "LIST1", "LIST2", "LIST3", "LIST4", "LIST5", "LIST6"};

void generate_broadcast_signal(uint32_t cur_frame, channel_t channel, void *content);
void mac_scheduler(uint32_t frames);

extern resource_t *dl_scheduled_bitmap[10];
extern resource_t *ul_scheduled_bitmap[48][10];

uint8_t output_file_name[] = "../../../out/output.html";
uint8_t template_file_name[] = "../ref/template.html";



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
		    dl_scheduled(cur_frame, 4, NPDSCH, SI_RNTI, "SIB1");
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
		    while(get_num_sf_tx(sibs) > 0 && sf>=0){
		        while(downlink_bitmap&BV(sf)){	//	find free subframe
                    ++sf;
                    if(sf==10){
                        sf=-1;
                        break;
                    }
                }
                if(sf>=0){
                    dl_scheduled(cur_frame, sf, NPDSCH, SI_RNTI, sibs_str[sibs-1]);
    		        downlink_bitmap |= BV(sf);
    		        decrease_num_sf_tx(sibs);
    		        ++sf;
                }
            }
        }
        
		//	uplink
		
	}
}



