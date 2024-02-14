#define IS_SNES         0b0000000100000000
#define NA1_UNPRESSED   0b0000001000000000
#define NA2_UNPRESSED   0b0000010000000000
#define NA3_UNPRESSED   0b0000100000000000
#define R_UNPRESSED     0b0001000000000000
#define L_UNPRESSED     0b0010000000000000
#define X_UNPRESSED     0b0100000000000000
#define A_UNPRESSED     0b1000000000000000

#define RT_UNPRESSED    0b0000000000000001
#define LT_UNPRESSED    0b0000000000000010
#define DN_UNPRESSED    0b0000000000000100
#define UP_UNPRESSED    0b0000000000001000
#define STA_UNPRESSED   0b0000000000010000
#define SEL_UNPRESSED   0b0000000000100000
#define Y_UNPRESSED     0b0000000001000000
#define B_UNPRESSED     0b0000000010000000

#define DPAD_UNPRESSED  0b0000000000001111

extern uint16_t controller_buttons;
#pragma zpsym("controller_buttons")

/* initializes the VIA for connecting 2 (S)NES controllers on Port A*/
extern void controller_init();

/* reads 2 bytes of (S)NES controller 1 button press state*/
extern void controller_read();

/* spin CPU for short delay*/
extern void delay();
