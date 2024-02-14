#include <rp6502.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "controller.h"
#include "homespun_font.h"

#define CHAR_WIDTH 7
#define CHAR_HEIGHT 8

#define BLACK  0
#define RED  1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
#define GREY 8

#define COLOR_MASK 0b1111

#define WIDTH 320
#define HEIGHT 240 // 180 or 240
#define BOARD_MARGIN_X 110
#define BOARD_MARGIN_Y 80
#define BOARDER_COLOR BLUE
#define GAME_OVER_DELAY 64
#define HUD_X 30
#define HUD_Y HEIGHT - 30

#define SNAKE_BASE 5
#define SNAKE_MAX 32
#define DRAW_CYCLE_DELAY 512

void main();

void draw_pixel(int16_t x, int16_t y, uint8_t color);

void draw_char(char c, int16_t x, int16_t y, uint8_t color) {
    int8_t i,j;
    c = (c & 0x7F);
    if (c < ' ') {
        c = 0;
    } else {
        c -= ' ';
    }
    for (j=0; j<CHAR_WIDTH; ++j)  {
        for (i=0; i<CHAR_HEIGHT; ++i) {

            if (font[c][j] & (1<<i)) {
                draw_pixel(x+j, y+i, color);
            }
        }
    }
}

void draw_pixel(int16_t x, int16_t y, uint8_t color) {
    RIA_ADDR0 = (x / 2) + (y * (WIDTH / 2));
    RIA_RW0 &= ~(COLOR_MASK << ((x % 2) * 4));
    RIA_RW0 |= (color << ((x % 2) * 4));
}

void draw_string(const char* str, int16_t x, int16_t y, uint8_t color) {
    while (*str) {
        draw_char(*str++, x, y, color);
        x += CHAR_WIDTH;
    }
}

static void wait() {
    uint8_t in_byte = 0;

    while (true) {
        if (RIA_RX_READY) {
            in_byte = RIA_RX;
        } else {
            in_byte = 0;
        }
        controller_read();
        if ((!(controller_buttons & STA_UNPRESSED)) || in_byte == ' ') {
            delay();
            delay();
            delay();
            delay();
            break;
        }

    }
}

static void long_delay() {
    uint8_t i;
    for (i = 0; i <= GAME_OVER_DELAY; ++i) {
        delay();
    }
}

static void vmode(uint16_t data) {
    xreg(data, 0, 1);
}

void draw_border(uint8_t color) {
    int16_t x, y;
    for (y = BOARD_MARGIN_Y; y <= HEIGHT - BOARD_MARGIN_Y; ++y) {
        draw_pixel(BOARD_MARGIN_X, y, color);
        draw_pixel(WIDTH - BOARD_MARGIN_X, y, color);
    }
    for (x = BOARD_MARGIN_X; x <= WIDTH - BOARD_MARGIN_X; ++x) {
        draw_pixel(x, BOARD_MARGIN_Y, color);
        draw_pixel(x, HEIGHT - BOARD_MARGIN_Y, color);
    }
}


static void clear(uint8_t color) {
    uint16_t i = 0;
    uint8_t vbyte;
    vbyte = (color << 4) | color;
    RIA_ADDR0 = 0;
    RIA_STEP0 = 1;
    
    // for (i = 0x1300; --i;)
    // {
    //     RIA_RW0 = 255;
    //     RIA_RW0 = 255;
    //     RIA_RW0 = 255;
    //     RIA_RW0 = 255;
    //     RIA_RW0 = 255;
    //     RIA_RW0 = 255;
    //     RIA_RW0 = 255;
    //     RIA_RW0 = 255;
    // }
    // RIA_ADDR0 = 0;
    for (i = 0x1300; --i;) {
        RIA_RW0 = vbyte;
        RIA_RW0 = vbyte;
        RIA_RW0 = vbyte;
        RIA_RW0 = vbyte;
        RIA_RW0 = vbyte;
        RIA_RW0 = vbyte;
        RIA_RW0 = vbyte;
        RIA_RW0 = vbyte;
    }
    RIA_ADDR0 = 0;
    RIA_STEP0 = 0;

}

void update_hud(uint8_t level, uint8_t color) {
    int buff_len = 3;

    char *level_string = (char*)malloc(buff_len * sizeof(char));

    sprintf(level_string, "%d", level + 1);
    
    draw_string(level_string, HUD_X + 49, HUD_Y, color);
    free(level_string);
}

void draw_hud(uint8_t level, uint8_t color) {
    draw_string("Level:", HUD_X, HUD_Y, color);
    update_hud(level, color);
}

