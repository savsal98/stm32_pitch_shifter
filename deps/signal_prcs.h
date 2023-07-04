#include <stm32f30x.h>
#include <math.h>

#define BufSize 4000
#define Overlap 500

typedef struct filters_data {
	  int32_t filtered_now;
	  int32_t filtered_prev;
	  int32_t input_now;
	  int32_t input_prev;
	  float alpha;

} hpf_data;

extern int Buf[BufSize];
extern int WtrP;
extern float Rd_P;
extern float CrossFade;

int DO_PITCH(int Sample);
float FILTER_INIT(float freq);

