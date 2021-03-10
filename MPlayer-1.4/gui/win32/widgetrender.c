/*
 * MPlayer GUI for Win32
 * Copyright (C) 2003 Sascha Sommer <saschasommer@freenet.de>
 * Copyright (C) 2006 Erik Augustson <erik_27can@yahoo.com>
 * Copyright (C) 2006 Gianluigi Tiesi <sherpya@netfarm.it>
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>

#include "gui/util/bitmap.h"
#include "gui/util/string.h"
#include "gui/interface.h"
#include "gui.h"

#include "access_mpcontext.h"
#include "help_mp.h"
#include "libavutil/avstring.h"
#include "libavutil/common.h"
#include "stream/stream.h"

#define MAX_LABELSIZE 250

static void render(int bitsperpixel, image *dst, image *src, int x, int y, int sx, int sy, int sw, int sh, int transparent)
{
    int i;
    int bpp = bitsperpixel / 8;
    int offset = (dst->width * bpp * y) + (x * bpp);
    int soffset = (src->width * bpp * sy) + (sx * bpp);

    for(i=0; i<sh; i++)
    {
        int c;
        for(c=0; c < (sw * bpp); c += bpp)
        {
            if(bpp == 2)
            {
                if(!transparent || (((src->data + soffset + (i * src->width * bpp) + c)[0] != 0x1f)
                    && ((src->data + soffset + (i * src->width * bpp) + c)[1] != 0x7c)))
                    memcpy(dst->data + offset + c, src->data + soffset + (i * src->width * bpp) + c, bpp);
            }
            else if(bpp > 2)
            {
                if(!transparent || !IS_TRANSPARENT(*((unsigned int *) (src->data + soffset + (i * src->width * bpp) + c))))
                    memcpy(dst->data + offset + c, src->data + soffset + (i * src->width * bpp) + c, bpp);
            }
        }
        offset += (dst->width * bpp);
    }
}

static image *find_background(skin_t *skin, widget *item)
{
    unsigned int i;
    for (i=0; i < skin->windowcount; i++)
        if(skin->windows[i]->type == item->window)
            return skin->windows[i]->base->bitmap[0];
    return NULL;
}

/******************************************************************/
/*                      FONT related functions                    */
/******************************************************************/

/* returns the pos of s2 inside s1 or -1 if  s1 doesn't contain s2 */
static int strpos(char *s1, const char* s2)
{
    unsigned int i, x;
    for (i=0; i < strlen(s1); i++)
    {
        if(s1[i] == s2[0])
        {
            if(strlen(s1 + i) >= strlen(s2))
            {
                for (x=0; x <strlen(s2); x++)
                    if(s1[i + x] != s2[x]) break;
                if(x == strlen(s2)) return i;
            }
        }
    }
    return -1;
}

/* replaces all occurences of what in dest with format */
static void stringreplace(char *dest, const char *what, const char *format, ... )
{
    char tmp[MAX_LABELSIZE];
    int offset=0;
    va_list va;
    va_start(va, format);
    vsnprintf(tmp, MAX_LABELSIZE, format, va);
    va_end(va);
    /* no search string == replace the entire string */
    if(!what)
    {
        memcpy(dest, tmp, strlen(tmp));
        dest[strlen(tmp)] = 0;
        return;
    }
    while((offset = strpos(dest, what)) != -1)
    {
        memmove(dest + offset + strlen(tmp), dest + offset + strlen(what), strlen(dest + offset + strlen(what)) + 1);
        memcpy(dest + offset, tmp, strlen(tmp));
    }
}

/**
 * @brief Convert #guiInfo member @ref guiInterface_t.Filename "Filename".
 *
 * @param how 0 (cut file path and extension),
 *            1 (additionally, convert lower case) or
 *            2 (additionally, convert upper case) or
 *            4 (unaltered title if available, otherwise like 0)
 * @param fname memory location of a buffer to receive the converted Filename
 * @param maxlen size of the @a fname buffer
 *
 * @return pointer to the @a fname buffer
 */