void main() {
    int16_t velocity_x = 1, velocity_y = 0 ;
    int16_t pixel_x = (rand16() % (WIDTH - (2 * BOARD_MARGIN_X) - 1)) + BOARD_MARGIN_X + 1;
    int16_t pixel_y = (rand16() % (HEIGHT - (2 * BOARD_MARGIN_Y) - 1)) + BOARD_MARGIN_Y + 1;
    const uint8_t snake_color = GREEN;
    const uint8_t bg_color = BLACK;
    const uint8_t food_color = YELLOW;
    uint16_t last_buttons = 0xFFFF;
    char in_byte = 0;
    bool paused = false;
    uint8_t level = 0;
    uint8_t head = 31;
    uint8_t next_head;
    uint8_t tail;
    uint8_t i;
    uint8_t pixel_index;
    uint16_t skip_draw = DRAW_CYCLE_DELAY;
    int16_t snake_x[SNAKE_MAX] = {
        // 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        // 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        // 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        // 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        // 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        // 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        // 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        // 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        // 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        // 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        // 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        // 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        // 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        // 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };
    int16_t snake_y[SNAKE_MAX] = {
        0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
        0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        // 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
    };
    controller_init();
    #if (HEIGHT == 180)
        vmode(2);
    #else
        vmode(1);
    #endif
    clear(bg_color);
    draw_string("Snake", 140, 100, GREEN);
    draw_string("press start or space", 90, 140, GREY);

    wait();
    clear(bg_color);
    draw_border(BOARDER_COLOR);
    draw_hud(level, GREEN);
    tail = head - level - SNAKE_BASE;
    for (i = 0; i <= level + SNAKE_BASE; ++i) {
        pixel_index = (head - i) % SNAKE_MAX;
        draw_pixel(snake_x[pixel_index], snake_y[pixel_index], snake_color);
    }
    draw_pixel(pixel_x, pixel_y, food_color);


    while (true) {
        controller_read();
        if (RIA_RX_READY) {
            in_byte = RIA_RX & 0b11011111;
        } else {
            in_byte = 0;
        }

        if ((((last_buttons & STA_UNPRESSED) != (controller_buttons & STA_UNPRESSED))
        && !(controller_buttons & STA_UNPRESSED))
        || in_byte == ' ') {
            paused = !paused;
            delay();
            delay();
            delay();
            delay();
        }
        if (!paused) {
            if ((!(controller_buttons & UP_UNPRESSED)|| in_byte == 'W') && velocity_y != 1) {
                velocity_x = 0;
                velocity_y = -1;
            }
            if ((!(controller_buttons & DN_UNPRESSED) || in_byte == 'S') && velocity_y != -1) {
                velocity_x = 0;
                velocity_y = 1;
            }
            if ((!(controller_buttons & LT_UNPRESSED) || in_byte == 'A') && velocity_x != 1) {
                velocity_x = -1;
                velocity_y = 0;
            }
            if ((!(controller_buttons & RT_UNPRESSED) || in_byte == 'D') && velocity_x != -1) {
                velocity_x = 1;
                velocity_y = 0;
            }
            if (!skip_draw) {
                next_head =  (head + 1) % SNAKE_MAX;
                snake_x[next_head] = snake_x[head] + velocity_x;
                snake_y[next_head] = snake_y[head] + velocity_y;
                
                if (snake_x[next_head] <= BOARD_MARGIN_X
                || snake_y[next_head] <= BOARD_MARGIN_Y
                || snake_x[next_head] >= WIDTH - BOARD_MARGIN_X
                || snake_y[next_head] >= HEIGHT - BOARD_MARGIN_Y) {
                    draw_pixel(snake_x[tail], snake_y[tail], bg_color);
                    // for (i = 0; i <= level + SNAKE_BASE; ++i) {
                    //     pixel_index = (head - i) % SNAKE_MAX;
                    //     draw_pixel(snake_x[pixel_index], snake_y[pixel_index], RED);
                    // }
                    draw_string("Try again", 130, 100, RED);
                    long_delay();
                    main();
                }
                RIA_ADDR0 = (snake_x[next_head] / 2) + (snake_y[next_head] * (WIDTH / 2));
                if (((RIA_RW0 >> ((snake_x[next_head] % 2) * 4)) & COLOR_MASK) == snake_color) {
                    draw_pixel(snake_x[tail], snake_y[tail], bg_color);
                    // for (i = 0; i <= level + SNAKE_BASE; ++i) {
                    //     pixel_index = (head - i) % SNAKE_MAX;
                    //     draw_pixel(snake_x[pixel_index], snake_y[pixel_index], RED);
                    // }
                    draw_string("Try again", 130, 100, RED);
                    long_delay();
                    main();
                }
                RIA_RW0 &= ~(COLOR_MASK << ((snake_x[next_head] % 2) * 4));
                RIA_RW0 |= (snake_color << ((snake_x[next_head] % 2) * 4));
                if (snake_x[next_head] == pixel_x && snake_y[next_head] == pixel_y) {
                    pixel_x = (rand16() % (WIDTH - (2 * BOARD_MARGIN_X) - 1)) + BOARD_MARGIN_X + 1;
                    pixel_y = (rand16() % (HEIGHT - (2 * BOARD_MARGIN_Y) - 1)) + BOARD_MARGIN_Y + 1;
                    if (level == 25) {
                        draw_string("Good Job", 130, 100, GREEN);
                        long_delay();
                        main();
                    } else {
                        draw_pixel(pixel_x, pixel_y, food_color);
                        update_hud(level, bg_color);
                        level += 1;
                        update_hud(level, GREEN);
                    }
                } else {
                    draw_pixel(snake_x[tail], snake_y[tail], bg_color);
                    tail = (tail + 1) % SNAKE_MAX;
                }
                head =  next_head;
                skip_draw = DRAW_CYCLE_DELAY - (level * 16);
            }
            skip_draw -= 1;
        }
        last_buttons = controller_buttons;
    }

}
