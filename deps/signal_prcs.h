#include <stm32f30x.h>
#include <math.h>

#define ring_buff_size 4000
#define overlap 500

typedef struct filters_data {
	  int32_t filtered_now;
	  int32_t filtered_prev;
	  int32_t input_now;
	  int32_t input_prev;
	  float alpha;

} hpf_data;

extern int ring_buff[ring_buff_size];
extern int write_pnt;
extern float read_pnt;
extern float crossfade;

int pitch_func(int sample);
float filter_init(float freq);