static char *TranslateFilename (int how, char *fname, size_t maxlen)
{
    char *p;
    size_t len;
    stream_t *stream;

    switch (guiInfo.StreamType)
    {
        case STREAMTYPE_FILE:

            if ((how == 4) && guiInfo.Title)
                av_strlcpy(fname, guiInfo.Title, maxlen);
            else if (guiInfo.Filename && *guiInfo.Filename)
            {
                p = strrchr(guiInfo.Filename, '\\');

                if (p) av_strlcpy(fname, p + 1, maxlen);
                else av_strlcpy(fname, guiInfo.Filename, maxlen);

                len = strlen(fname);

                if (len > 3 && fname[len - 3] == '.') fname[len - 3] = 0;
                else if (len > 4 && fname[len - 4] == '.') fname[len - 4] = 0;
                else if (len > 5 && fname[len - 5] == '.') fname[len - 5] = 0;
            }
            else av_strlcpy(fname, MSGTR_GUI_MSG_NoFileLoaded, maxlen);

            break;

        case STREAMTYPE_STREAM:

            av_strlcpy(fname, guiInfo.Filename, maxlen);
            break;

        case STREAMTYPE_CDDA:

            snprintf(fname, maxlen, MSGTR_GUI_TitleN, guiInfo.Track);
            break;

        case STREAMTYPE_VCD:

            snprintf(fname, maxlen, MSGTR_GUI_TitleN, guiInfo.Track - 1);
            break;

        case STREAMTYPE_DVD:

            if (guiInfo.Chapter) snprintf(fname, maxlen, MSGTR_GUI_ChapterN, guiInfo.Chapter);
            else av_strlcpy(fname, MSGTR_GUI_NoChapter, maxlen);

            break;

        case STREAMTYPE_TV:
        case STREAMTYPE_DVB:

            p = MSGTR_GUI_NoChannelName;
            stream = mpctx_get_stream(guiInfo.mpcontext);

            if (stream) stream_control(stream, STREAM_CTRL_GET_CURRENT_CHANNEL, &p);

            av_strlcpy(fname, p, maxlen);
            break;

        default:

            av_strlcpy(fname, MSGTR_GUI_MSG_NoMediaOpened, maxlen);
            break;
    }

    if (how == 1) strlower(fname);
    if (how == 2) strupper(fname);

    return fname;
}

/* replaces the chars with special meaning with the associated data from the player info struct */
static char *generatetextfromlabel(widget *item)
{
    char *text = malloc(MAX_LABELSIZE);
    char tmp[MAX_LABELSIZE];
    if(!item)
    {
        free(text);
        return NULL;
    }
    strcpy(text, item->label);
    if(item->type == tySlabel) return text;
    stringreplace(text, "$1", "%.2i:%.2i:%.2i", guiInfo.ElapsedTime / 3600,
                 (guiInfo.ElapsedTime / 60) % 60, guiInfo.ElapsedTime % 60);
    stringreplace(text, "$2", "%.4i:%.2i", guiInfo.ElapsedTime / 60, guiInfo.ElapsedTime % 60);
    stringreplace(text, "$3", "%.2i", guiInfo.ElapsedTime / 3600);
    stringreplace(text, "$4", "%.2i", (guiInfo.ElapsedTime / 60) % 60);
    stringreplace(text, "$5", "%.2i", guiInfo.ElapsedTime % 60);
    stringreplace(text, "$6", "%.2i:%.2i:%.2i", guiInfo.RunningTime / 3600,
                 (guiInfo.RunningTime / 60) % 60, guiInfo.RunningTime % 60);
    stringreplace(text, "$7", "%.4i:%.2i", guiInfo.RunningTime / 60, guiInfo.RunningTime % 60);
    stringreplace(text, "$8", "%i:%.2i:%.2i", guiInfo.ElapsedTime / 3600,
                 (guiInfo.ElapsedTime / 60) % 60, guiInfo.ElapsedTime % 60);
    stringreplace(text, "$v", "%3.2f%%", guiInfo.Volume);
    stringreplace(text, "$V", "%3.1f", guiInfo.Volume);
    stringreplace(text, "$U", "%3.0f", guiInfo.Volume);
    stringreplace(text, "$b", "%3.2f%%", guiInfo.Balance);
    stringreplace(text, "$B", "%3.1f", guiInfo.Balance);
    stringreplace(text, "$D", "%3.0f", guiInfo.Balance);
    stringreplace(text, "$t", "%.2i", guiInfo.Track);
    stringreplace(text, "$o", "%s", acp(TranslateFilename(0, tmp, sizeof(tmp))));
    stringreplace(text, "$O", "%s", acp(TranslateFilename(4, tmp, sizeof(tmp))));
    stringreplace(text, "$x", "%i", guiInfo.VideoWidth);
    stringreplace(text, "$y", "%i", guiInfo.VideoHeight);
    stringreplace(text, "$C", "%s", guiInfo.sh_video ? codecname : "");
    stringreplace(text, "$$", "$");

    if(guiInfo.Playing == GUI_STOP)
    {
        stringreplace(text, "$P", "s");
        stringreplace(text, "$s", "s");
    }
    else if(guiInfo.Playing == GUI_PLAY)
    {
        stringreplace(text, "$P", "p");
        stringreplace(text, "$p", "p");
    }
    else if(guiInfo.Playing == GUI_PAUSE)
    {
        stringreplace(text, "$P", "e");
        stringreplace(text, "$e", "e");
    }

    if(guiInfo.AudioChannels == 0) stringreplace(text, "$a", "n");
    else if(guiInfo.AudioChannels == 1) stringreplace(text, "$a", "m");
    else if(guiInfo.AudioChannels == 2) stringreplace(text, "$a", (guiInfo.AudioPassthrough ? "r" : "t"));
    else stringreplace(text, "$a", "r");

    if(guiInfo.StreamType == STREAMTYPE_FILE)
        stringreplace(text, "$T", "f");
    else if(guiInfo.StreamType == STREAMTYPE_DVD || guiInfo.StreamType == STREAMTYPE_DVDNAV)
        stringreplace(text, "$T", "d");
    else if(guiInfo.StreamType == STREAMTYPE_STREAM)
        stringreplace(text, "$T", "u");
    else stringreplace(text, "$T", " ");

    stringreplace(text, "$f", acp(TranslateFilename(1, tmp, sizeof(tmp))));
    stringreplace(text, "$F", acp(TranslateFilename(2, tmp, sizeof(tmp))));

    stringreplace(text, "$g", "");

    return text;
}

