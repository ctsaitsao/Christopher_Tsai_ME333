#ifndef UTILITIES__H__
#define UTILITIES__H__
#define PLOTPTS 100

enum mode_t {IDLE, PWM, ITEST, HOLD, TRACK}; //mode_t is a data type

void set_mode(enum mode_t new_mode); //new_mode is the variable here
enum mode_t get_mode(void);

int get_duty_cycle(void);

int get_dir(void);

int get_Kp_c(void);
int get_current_Ki(void);
void set_current_gains(void);

void sent_data(int r_data,int adc_data, int Str_data);
int get_Str_data(void);
void get_data(int* r, int* adc);

int get_position_Kp(void);
int get_position_Ki(void);
int get_Kd_p(void);
void set_positions_gains(void);

int get_angle(void);
int get_desired_current(void);
void send_current(int u);

void send_ref_array(int* ref_array,int size_ref);
int get_ref_array(int* array);
void get_data_2(int* r, int* adc);

#endif
