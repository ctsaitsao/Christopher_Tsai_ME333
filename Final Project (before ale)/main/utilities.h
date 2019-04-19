enum mode_t {IDLE, PWM, ITEST, HOLD, TRACK}; //mode_t is a data type

void set_mode(enum mode_t new_mode); //new_mode is the variable here
enum mode_t get_mode(void);
