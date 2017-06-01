#include "si.h"


static sib1_nb_sched_t sib1_sched;
static sib1_nb_t sib1;

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

uint32_t get_num_sf_tx(uint8_t sibs){
	return sib1.scheduling_info[sibs-1].num_sf_tx;
}

void decrease_num_sf_tx(uint8_t sibs){
	--sib1.scheduling_info[sibs-1].num_sf_tx;
}