/* cuts text to buflen scrolling from right to left */
static void scrolltext(char *text, unsigned int buflen, float *value)
{
    char *buffer = malloc(buflen + 1);
    unsigned int x,i;
    if(*value < buflen) x = 0;
    else x = *value - buflen;
    memset(buffer, ' ', buflen);
    for (i = (*value>=buflen) ? 0 : buflen - *value; i<buflen; i++)
    {
        if(x < strlen(text))
            buffer[i] = text[x];
        x++;
    }
    buffer[buflen] = 0;
    *value += 1.0f;
    if(*value >= strlen(text) + buflen) *value = 0.0f;
    strcpy(text, buffer);
    free(buffer);
}

/* updates all dlabels and slabels */
void renderinfobox(skin_t *skin, window_priv_t *priv)
{
    unsigned int i;
    if (!priv) return;

    /* repaint the area behind the text*/
    /* we have to do this for all labels here, because they may overlap in buggy skins ;( */

    for (i=0; i<skin->widgetcount; i++)
        if((skin->widgets[i]->type == tyDlabel) || (skin->widgets[i]->type == tySlabel))
        {
            char *text = generatetextfromlabel(skin->widgets[i]);

            if(*text && (skin->widgets[i]->window == priv->type))
                render(skin->desktopbpp,
                       &priv->img,
                       find_background(skin, skin->widgets[i]),
                       skin->widgets[i]->x,
                       skin->widgets[i]->y,
                       skin->widgets[i]->x,
                       skin->widgets[i]->y,
                       skin->widgets[i]->length,
                       skin->widgets[i]->font->chars[0]->height,
                       1);

            free(text);
        }

    /* load all slabels and dlabels */
    for (i=0; i<skin->widgetcount; i++)
    {
        widget *item = skin->widgets[i];
        if(item->window != priv->type) continue;
        if((i == skin->widgetcount) || (item->type == tyDlabel) || (item->type == tySlabel))
        {
            char *text = generatetextfromlabel(item);
            unsigned int current, c;
            int offset = 0;
            unsigned int textlen;
            if(!text) continue;
            textlen = strlen(text);

            /* render(win, win->background, gui->skin->widgets[i]->x, gui->skin->widgets[i]->y,
                      gui->skin->widgets[i]->x, gui->skin->widgets[i]->y,
                      gui->skin->widgets[i]->length, gui->skin->widgets[i]->font->chars[0]->height,1); */

            /* calculate text size */
            for (current=0; current<textlen; current++)
            {
                for (c=0; c<item->font->charcount; c++)
                    if(item->font->chars[c]->c == text[current])
                    {
                        offset += item->font->chars[c]->width;
                        break;
                    }
            }

            /* labels can be scrolled if they are to big */
            if((item->type == tyDlabel) && (item->length < offset))
            {
                int tomuch = (offset - item->length) / (offset /textlen);
                scrolltext(text, textlen - tomuch - 1, &skin->widgets[i]->value);
                textlen = strlen(text);
            }

            /* align the text */
            if(item->align == 1)
                offset = (item->length-offset) / 2;
            else if(item->align == 2)
                offset = item->length-offset;
            else
                offset = 0;

            if(offset < 0) offset = 0;

            /* render the text */
            for (current=0; current<textlen; current++)
            {
                for (c=0; c<item->font->charcount; c++)
                {
                    char_t *cchar = item->font->chars[c];
                    if(cchar->c == *(text + current))
                    {
                        render(skin->desktopbpp,
                               &priv->img,
                               item->font->image,
                               item->x + offset,
                               item->y,
                               cchar->x,
                               cchar->y,
                               (cchar->width + offset > item->length) ? item->length - offset : cchar->width,
                               cchar->height,
                               1);
                        offset += cchar->width;
                    break;
                    }
                }
            }
            free(text);
        }
    }
}

