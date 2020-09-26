#include <common.h> // stdint.h, stdlib.h, stdio.h, gpu.h (psxgpu.h)
#include <psxapi.h>
#include <psxpad.h>

struct DisplayControlState {
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t x1 = 616;
    uint16_t x2 = 620;
    uint16_t y1 = 16;
    uint16_t y2 = 256;
    uint16_t clockdiv_index = 0;
    bool dirty = false;
} g_dcs;

#define SCR_W 320
#define SCR_H 480

struct DB { 
    DISPENV disp;
    DRAWENV draw;
};

DB db[2]; 
int activeDb = 0; // Double buffering index

char pad_buff[2][34];
uint16_t prev_buttons = 0xFFFF;
uint16_t curr_buttons = 0xFFFF;

bool buttonPressed(uint16_t button) {
    return ((curr_buttons & button) == 0) && ((prev_buttons & button) != 0);
}

void reportHorizontalRange() {
    printf("x1: %u, x2: %u\n", g_dcs.x1, g_dcs.x2);
}

void pollPad() {
    PADTYPE* pad = ((PADTYPE *)&pad_buff[0][0]);

    // Status is 0 if receive is successful
    // Only support input for digital, dual analog, and DualShock controllers for now
    if (pad->stat == 0 && (pad->type == 0x04 || pad->type == 0x07)) {

        prev_buttons = curr_buttons;
        curr_buttons = pad->btn;

        if (buttonPressed(PAD_LEFT)) {
            --g_dcs.x1;
            reportHorizontalRange();
        }
        if (buttonPressed(PAD_RIGHT)) {
            ++g_dcs.x1;
            reportHorizontalRange();
        }
        if (buttonPressed(PAD_SQUARE)) {
            --g_dcs.x2;
            reportHorizontalRange();
        }
        if (buttonPressed(PAD_CIRCLE)) {
            ++g_dcs.x2;
            reportHorizontalRange();
        }
        if (buttonPressed(PAD_TRIANGLE)) {
            ++g_dcs.clockdiv_index;

            if (g_dcs.clockdiv_index >= 5)
                g_dcs.clockdiv_index = 0;

            uint32_t clockdivs[5] = {10, 8, 5, 4, 7};
            printf("Using clockdiv %u\n", clockdivs[g_dcs.clockdiv_index]);
        }
    }
    else {
        prev_buttons = 0xFFFF;
        curr_buttons = 0xFFFF;
    }
}

/* GP1 wrappers */

// VRAM address of display area
void GP1_05h(uint16_t x, uint16_t y) {
    writeGP1(0x05, ((y & 0x1FF) << 10) | (x & 0x3FF));
}

// Horizontal display range
void GP1_06h(uint16_t x1, uint16_t x2) {
    writeGP1(0x06, ((x2 & 0xFFF) << 12) | (x1 & 0xFFF));
}

// Vertical display range
void GP1_07h(uint16_t y1, uint16_t y2) {
    writeGP1(0x07, ((y2 & 0x3FF) << 10) | (y1 & 0x3FF));
}

uint32_t clockdiv_masks[5] = {0x000000, 0x000001, 0x000002, 0x000003, 0x000040};

// Change horizontal 
void GP1_ChangeHorizontalResolution(uint32_t index)
{
    // Run with fixed GPU configuration in other areas for now
    writeGP1(0x08, (0x000024 | clockdiv_masks[index]));
}

void init() {
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    for (int i = 0; i<=1; i++) {
        SetDefDispEnv(&db[i].disp, 0, !i ? 0 : SCR_H, SCR_W, SCR_H);
        SetDefDrawEnv(&db[i].draw, 0, !i ? SCR_H : 0, SCR_W, SCR_H);

        db[i].disp.isinter = true;
        db[i].disp.isrgb24 = false;

        db[i].draw.dtd = true;
        db[i].draw.isbg = true; // Clear bg on PutDrawEnv

        setRGB0(&db[i].draw, 0, 0, 0);
    }
    activeDb = 0;

	PutDrawEnv(&db[activeDb].draw);
    PutDispEnv(&db[activeDb].disp);

    SetDispMask(1);

    InitPAD(&pad_buff[0][0], 34, &pad_buff[1][0], 34);
    StartPAD();
    ChangeClearPAD(0);
}

void display() {
    //activeDb = !activeDb; 

    //PutDrawEnv(&db[activeDb].draw);
    //PutDispEnv(&db[activeDb].disp);

    DrawSync(0);

    clearScreenColor(0xFF, 0xFF, 0xFF);
    
    GP1_05h(g_dcs.x, g_dcs.y);
    GP1_06h(g_dcs.x1, g_dcs.x2);
    GP1_07h(g_dcs.y1, g_dcs.y2);
    GP1_ChangeHorizontalResolution(g_dcs.clockdiv_index);

    VSync(0);
}

int main() {
    init();
    printf("gpu/display-control\n");

    //g_dcs.dirty = false;

    while (1) {
        pollPad();

        display();
    }

    return 0;
}
