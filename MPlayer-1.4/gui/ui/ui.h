/*
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
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPLAYER_GUI_UI_H
#define MPLAYER_GUI_UI_H

/// End stops of a rotary potentiometer (::itRPotmeter)
enum
{
  NOT_STOPPED,
  STOPPED_AT_0,
  STOPPED_AT_100
};

extern unsigned char * menuDrawBuffer;
extern int             mainVisible;

extern int             uiMainRender;
extern int             uiPlaybarFade;

extern int             sx, sy;

void uiMainInit( void );
void uiMainDone( void );

void uiVideoInit( void );
void uiVideoDone( void );

void uiMenuInit( void );
void uiMenuDone( void );
void uiMenuHide( int mx, int my, int w );
void uiMenuShow( int mx, int my );

void uiPlaybarInit( void );
void uiPlaybarDone( void );
void uiPlaybarShow( int y );

#endif /* MPLAYER_GUI_UI_H */