/******************************************************************/
/*                   WIDGET related functions                     */
/******************************************************************/

void renderwidget(skin_t *skin, image *dest, widget *item, int state)
{
    image *img = NULL;
    int height;
    int y;

    if(!dest) return;
    if((item->type == tyButton) || (item->type == tyHpotmeter) || (item->type == tyVpotmeter) || (item->type == tyPimage))
        img = item->bitmap[0];
    if(item->type == tyRpotmeter)
        img = item->bitmap[1];

    if(!img) return;

    y = item->y;
    if(item->type == tyPimage || /* legacy (potmeter) */ (item->type == tyHpotmeter && item->width == item->wwidth) || item->type == tyRpotmeter)
    {
        height = img->height / item->phases;
        y =  height * (int)(item->value * item->phases / 100);
        if(y > img->height-height)
            y = img->height - height;
    }
    else
    {
        height = img->height / 3;
        y = state * height;
    }

    /* redraw background */
    if(item->type == tyButton)
        render(skin->desktopbpp, dest, find_background(skin,item), item->x, item->y, item->x, item->y, img->width, height, 1);

    if((item->type == tyHpotmeter) || (item->type == tyVpotmeter) || (item->type == tyRpotmeter) || (item->type == tyPimage))
    {
        if(item->type == tyVpotmeter)
        {
            /* repaint the area behind the slider */
            render(skin->desktopbpp, dest, find_background(skin, item), item->wx, item->wy, item->wx, item->wy, item->width, item->wheight, 1);
            item->y = (100 - item->value) * (item->wheight-item->height) / 100 + item->wy;
        }
        else if(item->type == tyRpotmeter)
        {
            /* repaint the area behind the rpotmeter */
            render(skin->desktopbpp, dest, find_background(skin, item), item->wx, item->wy, item->wx, item->wy, item->wwidth, item->wheight, 1);
            item->x = item->wx;
            item->y = item->wy;
        }
        else
        {
            /* repaint the area behind the slider */
            render(skin->desktopbpp, dest, find_background(skin, item), item->wx, item->wy, item->wx, item->wy, item->wwidth, item->height, 1);
            item->x = item->value * (item->wwidth-item->width) / 100 + item->wx;
        }
    }
    render(skin->desktopbpp, dest, img, item->x, item->y, 0, y, img->width, height, 1);

    /* rpotmeter button */
    if(item->type == tyRpotmeter && item->bitmap[0] != item->bitmap[1])
    {
        img = item->bitmap[0];

        if(img)
        {
            double radius, radian;
            int ix, iy;

            // keep the button inside the potmeter outline
            radius = (FFMIN(item->wwidth, item->wheight) - item->maxwh) / 2.0;

            radian = item->value / 100.0 * item->arclength + item->zeropoint;

            // coordinates plus a correction for a non-square item
            // (remember: both axes are mirrored, we have a clockwise radian)
            ix = item->wx + radius * (1 + cos(radian)) + FFMAX(0, (item->wwidth - item->wheight) / 2.0) + 0.5;
            iy = item->wy + radius * (1 + sin(radian)) + FFMAX(0, (item->wheight - item->wwidth) / 2.0) + 0.5;

            height = img->height / 3;
            y = state * height;

            render(skin->desktopbpp, dest, img, ix, iy, 0, y, img->width, height, 1);
        }
    }
}
