#include <signal_prcs.h>
#include <indc_ctrl.h>

int ring_buff[ring_buff_size];
int write_pnt;
float read_pnt;
float crossfade;

float filter_init(float freq)
{
	//частота среза фильтра
	float rc = 1.0/(2.0*3.14159*freq);
	return (rc / (rc + 1.0/44000.0));
}

int pitch_func(int sample)
{
	int sum = sample;

	//write to ring buffer
	ring_buff[write_pnt] = sum;

	//read fractional readpointer and generate 0° and 180° read-pointer in integer
	int read_pnt_int = roundf(read_pnt);
	int read_pnt_int_2 = 0;
	if (read_pnt_int >= ring_buff_size/2) read_pnt_int_2 = read_pnt_int - (ring_buff_size/2);
	else read_pnt_int_2 = read_pnt_int + (ring_buff_size/2);

	//read the two samples...
	float read_value_1 = (float) ring_buff[read_pnt_int];
	float read_value_2 = (float) ring_buff[read_pnt_int_2];

	//Check if first readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to second read-pointer
	if (overlap >= (write_pnt-read_pnt_int) && (write_pnt-read_pnt_int) >= 0 && shift!=1.0f)
	{
		int rel = write_pnt-read_pnt_int;
		crossfade = ((float)rel)/(float)overlap;
	}
	else if (write_pnt-read_pnt_int == 0) crossfade = 0.0f;

	//Check if second readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to first read-pointer
	if (overlap >= (write_pnt-read_pnt_int_2) && (write_pnt-read_pnt_int_2) >= 0 && shift!=1.0f)
	{
			int rel = write_pnt-read_pnt_int_2;
			crossfade = 1.0f - ((float)rel)/(float)overlap;
	}
	else if (write_pnt-read_pnt_int_2 == 0) crossfade = 1.0f;

	//do cross-fading and sum up
	sum = (read_value_1*crossfade + read_value_2*(1.0f-crossfade));

	//increment fractional read-pointer and write-pointer
	read_pnt += shift;
	write_pnt++;
	if (write_pnt == ring_buff_size) write_pnt = 0;
	if (roundf(read_pnt) >= ring_buff_size) read_pnt = 0.0f;

	return sum;

}

