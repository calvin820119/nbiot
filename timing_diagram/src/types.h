
#ifndef _TYPES_H_
#define _TYPES_H_

#define num_subframe_per_frame 10

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





typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned long uint32_t;
typedef long int32_t;

#endif
