//#include "tms9918a.h"
//#include <stdio.h>
//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>
//#include "ports.h"
//
//struct tms9918a *vdp;
//struct tms9918a_renderer *vdp_renderer;
//
//static uint32_t vdp_ctab[16] = {
//    0xFF000000,		/* transparent (we render as black) */
//    0xFF000000,		/* black */
//    0xFF20C020,		/* green */
//    0xFF60D060,		/* light green */
//
//    0xFF2020D0,		/* blue */
//    0xFF4060D0,		/* light blue */
//    0xFFA02020,		/* dark red */
//    0xFF40C0D0,		/* cyan */
//
//    0xFFD02020,		/* red */
//    0xFFD06060,		/* light red */
//    0xFFC0C020,		/* dark yellow */
//    0xFFC0C080,		/* yellow */
//
//    0xFF208020,		/* dark green */
//    0xFFC040A0,		/* magneta */
//    0xFFA0A0A0,		/* grey */
//    0xFFD0D0D0		/* white */
//};
//
///*
// *	Sprites
// */
//
// /*
//  *	Draw a horizontal slice of a sprite into the render buffer. If it
//  *	needs magnifying then do the magnification as we render. We use a
//  *	simple line buffer to detect collisions
//  */
//static void tms9918a_render_slice(struct tms9918a* vdp, int y, uint8_t* sprat, uint16_t bits, unsigned int width)
//{
//    int x = sprat[1];
//    uint32_t* pixptr = vdp->rasterbuffer + 256 * y;
//    uint8_t* colptr = vdp->colbuf + 32;
//    uint32_t foreground = vdp->colourmap[sprat[3] & 0x0F];
//    int mag = vdp->reg[1] & 0x01;
//    int step = 1;
//    int i;
//
//    if (sprat[3] & 0x80)
//        x -= 32;
//    pixptr += x;
//    colptr += x;
//
//    /* Walk across the sprite doing rendering and collisions. Collisions apply
//       to offscreen objects. Colbuf is sized to cover this */
//    for (i = x; i < x + (int)width; i++) {
//        if (i >= 0 && i <= 255) {
//            if (bits & 0x8000U)
//                *pixptr++ = foreground;
//        }
//        /* This pixel was already sprite written - collision */
//        if (*colptr) {
//            vdp->status |= 0x20;
//            *colptr++ = 1;
//        }
//        else {
//            pixptr++;
//            colptr++;
//        }
//        /* For magnified sprites write each pixel twice */
//        step ^= mag;
//        if (step == 1)
//            bits >>= 1;
//    }
//}
//
///*
// *	Calculate the slice of a sprite to render and feed it to the actual
// *	bit renderer.
// */
//static void tms9918a_render_sprite(struct tms9918a* vdp, int y, uint8_t* sprat, uint8_t* spdat)
//{
//    int row = *sprat;
//    uint16_t bits;
//    unsigned int width = 8;
//
//    /* Figure out the right data row */
//    if (row >= 0xE1)
//        row -= 0x100;	/* Signed top border */
//    row = y - row;
//    /* Get the data and expand it if needed */
//    if ((vdp->reg[1] & 0x02) == 0) {
//        spdat += row;
//        width = 8;
//        bits = *spdat << 24;
//    }
//    else {
//        spdat += 2 * row;
//        bits = *spdat++ << 8;
//        bits |= spdat[16];
//        width = 16;
//    }
//    tms9918a_render_slice(vdp, y, sprat, bits, width);
//}
//
///*
// *	Composite the sprites for a given scan line
// */
//static void tms9918a_sprite_line(struct tms9918a* vdp, int y)
//{
//    uint8_t* sphead[4];
//    uint8_t** spqueue = sphead;
//    uint8_t* sprat = vdp->framebuffer + ((vdp->reg[5] & 0x7F) << 7);
//    uint8_t* spdat = vdp->framebuffer + ((vdp->reg[6] & 0x07) << 11);
//    int i;
//    unsigned int spheight = vdp->reg[1] & 0x02 ? 16 : 8;
//    unsigned int mag = vdp->reg[1] & 0x01;
//    unsigned int ns = 0;
//    unsigned int spshft = (spheight == 8) ? 3 : 5;
//
//    if (mag)
//        spheight <<= 1;
//
//    /* Clear the collision buffer for the line */
//    memset(vdp->colbuf, 0, sizeof(vdp->colbuf));
//
//    /* Walk the sprite table and queue any sprite on this line */
//    for (i = 0; i < 32; i++) {
//        if (*sprat == 0xD0)
//            return;
//        if (*sprat <= y && *sprat + spheight >= y) {
//            ns++;
//            /* Too many sprites: only 4 get handled */
//            /* Q: do the full 32 get collision detected ? */
//            if (ns > 4) {
//                vdp->status |= 0x40 | i;	/* Too many sprites */
//                break;
//            }
//            *spqueue++ = sprat;
//        }
//        sprat += 4;
//    }
//    /* We need to render the ones we got in reverse order to get right
//       pixel priority */
//    while (spqueue > sphead) {
//        sprat = *--spqueue;
//        tms9918a_render_sprite(vdp, y, sprat, spdat + (sprat[2] << spshft));
//    }
//}
//
///*
// *	Add sprites to the raster image
// *
// *	BUG?: Do we need to do a pure collision sweep for the lines above
// *	and below the picture ?
// */
//static void tms9918a_raster_sprites(struct tms9918a* vdp)
//{
//    unsigned int i;
//    for (i = 0; i < 192; i++)
//        tms9918a_sprite_line(vdp, i);
//}
//
///*
// *	G1 - colour data from a character tied colour map
// */
//static void tms9918a_raster_pattern_g1(struct tms9918a* vdp, uint8_t code, uint8_t* pattern, uint8_t* colour, uint32_t* out)
//{
//    unsigned int x, y;
//    uint32_t foreground, background;
//    uint8_t bits;
//
//    pattern += code << 3;
//    colour += code >> 3;
//    foreground = vdp->colourmap[*colour >> 4];
//    background = vdp->colourmap[*colour & 0x0F];
//
//    for (y = 0; y < 8; y++) {
//        bits = *pattern++;
//        for (x = 0; x < 8; x++) {
//            if (bits & 0x80)
//                *out++ = foreground;
//            else
//                *out++ = background;
//            bits <<= 1;
//        }
//        out += 248;
//    }
//}
//
///*
// *	768 characters, 256 byte pattern table, colur table holds fg/bg
// *	colour for each group of 8 symbols
// */
//static void tms9918a_rasterize_g1(struct tms9918a* vdp)
//{
//    unsigned int x, y;
//    uint8_t* p = vdp->framebuffer + ((vdp->reg[2] & 0x0F) << 10);
//    uint8_t* pattern = vdp->framebuffer + ((vdp->reg[4] & 0x07) << 11);
//    uint8_t* colour = vdp->framebuffer + (vdp->reg[3] << 6);
//    uint32_t* fp = vdp->rasterbuffer;
//
//    for (y = 0; y < 8; y++) {
//        for (x = 0; x < 32; x++) {
//            tms9918a_raster_pattern_g1(vdp, *p++, pattern, colour, fp);
//            fp += 8;
//        }
//        fp += 7 * 256;
//    }
//
//    for (; y < 16; y++) {
//        for (x = 0; x < 32; x++) {
//            tms9918a_raster_pattern_g1(vdp, *p++, pattern, colour, fp);
//            fp += 8;
//        }
//        fp += 7 * 256;
//    }
//
//    for (; y < 24; y++) {
//        for (x = 0; x < 32; x++) {
//            tms9918a_raster_pattern_g1(vdp, *p++, pattern, colour, fp);
//            fp += 8;
//        }
//        fp += 7 * 256;
//    }
//    tms9918a_raster_sprites(vdp);
//}
//
///*
// *	In G2 mode we have two colours per row of the pattern
// */
//static void tms9918a_raster_pattern_g2(struct tms9918a* vdp, uint8_t code, uint8_t* pattern, uint8_t* colour, uint32_t* out)
//{
//    unsigned int x, y;
//    uint32_t foreground, background;
//    uint8_t bits;
//
//    pattern += code << 3;
//    colour += code << 3;
//
//    for (y = 0; y < 8; y++) {
//        bits = *pattern++;
//        foreground = vdp->colourmap[*colour >> 4];
//        background = vdp->colourmap[*colour++ & 0x0F];
//        for (x = 0; x < 8; x++) {
//            if (bits & 0x80)
//                *out++ = foreground;
//            else
//                *out++ = background;
//            bits <<= 1;
//        }
//        out += 248;
//    }
//}
//
///*
// *	768 characters, 768 patterns, two colours per character row
// *	Patterns and colour must be on 0x2000 boundaries
// */
//static void tms9918a_rasterize_g2(struct tms9918a* vdp)
//{
//    unsigned int x, y;
//    uint8_t* p = vdp->framebuffer + ((vdp->reg[2] & 0x0F) << 10);
//    uint8_t* pattern = vdp->framebuffer + ((vdp->reg[4] & 0x04) << 11);
//    uint8_t* colour = vdp->framebuffer + ((vdp->reg[3] & 0x80) << 6);
//    uint32_t* fp = vdp->rasterbuffer;
//
//    uint8_t* pattern0 = pattern;
//    uint8_t* colour0 = colour;
//
//    for (y = 0; y < 8; y++) {
//        for (x = 0; x < 32; x++) {
//            tms9918a_raster_pattern_g2(vdp, *p++, pattern, colour, fp);
//            fp += 8;
//        }
//        fp += 7 * 256;
//    }
//
//    if (vdp->reg[4] & 0x01)
//        pattern += 0x0800;
//    if (vdp->reg[3] & 0x20)
//        colour += 0x0800;
//
//    for (; y < 16; y++) {
//        for (x = 0; x < 32; x++) {
//            tms9918a_raster_pattern_g2(vdp, *p++, pattern, colour, fp);
//            fp += 8;
//        }
//        fp += 7 * 256;
//    }
//
//    /* Oddly these don't appear to be incremental but each chunk is relative
//       to base. I guess it makes more sense in logic to mask in the bits */
//    if (vdp->reg[4] & 0x02)
//        pattern = pattern0 + 0x1000;
//    if (vdp->reg[3] & 0x40)
//        colour = colour0 + 0x1000;
//
//    for (; y < 24; y++) {
//        for (x = 0; x < 32; x++) {
//            tms9918a_raster_pattern_g2(vdp, *p++, pattern, colour, fp);
//            fp += 8;
//        }
//        fp += 7 * 256;
//    }
//    tms9918a_raster_sprites(vdp);
//}
//
///* Rasterize a 4 x 4 pixel block */
//static void quad(uint32_t* p, uint32_t code)
//{
//    *p++ = code;
//    *p++ = code;
//    *p++ = code;
//    *p = code;
//    p += 253;
//    *p++ = code;
//    *p++ = code;
//    *p++ = code;
//    *p = code;
//    p += 253;
//    *p++ = code;
//    *p++ = code;
//    *p++ = code;
//    *p = code;
//    p += 253;
//    *p++ = code;
//    *p++ = code;
//    *p++ = code;
//    *p++ = code;
//}
//
//static void tms9918a_raster_multi(struct tms9918a* vdp, uint8_t code, uint8_t* pattern, uint32_t* out)
//{
//    uint8_t px;
//    pattern += code << 3;
//    px = *pattern++;
//    quad(out, vdp->colourmap[px >> 4]);
//    quad(out + 4, vdp->colourmap[px & 15]);
//    out += 4 * 256;
//    px = *pattern;
//    quad(out, vdp->colourmap[px >> 4]);
//    quad(out + 4, vdp->colourmap[px & 15]);
//}
//
///* Aka semi-graphics - this mode is almost never used. Each character
//   is now a 2 byte pattern describing four squares in 16 colour (15 + bg).
//   The row low bits provides the upper 2bits of the pattern code so that
//   they are interleaved and all used */
//static void tms9918a_rasterize_mc(struct tms9918a* vdp)
//{
//    unsigned int x, y;
//    uint8_t* p = vdp->framebuffer + ((vdp->reg[2] & 0x0F) << 10);
//    uint8_t* pattern = vdp->framebuffer + ((vdp->reg[4] & 0x07) << 11);
//    uint32_t* fp = vdp->rasterbuffer;
//
//    for (y = 0; y < 24; y++) {
//        for (x = 0; x < 32; x++) {
//            tms9918a_raster_multi(vdp, *p++, pattern + ((y & 3) << 1), fp);
//            fp += 8;
//        }
//        fp += 7 * 256;
//    }
//    tms9918a_raster_sprites(vdp);
//}
//
///*
// *	Rasterise a text symbol
// */
//static void tms9918a_raster_pattern6(struct tms9918a* vdp, uint8_t code, uint8_t* pattern, uint32_t* out)
//{
//    unsigned int x, y;
//    uint8_t bits;
//    uint32_t background = vdp->colourmap[vdp->reg[7] & 0x0F];
//    uint32_t foreground = vdp->colourmap[vdp->reg[7] >> 4];
//
//    pattern += code << 3;
//
//    /* 8 rows, left 6 columns (highest bits) used */
//    for (y = 0; y < 8; y++) {
//        bits = *pattern++;
//        for (x = 0; x < 6; x++) {
//            if (bits & 0x80)
//                *out++ = foreground;
//            else
//                *out++ = background;
//            bits <<= 1;
//        }
//        out += 250;	/* 256 bytes per row even when working in 240 pixel */
//    }
//}
//
///*
// *	960 characters using 6bits of each pattern. No sprites, no colour
// *	tables. 8 pixels of border left and right
// */
//static void tms9918a_rasterize_text(struct tms9918a* vdp)
//{
//    uint8_t* p = vdp->framebuffer + ((vdp->reg[2] & 0x0F) << 10);
//    uint8_t* pattern = vdp->framebuffer + ((vdp->reg[4] & 0x07) << 11);
//    uint32_t* fp = vdp->rasterbuffer;
//    unsigned int x, y;
//    uint32_t background = vdp->colourmap[vdp->reg[7] & 0x0F];
//
//    /* Everything really happens in screen thirds but for this mode it
//       does not actually matter */
//    for (y = 0; y < 24; y++) {
//        /* Weird 6bit wide mode */
//        for (x = 0; x < 8; x++) {
//            fp[256] = background;
//            fp[512] = background;
//            fp[768] = background;
//            fp[1024] = background;
//            fp[1280] = background;
//            fp[1536] = background;
//            fp[1792] = background;
//            *fp++ = background;
//        }
//        for (x = 0; x < 40; x++) {
//            tms9918a_raster_pattern6(vdp, *p++, pattern, fp);
//            fp += 6;
//        }
//        for (x = 0; x < 8; x++) {
//            fp[256] = background;
//            fp[512] = background;
//            fp[768] = background;
//            fp[1024] = background;
//            fp[1280] = background;
//            fp[1536] = background;
//            fp[1792] = background;
//            *fp++ = background;
//        }
//        /* Our rows are 256 pixels but for text we use the middle 240 */
//        fp += 7 * 256;
//    }
//    /* No sprites in text mode */
//}
//
///*
// *	Rasterize the frame buffer for the current settings. Generates a
// *	32bit frame buffer image in 256x192 pixels ready for SDL2 or similar
// *	to scale and render onto the actual framebuffer. Call this every
// *	vblank frame.
// */
//void tms9918a_rasterize(struct tms9918a* vdp)
//{
//    unsigned int mode = (vdp->reg[1] >> 2) & 0x06;
//    mode |= (vdp->reg[0] & 0x02) >> 1;
//
//    if ((vdp->reg[1] & 0x40) == 0)
//        memset(vdp->rasterbuffer, 0, sizeof(vdp->rasterbuffer));
//    else {
//        switch (mode) {
//        case 0:
//            tms9918a_rasterize_g1(vdp);
//            break;
//        case 1:
//            tms9918a_rasterize_g2(vdp);
//            break;
//        case 2:
//            tms9918a_rasterize_mc(vdp);
//            break;
//        case 4:
//            tms9918a_rasterize_text(vdp);
//            break;
//        default:
//            /* There are things that happen for the invalid cases but address
//               them later maybe */
//            memset(vdp->rasterbuffer, 0, sizeof(vdp->rasterbuffer));
//        }
//    }
//    if (vdp->trace)
//        fprintf(stderr, "vdp: frame done.\n");
//    vdp->status |= 0x80;
//}
//
//static uint8_t tms9918a_status(struct tms9918a* vdp)
//{
//    uint8_t r = vdp->status;
//    vdp->status = 0;
//    return r;
//}
//
///*
// *	This is the whole VDP memory interface. It's a thing of beauty
// *	Everything else is rendering time (on the real one even the loading
// *	of the real readback data and slotting in the writes).
// */
//
//void tms9918a_write(struct tms9918a* vdp, uint8_t addr, uint8_t val)
//{
//    switch (addr & 1) {
//    case 0:
//        if (vdp->trace)
//            fprintf(stderr, "vdp: write fb %04x<-%02X\n", vdp->addr, val);
//        vdp->framebuffer[vdp->addr] = val;
//        vdp->addr++;
//        vdp->addr &= vdp->memmask;
//        /* A data write clears the latch, this means you can write the low
//           byte of the address and data repeatedly and usefully */
//        vdp->latch = 0;
//        break;
//    case 1:
//        if (vdp->latch == 0) {
//            /* The set up affects the low address bits immediately. it does
//               not seem to change the direction */
//            vdp->addr &= 0xFF00;
//            vdp->addr |= val;
//            vdp->latch = 1;
//            return;
//        }
//        vdp->latch = 0;
//        switch (val & 0xC0) {
//            /* Memory read set up */
//        case 0x00:
//            vdp->addr &= 0xFF;
//            vdp->addr |= val << 8;
//            vdp->read = 1;
//            vdp->addr &= vdp->memmask;
//            /* Strictly speaking this makes a request, the result turns
//               up in a bit. We might want to model a downcounter to trap
//               errors in this */
//            if (vdp->trace)
//                fprintf(stderr, "vdp: set up to read %04x\n", vdp->addr);
//            break;
//            /* Memory write set up */
//        case 0x40:
//            vdp->addr &= 0xFF;
//            vdp->addr |= val << 8;
//            vdp->addr &= vdp->memmask;
//            vdp->read = 0;
//            if (vdp->trace)
//                fprintf(stderr, "vdp: set up to write %04x\n", vdp->addr);
//            break;
//            /* Write to a register. Not clear if the low part of the address
//               and latched data are one but they seem to be */
//        case 0x80:
//            vdp->reg[val & 7] = vdp->addr & 0xFF;
//            if (vdp->trace)
//                fprintf(stderr, "vdp: write reg %02X <- %02x\n", val, vdp->addr & 0xFF);
//            break;
//            /* Unused on the VDP1 */
//        case 0xC0:
//            break;
//        }
//    }
//}
//
//uint8_t tms9918a_read(struct tms9918a* vdp, uint8_t addr)
//{
//    uint8_t r;
//    switch (addr & 1) {
//    case 0:
//        r = vdp->framebuffer[vdp->addr++ & vdp->memmask];
//        if (vdp->trace)
//            fprintf(stderr, "vdp: read data %02x\n", r);
//        break;
//    case 1:
//        r = tms9918a_status(vdp);
//        /* Nasty if we have data latched it just went poof! */
//        if (vdp->latch && vdp->trace)
//            fprintf(stderr, "vdp: status read cleared latch.\n");
//        if (vdp->trace)
//            fprintf(stderr, "vdp: read status %02x\n", r);
//        break;
//    }
//    vdp->latch = 0;
//    return r;
//}
//
//void tms9918a_reset(struct tms9918a* vdp)
//{
//    vdp->reg[0] = 0;
//    vdp->reg[1] = 0;
//    vdp->latch = 0;
//    vdp->read = 0;
//    vdp->memmask = 0x3FFF;	/* 16K */
//}
//
//void vdp_write(uint16_t reg, uint8_t value) {
//    tms9918a_write(vdp, reg, value);
//}
//
//uint8_t vdp_read(uint16_t reg) {
//    return tms9918a_read(vdp, reg);
//}
//
//bool tms9918a_init(void) {
//    vdp = tms9918a_create();
//    if (vdp == NULL)
//        return false;
//
//    vdp_renderer = tms9918a_renderer_create(vdp);
//    if (vdp_renderer == NULL)
//        return false;
//
//    vdp->trace = 0;
//    set_port_write_redirector(0xC0, 0xDF, &vdp_write);
//    set_port_read_redirector(0xC0, 0xDF, &vdp_read);
//    return true;
//}
//
//void tms9918a_destroy(void) {
//    tms9918a_free(vdp);
//    tms9918a_renderer_free(vdp_renderer);
//    
//}
//
//struct tms9918a* tms9918a_create(void)
//{
//    struct tms9918a* vdp = malloc(sizeof(struct tms9918a));
//    if (vdp == NULL) {
//        fprintf(stderr, "Out of memory.\n");
//        exit(1);
//    }
//    tms9918a_reset(vdp);
//
//    return vdp;
//}
//
//void tms9918a_free(struct tms9918a* vdp) {
//    if (vdp != NULL) {
//        free(vdp);
//    }
//}
//
//void tms9918a_trace(struct tms9918a* vdp, int onoff)
//{
//    vdp->trace = onoff;
//}
//
//int tms9918a_irq_pending(struct tms9918a* vdp)
//{
//    if (vdp->reg[1] & 0x20)
//        return vdp->status & 0x80;
//    return 0;
//}
//
//uint32_t* tms9918a_get_raster(struct tms9918a* vdp)
//{
//    return vdp->rasterbuffer;
//}
//
//void tms9918a_set_colourmap(struct tms9918a* vdp, uint32_t* ctab)
//{
//    vdp->colourmap = ctab;
//}
//
//bool tms9918a_render(struct tms9918a_renderer* render)
//{
//    SDL_Rect sr;
//
//    sr.x = 0;
//    sr.y = 0;
//    sr.w = 256;
//    sr.h = 192;
//    SDL_UpdateTexture(render->texture, NULL, tms9918a_get_raster(render->vdp), 1024);
//    SDL_RenderClear(render->render);
//    SDL_RenderCopy(render->render, render->texture, NULL, &sr);
//    SDL_RenderPresent(render->render);
//
//    SDL_Event event;
//    while (SDL_PollEvent(&event)) {
//        if (event.type == SDL_QUIT) {
//            return false;
//        }
//        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
//            return false;
//        }
//        if (event.type == SDL_KEYDOWN) {
//            bool consumed = false;
//            //if (cmd_down && !(disable_emu_cmd_keys || mouse_grabbed)) {
//            //    if (event.key.keysym.sym == SDLK_s) {
//            //        //machine_dump("user keyboard request");
//            //        consumed = true;
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_r) {
//            //        hard_reset = true;
//            //        reset_requested = true;
//            //        //machine_reset();
//            //        consumed = true;
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_BACKSPACE) {
//            //        //machine_nmi();
//            //        //consumed = true;
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_v) {
//            //        //machine_paste(SDL_GetClipboardText());
//            //        //consumed = true;
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_f || event.key.keysym.sym == SDLK_RETURN) {
//            //        /*is_fullscreen = !is_fullscreen;
//            //        SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
//            //        consumed = true;*/
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_EQUALS) {
//            //        //machine_toggle_warp();
//            //        //consumed = true;
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_a) {
//            //        //sdcard_attach();
//            //        //consumed = true;
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_d) {
//            //        //sdcard_detach();
//            //        //consumed = true;
//            //    }
//            //    else if (event.key.keysym.sym == SDLK_p) {
//            //        //screenshot();
//            //        //consumed = true;
//            //    }
//            //}
//            //if (cmd_down) {
//            //    if (event.key.keysym.sym == SDLK_m) {
//            //        mousegrab_toggle();
//            //        //consumed = true;
//            //    }
//            //}
//            //if (!consumed) {
//            //    /*if (event.key.keysym.scancode == LSHORTCUT_KEY || event.key.keysym.scancode == RSHORTCUT_KEY) {
//            //        cmd_down = true;
//            //    }
//            //    handle_keyboard(true, event.key.keysym.sym, event.key.keysym.scancode);*/
//            //}
//            continue;
//        }
//        if (event.type == SDL_KEYUP) {
//            /*if (event.key.keysym.scancode == LSHORTCUT_KEY || event.key.keysym.scancode == RSHORTCUT_KEY) {
//                cmd_down = false;
//            }
//            handle_keyboard(false, event.key.keysym.sym, event.key.keysym.scancode);*/
//            continue;
//        }
//        if (event.type == SDL_MOUSEBUTTONDOWN) {
//            //switch (event.button.button) {
//            //case SDL_BUTTON_LEFT:
//            //    //mouse_button_down(0);
//            //    mouse_changed = true;
//            //    break;
//            //case SDL_BUTTON_RIGHT:
//            //    //mouse_button_down(1);
//            //    mouse_changed = true;
//            //    break;
//            //case SDL_BUTTON_MIDDLE:
//            //    //mouse_button_down(2);
//            //    mouse_changed = true;
//            //    break;
//            //}
//        }
//        if (event.type == SDL_MOUSEBUTTONUP) {
//            //switch (event.button.button) {
//            //case SDL_BUTTON_LEFT:
//            //    //mouse_button_up(0);
//            //    mouse_changed = true;
//            //    break;
//            //case SDL_BUTTON_RIGHT:
//            //    //mouse_button_up(1);
//            //    mouse_changed = true;
//            //    break;
//            //case SDL_BUTTON_MIDDLE:
//            //    //mouse_button_up(2);
//            //    mouse_changed = true;
//            //    break;
//            //}
//        }
//        if (event.type == SDL_MOUSEMOTION) {
//            static int mouse_x;
//            static int mouse_y;
//            /*if (mouse_grabbed) {
//                mouse_move(event.motion.xrel, event.motion.yrel);
//            }
//            else {
//                mouse_move(event.motion.x - mouse_x, event.motion.y - mouse_y);
//            }*/
//            mouse_x = event.motion.x;
//            mouse_y = event.motion.y;
//            //mouse_changed = true;
//        }
//        if (event.type == SDL_MOUSEWHEEL) {
//            //mouse_set_wheel(event.wheel.y);
//            //mouse_changed = true;
//        }
//
//        /*if (event.type == SDL_JOYDEVICEADDED) {
//            joystick_add(event.jdevice.which);
//        }
//        if (event.type == SDL_JOYDEVICEREMOVED) {
//            joystick_remove(event.jdevice.which);
//        }
//        if (event.type == SDL_CONTROLLERBUTTONDOWN) {
//            joystick_button_down(event.cbutton.which, event.cbutton.button);
//        }
//        if (event.type == SDL_CONTROLLERBUTTONUP) {
//            joystick_button_up(event.cbutton.which, event.cbutton.button);
//        }*/
//
//    }
//
//    return true;
//}
//
//void tms9918a_renderer_free(struct tms9918a_renderer* render)
//{
//    if (render->texture)
//        SDL_DestroyTexture(render->texture);
//    free(render);
//}
//
//
//struct tms9918a_renderer* tms9918a_renderer_create(struct tms9918a* vdp)
//{
//    struct tms9918a_renderer* render;
//
//    /* We will need a nicer way to do this once we have multiple SDL using
//       devices */
//    if (sdl_live == 0) {
//        sdl_live = 1;
//        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
//            fprintf(stderr, "SDL init failed: %s.\n", SDL_GetError());
//            exit(1);
//        }
//        atexit(SDL_Quit);
//    }
//
//    render = malloc(sizeof(struct tms9918a_renderer));
//    if (render == NULL) {
//        fprintf(stderr, "Out of memory.\n");
//        exit(1);
//    }
//    memset(render, 0, sizeof(struct tms9918a_renderer));
//    render->vdp = vdp;
//    tms9918a_set_colourmap(vdp, vdp_ctab);
//    render->window = SDL_CreateWindow("TMS9918A",
//        SDL_WINDOWPOS_UNDEFINED,
//        SDL_WINDOWPOS_UNDEFINED,
//        512, 384,
//        SDL_WINDOW_RESIZABLE);
//    if (render->window == NULL) {
//        fprintf(stderr, "Unable to create window: %s.\n", SDL_GetError());
//        exit(1);
//    }
//    render->render = SDL_CreateRenderer(render->window, -1, SDL_RENDERER_ACCELERATED);
//    if (render->render == NULL) {
//        fprintf(stderr, "Unable to create renderer: %s.\n", SDL_GetError());
//        exit(1);
//    }
//    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
//    SDL_RenderSetLogicalSize(render->render, 256, 192);
//
//    render->texture = SDL_CreateTexture(render->render,
//        SDL_PIXELFORMAT_ARGB8888,
//        //SDL_TEXTUREACCESS_STREAMING
//        256, 192);
//    if (render->render == NULL) {
//        fprintf(stderr, "Unable to create renderer: %s.\n", SDL_GetError());
//        exit(1);
//    }
//    SDL_SetRenderDrawColor(render->render, 0, 0, 0, 255);
//    SDL_RenderClear(render->render);
//    SDL_RenderPresent(render->render);
//    return render;
//}