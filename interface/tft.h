#ifndef TFT_H_
#define TFT_H_

void display_calibrate_touch(void);
void display_init(void);
void display_loop(void);
void display_process_touch(void);
void display_process_touch_buttons(void);
void display_process_touch_dial(void);
void display_update(void);
void display_query_controller_state(void);

#endif /* TFT_H_ */
