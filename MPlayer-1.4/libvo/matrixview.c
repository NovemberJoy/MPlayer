/*
 * Copyright (C) 2003 Alex Zolotov <nightradio@knoppix.ru>
 * Mucked with by Tugrul Galatali <tugrul@galatali.com>
 *
 * MatrixView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MatrixView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MatrixView; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * Ported to an MPlayer video out plugin by Pigeon <pigeon at pigeond.net>
 * August 2006
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "gl_common.h"
#include "matrixview.h"
#include "matrixview_font.h"

static float matrix_contrast   = 1.5;
static float matrix_brightness = 1.0;

#define MAX_TEXT_X 0x4000
#define MAX_TEXT_Y 0x4000
static int text_x = 0;
static int text_y = 0;
#define _text_x (text_x/2)
#define _text_y (text_y/2)

// Scene position
#define Z_Off -128.0f
#define Z_Depth 8

static uint8_t *speed;
static uint8_t *text;
static uint8_t *text_light;

static float *bump_pic;

static void draw_flare(float x, float y, float z);

static void draw_char(int num, int light, int illuminated, float x, float y, float z)
{
    int light2 = 0;
    float tx, ty;

    num %= 55;
    if (light < 10) light = 0;
    //light = light / 255;        //light=7-light;num+=(light*60);
    light *= matrix_brightness;
    if (illuminated) {
        draw_flare(x, y, z);
        light += 128;
        if (light > 255) light = 255;
        light2 = 128;
    }
    ty = (float)(num / 10) / 6;
    tx = (float)(num % 10) / 10;
    mpglColor4ub(light2, light, light2, 255);        // Basic polygon color

    mpglTexCoord2f(tx, ty);
    mpglVertex3f(x, y, z);
    mpglTexCoord2f(tx + 0.1, ty);
    mpglVertex3f(x + 1, y, z);
    mpglTexCoord2f(tx + 0.1, ty + 0.166);
    mpglVertex3f(x + 1, y - 1, z);
    mpglTexCoord2f(tx, ty + 0.166);
    mpglVertex3f(x, y - 1, z);
}

static void draw_flare(float x, float y, float z)        //flare
{
    mpglColor4ub(204, 204, 204, 255);        // Basic polygon color

    mpglTexCoord2f(1.0 - 4.0/128, 1.0 - 4.0/64);
    mpglVertex3f(x - 1, y + 1, z);
    mpglTexCoord2f(1.0 - 1.0/128, 1.0 - 4.0/64);
    mpglVertex3f(x + 2, y + 1, z);
    mpglTexCoord2f(1.0 - 1.0/128, 1.0 - 1.0/64);
    mpglVertex3f(x + 2, y - 2, z);
    mpglTexCoord2f(1.0 - 4.0/128, 1.0 - 1.0/64);
    mpglVertex3f(x - 1, y - 2, z);
}

static void draw_text(const uint8_t *pic)
{
    int x, y;
    int p = 0;
    int c, c_pic;
    int pic_fade = 255;
    int illuminated;

    for (y = _text_y; y > -_text_y; y--) {
        for (x = -_text_x; x < _text_x; x++) {
            c  = text_light[p] - (text[p] >> 1);
            c += pic_fade;
            if (c > 255)
                c = 255;

            if (pic) {
                // Original code
                //c_pic = pic[p] * matrix_contrast - (255 - pic_fade);

                c_pic = (255 - pic[p]) * matrix_contrast - (255 - pic_fade);

                if (c_pic < 0)
                    c_pic = 0;

                c -= c_pic;

                if (c < 0)
                    c = 0;

                bump_pic[p] = (255.0f - c_pic) / (256 / Z_Depth);
            } else {
                bump_pic[p] = Z_Depth;
            }

            illuminated = text_light[p] > 128 && text_light[p + text_x] < 10;
            draw_char(text[p], c, illuminated, x, y, bump_pic[p]);

            p++;
        }
    }
}

static void scroll(double dCurrentTime)
{
    int a, s, polovina;
    //static double dLastCycle = -1;
    static double dLastMove = -1;

    if (dCurrentTime - dLastMove > 1.0 / (text_y / 1.5)) {
        dLastMove = dCurrentTime;

        polovina = text_x * text_y / 2;
        s = 0;
        for (a = text_x * text_y + text_x - 1; a >= text_x; a--) {
            if (speed[s])
                text_light[a] = text_light[a - text_x];        //scroll light table down
            s++;
            if (s >= text_x)
                s = 0;
        }
        memmove(text_light + text_x, text_light, text_x * text_y);
        memset(text_light, 253, text_x);

        s = 0;
        for (a = polovina; a < text_x * text_y; a++) {
            if (text_light[a] == 255)
                text_light[s] = text_light[s + text_x] >> 1;        //make black bugs in top line

            s++;

            if (s >= text_x)
                s = 0;
        }
    }
}

static void make_change(double dCurrentTime)
{
    int r = rand() % text_x * text_y;

    text[r] += 133;        //random bugs

    r = rand() % (4 * text_x);
    if (r < text_x && text_light[r])
        text_light[r] = 255;        //white bugs

    scroll (dCurrentTime);
}


static void make_text(void)
{
    int a;

    for (a = 0; a < text_x * text_y; a++)
        text[a] = rand() >> 8; // avoid the lowest bits of rand()

    for (a = 0; a < text_x; a++)
        speed[a] = rand() >= RAND_MAX / 2;
}

static void ourBuildTextures(void)
{
    mpglTexImage2D(GL_TEXTURE_2D, 0, 1, 128, 64, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                   font_texture);
    mpglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    mpglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void matrixview_init(int w, int h)
{
    make_text();

    ourBuildTextures();

    // Color to clear color buffer to.
    mpglClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Allow adjusting of texture color via glColor
    mpglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    mpglEnable(GL_BLEND);
    mpglEnable(GL_TEXTURE_2D);

    mpglBlendFunc(GL_ONE, GL_ONE);

    matrixview_reshape(w, h);
}


void matrixview_reshape(int w, int h)
{
    double nearplane = -Z_Off - Z_Depth;
    // perspective projection, also adjusting vertex position
    // by Z_Off and with simplified Z equation since the absolute
    // Z value does not matter, only relative to other pixels
    float matrix[16] = {
      nearplane / _text_x, 0, 0, 0,
      0, nearplane / _text_y, 0, 0,
      0, 0,  1, -1,
      0, 0,  0, -Z_Off
    };
    mpglViewport(0, 0, w, h);

    mpglLoadMatrixf(matrix);
}


void matrixview_draw(double currentTime, const uint8_t *data)
{
    // Clear the color and depth buffers.
    mpglClear(GL_COLOR_BUFFER_BIT);

    // OK, let's start drawing our planer quads.
    mpglBegin(GL_QUADS);
    draw_text(data);
    mpglEnd();

    make_change(currentTime);
}

void matrixview_contrast_set(float contrast)
{
    matrix_contrast = contrast;
}

void matrixview_brightness_set(float brightness)
{
    matrix_brightness = brightness;
}


void matrixview_matrix_resize(int w, int h)
{
    int elems;
    free(speed);
    speed = NULL;
    free(text);
    text = NULL;
    free(text_light);
    text_light = NULL;
    if (w > MAX_TEXT_X || h > MAX_TEXT_Y)
        return;
    elems = w * (h + 1);
    speed      = calloc(w,     sizeof(*speed));
    text       = calloc(elems, sizeof(*text));
    text_light = calloc(elems, sizeof(*text_light));
    bump_pic   = calloc(elems, sizeof(*bump_pic));
    text_x = w;
    text_y = h;
    make_text();
}
