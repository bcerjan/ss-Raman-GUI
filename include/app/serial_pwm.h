#ifndef SERIAL_PWM
#def SERIAL_PWM

char **get_port_list(int *len);
void free_port_list(char **list);
void set_duty_cycle(gchar port_id, float percent);

#endif
