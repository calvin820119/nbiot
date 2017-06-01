#ifndef _SYSTEM_INFORMATION_H_
#define _SYSTEM_INFORMATION_H_ 

#include "types.h"



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


void init_SI();
uint8_t is_sibs_frame(uint32_t cur_frame);
uint8_t is_sib1_frame(uint32_t cur_frame);
uint32_t get_num_sf_tx(uint8_t sibs);
void decrease_num_sf_tx(uint8_t sibs);


#endif
