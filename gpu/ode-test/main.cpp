#include <common.h> // stdint.h, stdlib.h, stdio.h, gpu.h (psxgpu.h)
#include <psxapi.h>
#include <psxpad.h>

struct DisplayControlState {
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t x1 = 608;
    uint16_t x2 = 3168;
    uint16_t y1 = 16;
    uint16_t y2 = 256;
    uint16_t clockdiv_index = 0;
    bool dirty = false;
} g_dcs;

DISPENV disp;
DRAWENV draw;

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

    SetDefDispEnv(&disp, 0, 0, 640, 480);
    SetDefDrawEnv(&draw, 0, 0, 640, 480);

    disp.isinter = true;
    disp.isrgb24 = false;

    draw.dtd = true;
    draw.isbg = true;

    setRGB0(&draw, 0, 0, 0);

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);
}

void display() {
    //activeDb = !activeDb; 

    //PutDrawEnv(&db[activeDb].draw);
    //PutDispEnv(&db[activeDb].disp);

    DrawSync(0);

    VSync(0);

    clearScreenColor(0xFF, 0xFF, 0xFF);
    
    GP1_05h(g_dcs.x, g_dcs.y);
    GP1_06h(g_dcs.x1, g_dcs.x2);
    GP1_07h(g_dcs.y1, g_dcs.y2);
    GP1_ChangeHorizontalResolution(g_dcs.clockdiv_index);

    
    for (;;) // fix me
    {
        int hblanks = VSync(1);
        uint32_t gpu_stat = ReadGPUstat();
        int interlace_field = ((gpu_stat >> 13) & 0x01);
        int v_res = ((gpu_stat >> 19) & 0x01);
        int v_interlace = ((gpu_stat >> 22) & 0x01);
        int ode_bit = ((gpu_stat >> 31) & 0x01); //equivalent to GetODE()

        printf("hblanks %d, field %d, res %d, interlace %d, ode %d\n", hblanks, interlace_field, v_res, v_interlace, ode_bit);
    }
}

int main() {
    init();
    printf("gpu/ode-test\n");
    printf("(x, y) = (%d, %d)\n", g_dcs.x, g_dcs.y);

    while (1) {
        display();
    }

    return 0;
}
