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

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libvo/video_out.h"
#include "libao2/audio_out.h"
#include "osdep/priority.h"
#include "mixer.h"
#include "gui/util/list.h"
#include "gui/util/string.h"
#include "gui/ui/ui.h"
#include "gui/interface.h"
#include "gui.h"
#include "mp_msg.h"
#include "help_mp.h"
#include "dialogs.h"


/**
 * @brief Translate between the priority text presented to the user
 *        and the internal priority name.
 *
 * @param prio priority text or name to be translated
 * @param idx 0 (translate internal name to localized user text) or
 *            1 (translate localized user text to internal name)
 *
 * @return translation according to @a idx
 */
static const char *get_priority (const char *prio, int idx)
{
  static const struct
  {
    const char *localization;
    const char *name;
  } priority[] = {{MSGTR_GUI_WIN32_PriorityHigh, "high"},
                  {MSGTR_GUI_WIN32_PriorityAboveNormal, "abovenormal"},
                  {MSGTR_GUI_WIN32_PriorityNormal, "normal"},
                  {MSGTR_GUI_WIN32_PriorityBelowNormal, "belownormal"},
                  {MSGTR_GUI_WIN32_PriorityLow, "idle"}};
  unsigned int i;

  for (i = 0; i < sizeof(priority) / sizeof(*priority); i++)
  {
    const char *l = acp(priority[i].localization);

    if (strcmp(idx == 0 ? priority[i].name : l, prio) == 0)
      return (idx == 0 ? l : priority[i].name);
  }

  return NULL;
}

static LRESULT CALLBACK PrefsWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND btn, label, edit1, edit2, updown1, updown2, track1, track2;
    static HWND vo_driver, ao_driver, prio;
    int i = 0, j = 0;
    float stereopos = gtkAOExtraStereoMul * 10.0;
    float delaypos = audio_delay * 10.0;

    switch (iMsg)
    {
        case WM_CREATE:
        {
            /* video and audio drivers */
            label = CreateWindow("static", acp(MSGTR_GUI_Video),
                                 WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                 10, 14, 60, 15, hwnd,
                                 NULL, ((LPCREATESTRUCT) lParam) -> hInstance,
                                 NULL);
            SendMessage(label, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            label = CreateWindow("static", acp(MSGTR_GUI_Audio),
                                 WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                 205, 14, 60, 15, hwnd,
                                 NULL, ((LPCREATESTRUCT) lParam) -> hInstance,
                                 NULL);
            SendMessage(label, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            label = CreateWindow("static", acp(MSGTR_GUI_Coefficient":"),
                                 WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                 10, 148, 140, 15, hwnd,
                                 NULL, ((LPCREATESTRUCT) lParam) -> hInstance,
                                 NULL);
            SendMessage(label, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            label = CreateWindow("static", acp(MSGTR_GUI_AudioDelay":"),
                                 WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                 10, 187, 140, 15, hwnd,
                                 NULL, ((LPCREATESTRUCT) lParam) -> hInstance,
                                 NULL);
            SendMessage(label, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            label = CreateWindow("static", acp(MSGTR_GUI_OsdLevel),
                                 WS_CHILD | WS_VISIBLE,
                                 10, 286, 115, 15, hwnd,
                                 NULL, ((LPCREATESTRUCT) lParam) -> hInstance,
                                 NULL);
            SendMessage(label, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            label = CreateWindow("static", acp(MSGTR_GUI_WIN32_Priority),
                                 WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                 200, 286, 100, 15, hwnd,
                                 NULL, ((LPCREATESTRUCT) lParam) -> hInstance,
                                 NULL);
            SendMessage(label, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            vo_driver = CreateWindow("combobox", NULL,
                                     CBS_DROPDOWNLIST | CB_SHOWDROPDOWN |
                                     CBS_NOINTEGRALHEIGHT | CBS_HASSTRINGS |
                                     WS_CHILD | WS_VISIBLE |
                                     WS_VSCROLL | WS_TABSTOP,
                                     80, 10, 100, 160, hwnd,
                                     (HMENU) ID_VO_DRIVER,
                                     ((LPCREATESTRUCT) lParam) -> hInstance,
                                     NULL);

            ao_driver = CreateWindow("combobox", NULL,
                                     CBS_DROPDOWNLIST | CB_SHOWDROPDOWN |
                                     CBS_NOINTEGRALHEIGHT | CBS_HASSTRINGS |
                                     WS_CHILD | WS_VISIBLE |
                                     WS_VSCROLL | WS_TABSTOP,
                                     275, 10, 100, 160, hwnd,
                                     (HMENU) ID_AO_DRIVER,
                                     ((LPCREATESTRUCT) lParam) -> hInstance,
                                     NULL);

            prio = CreateWindow("combobox", NULL,
                                CBS_DROPDOWNLIST | CB_SHOWDROPDOWN |
                                CBS_NOINTEGRALHEIGHT | CBS_HASSTRINGS |
                                WS_CHILD | WS_VISIBLE |
                                WS_VSCROLL | WS_TABSTOP,
                                310, 282, 100, 160, hwnd,
                                (HMENU) ID_PRIO,
                                ((LPCREATESTRUCT) lParam) -> hInstance,
                                NULL);

            /* checkboxes */
            btn = CreateWindow("button", acp(MSGTR_GUI_EnableDoubleBuffering),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               10, 35, 205, 25,
                               hwnd, (HMENU) ID_DOUBLE,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_EnableDirectRendering),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               10, 57, 205, 25,
                               hwnd, (HMENU) ID_DIRECT,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_EnableFrameDropping),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               10, 79, 205, 25,
                               hwnd, (HMENU) ID_FRAMEDROP,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_QuitAfterPlaying),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               10, 101, 225, 25,
                               hwnd, (HMENU) ID_IDLE,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_NormalizeSound),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               220, 35, 190, 25,
                               hwnd, (HMENU) ID_NORMALIZE,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_EnableSoftwareMixer),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               220, 57, 190, 25,
                               hwnd, (HMENU) ID_SOFTMIX,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_EnableExtraStereo),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               220, 79, 190, 25,
                               hwnd, (HMENU) ID_EXTRASTEREO,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_EnableCache),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               10, 222, 100, 25,
                               hwnd, (HMENU) ID_CACHE,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_EnableAutomaticAVSync),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               225, 222, 185, 25, hwnd,
                               (HMENU) ID_AUTOSYNC,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_WIN32_DisplayInVideoWindow),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               10, 249, 300, 25,
                               hwnd, (HMENU) ID_VIDEOWINDOW,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            /* osd level */
            btn = CreateWindow("button", acp(MSGTR_GUI_OsdLevel0),
                               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                               15, 297, 200, 25, hwnd,
                               (HMENU) ID_NONE,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_OsdLevel1),
                               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                               15, 317, 395, 25, hwnd,
                               (HMENU) ID_OSD1,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_OsdLevel2),
                               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                               15, 337, 395, 25, hwnd,
                               (HMENU) ID_OSD2,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_OsdLevel3),
                               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                               15, 357, 395, 25, hwnd,
                               (HMENU) ID_OSD3,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_Ok),
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               248, 417, 80, 25, hwnd,
                               (HMENU) ID_APPLY,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_Cancel),
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               334, 417, 80, 25, hwnd,
                               (HMENU) ID_CANCEL,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            btn = CreateWindow("button", acp(MSGTR_GUI_WIN32_Defaults),
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               5, 417, 80, 25, hwnd,
                               (HMENU) ID_DEFAULTS,
                               ((LPCREATESTRUCT) lParam) -> hInstance,
                               NULL);
            SendMessage(btn, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            /* extra stereo coefficient trackbar */
            track1 = CreateWindow(TRACKBAR_CLASS, "Coefficient",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                                  WS_DISABLED | TBS_HORZ |
                                  TBS_BOTTOM | TBS_NOTICKS,
                                  165, 142, 245, 35, hwnd,
                                  (HMENU) ID_TRACKBAR1,
                                  ((LPCREATESTRUCT) lParam) -> hInstance,
                                  NULL);
            SendDlgItemMessage(hwnd, ID_TRACKBAR1, TBM_SETRANGE, 1, MAKELONG(-100, 100));

            /* audio delay */
            track2 = CreateWindow(TRACKBAR_CLASS, "Audio delay",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                                  WS_DISABLED | TBS_HORZ |
                                  TBS_BOTTOM | TBS_NOTICKS,
                                  165, 182, 245, 35, hwnd,
                                  (HMENU) ID_TRACKBAR2,
                                  ((LPCREATESTRUCT) lParam) -> hInstance,
                                  NULL);
            SendDlgItemMessage(hwnd, ID_TRACKBAR2, TBM_SETRANGE, 1, MAKELONG(-1000, 1000));
            SendDlgItemMessage(hwnd, ID_TRACKBAR2, TBM_SETLINESIZE, 0, (LPARAM) 1);

            /* cache */
            edit1 = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "cache",
                                   WS_CHILD | WS_VISIBLE | WS_DISABLED |
                                   ES_LEFT | ES_AUTOHSCROLL,
                                   115, 225, 60, 20, hwnd,
                                   (HMENU) ID_EDIT1,
                                   ((LPCREATESTRUCT) lParam) -> hInstance,
                                   NULL);
            SendMessage(edit1, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            updown1 = CreateUpDownControl(WS_CHILD | WS_VISIBLE |
                                          WS_DISABLED | UDS_SETBUDDYINT |
                                          UDS_ARROWKEYS | UDS_NOTHOUSANDS,
                                          165, 225, 20, 20, hwnd,
                                          ID_UPDOWN1,
                                          ((LPCREATESTRUCT) lParam) -> hInstance,
                                          (HWND)edit1, 0, 0, 0);
            SendDlgItemMessage(hwnd, ID_UPDOWN1, UDM_SETRANGE32, (WPARAM)32, (LPARAM)0x7fffffff);

            /* autosync */
            edit2 = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "autosync",
                                   WS_CHILD | WS_VISIBLE | WS_DISABLED |
                                   ES_LEFT | ES_AUTOHSCROLL,
                                   355, 247, 40, 20, hwnd,
                                   (HMENU) ID_EDIT2,
                                   ((LPCREATESTRUCT) lParam) -> hInstance,
                                   NULL);
            SendMessage(edit2, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            updown2 = CreateUpDownControl(WS_CHILD | WS_VISIBLE |
                                          WS_DISABLED | UDS_SETBUDDYINT |
                                          UDS_ARROWKEYS | UDS_NOTHOUSANDS,
                                          395, 247, 20, 20, hwnd,
                                          ID_UPDOWN2,
                                          ((LPCREATESTRUCT) lParam) -> hInstance,
                                          (HWND)edit2, 0, 0, 0);
            SendDlgItemMessage(hwnd, ID_UPDOWN2, UDM_SETRANGE32, (WPARAM)0, (LPARAM)10000);

            while(video_out_drivers[i])
            {
                const vo_info_t *info = video_out_drivers[i++]->info;
                if(!video_driver_list)
                    listSet(&video_driver_list, info->short_name);
                SendDlgItemMessage(hwnd, ID_VO_DRIVER, CB_ADDSTRING, 0, (LPARAM) info->short_name);
            }
            /* Special case for directx:noaccel */
            SendDlgItemMessage(hwnd, ID_VO_DRIVER, CB_ADDSTRING, 0, (LPARAM) "directx:noaccel");
            SendMessage(vo_driver, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            while(audio_out_drivers[j])
            {
                const ao_info_t *info = audio_out_drivers[j++]->info;
                if(!audio_driver_list)
                    listSet(&audio_driver_list, info->short_name);
                SendDlgItemMessage(hwnd, ID_AO_DRIVER, CB_ADDSTRING, 0, (LPARAM) info->short_name);
            }
            SendMessage(ao_driver, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            /* priority list, i'm leaving out realtime for safety's sake */
            SendDlgItemMessage(hwnd, ID_PRIO, CB_INSERTSTRING, 0, (LPARAM) acp(MSGTR_GUI_WIN32_PriorityLow));
            SendDlgItemMessage(hwnd, ID_PRIO, CB_INSERTSTRING, 0, (LPARAM) acp(MSGTR_GUI_WIN32_PriorityBelowNormal));
            SendDlgItemMessage(hwnd, ID_PRIO, CB_INSERTSTRING, 0, (LPARAM) acp(MSGTR_GUI_WIN32_PriorityNormal));
            SendDlgItemMessage(hwnd, ID_PRIO, CB_INSERTSTRING, 0, (LPARAM) acp(MSGTR_GUI_WIN32_PriorityAboveNormal));
            SendDlgItemMessage(hwnd, ID_PRIO, CB_INSERTSTRING, 0, (LPARAM) acp(MSGTR_GUI_WIN32_PriorityHigh));
            SendMessage(prio, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0);

            /* set our preferences on what we already have */
            if(video_driver_list)
                SendDlgItemMessage(hwnd, ID_VO_DRIVER, CB_SETCURSEL,
                                   (WPARAM)SendMessage(vo_driver, CB_FINDSTRING, -1,
                                   (LPARAM)video_driver_list[0]), 0);

            if(audio_driver_list)
                SendDlgItemMessage(hwnd, ID_AO_DRIVER, CB_SETCURSEL,
                                   (WPARAM)SendMessage(ao_driver, CB_FINDSTRING, -1,
                                   (LPARAM)audio_driver_list[0]), 0);

            if(vo_doublebuffering)
                SendDlgItemMessage(hwnd, ID_DOUBLE, BM_SETCHECK, 1, 0);
            if(vo_directrendering)
                SendDlgItemMessage(hwnd, ID_DIRECT, BM_SETCHECK, 1, 0);
            if(!player_idle_mode)
                SendDlgItemMessage(hwnd, ID_IDLE, BM_SETCHECK, 1, 0);
            if(frame_dropping)
                SendDlgItemMessage(hwnd, ID_FRAMEDROP, BM_SETCHECK, 1, 0);
            if(gtkAONorm)
                SendDlgItemMessage(hwnd, ID_NORMALIZE, BM_SETCHECK, 1, 0);
            if(soft_vol)
                SendDlgItemMessage(hwnd, ID_SOFTMIX, BM_SETCHECK, 1, 0);
            if(gtkAOExtraStereo)
            {
                SendDlgItemMessage(hwnd, ID_EXTRASTEREO, BM_SETCHECK, 1, 0);
                if(!guiInfo.Playing)
                {
                    EnableWindow(track1, 1);
                    EnableWindow(track2, 1);
                }
            }
            else gtkAOExtraStereoMul = 1.0f;
            SendDlgItemMessage(hwnd, ID_TRACKBAR1, TBM_SETPOS, 1, (LPARAM)stereopos);

            SendDlgItemMessage(hwnd, ID_TRACKBAR2, TBM_SETPOS, 1, (LPARAM)delaypos);
            EnableWindow(GetDlgItem(hwnd, ID_TRACKBAR2), TRUE);

            if(gtkCacheOn) {
                SendDlgItemMessage(hwnd, ID_CACHE, BM_SETCHECK, 1, 0);
                EnableWindow(edit1, 1);
                EnableWindow(updown1, 1);
            }
            else gtkCacheSize = 2048;
            SendDlgItemMessage(hwnd, ID_UPDOWN1, UDM_SETPOS32, 0, (LPARAM)gtkCacheSize);

            if(gtkAutoSyncOn) {
                SendDlgItemMessage(hwnd, ID_AUTOSYNC, BM_SETCHECK, 1, 0);
                EnableWindow(edit2, 1);
                EnableWindow(updown2, 1);
            }
            else gtkAutoSync = 0;
            SendDlgItemMessage(hwnd, ID_UPDOWN2, UDM_SETPOS32, 0, (LPARAM)gtkAutoSync);

            if(video_window)
                SendDlgItemMessage(hwnd, ID_VIDEOWINDOW, BM_SETCHECK, 1, 0);

            if(!osd_level)
                SendDlgItemMessage(hwnd, ID_NONE, BM_SETCHECK, 1, 0);
            else if(osd_level == 1)
                SendDlgItemMessage(hwnd, ID_OSD1, BM_SETCHECK, 1, 0);
            else if(osd_level == 2)
                SendDlgItemMessage(hwnd, ID_OSD2, BM_SETCHECK, 1, 0);
            else if(osd_level == 3)
                SendDlgItemMessage(hwnd, ID_OSD3, BM_SETCHECK, 1, 0);

            if(proc_priority)
                SendDlgItemMessage(hwnd, ID_PRIO, CB_SETCURSEL,
                                   (WPARAM)SendMessage(prio, CB_FINDSTRING, -1,
                                   (LPARAM)get_priority(proc_priority, 0)), 0);

            else SendDlgItemMessage(hwnd, ID_PRIO, CB_SETCURSEL, 2, 0);

            break;
        }
        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            return (INT_PTR)SOLID_GREY;
        }
        break;
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case ID_EXTRASTEREO:
                {
                    if(SendDlgItemMessage(hwnd, ID_EXTRASTEREO, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        EnableWindow(GetDlgItem(hwnd, ID_TRACKBAR1), 1);
                    } else {
                        EnableWindow(GetDlgItem(hwnd, ID_TRACKBAR1), 0);
                        SendDlgItemMessage(hwnd, ID_TRACKBAR1, TBM_SETPOS, 1, (LPARAM)10.0);
                    }
                    break;
                }
                case ID_CACHE:
                {
                    if(SendDlgItemMessage(hwnd, ID_CACHE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        EnableWindow(GetDlgItem(hwnd, ID_EDIT1), 1);
                        EnableWindow(GetDlgItem(hwnd, ID_UPDOWN1), 1);
                    } else {
                        EnableWindow(GetDlgItem(hwnd, ID_EDIT1), 0);
                        EnableWindow(GetDlgItem(hwnd, ID_UPDOWN1), 0);
                        SendDlgItemMessage(hwnd, ID_UPDOWN1, UDM_SETPOS32, 1, (LPARAM)2048);
                    }
                    break;
                }
                case ID_AUTOSYNC:
                {
                    if(SendDlgItemMessage(hwnd, ID_AUTOSYNC, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        EnableWindow(GetDlgItem(hwnd, ID_EDIT2), 1);
                        EnableWindow(GetDlgItem(hwnd, ID_UPDOWN2), 1);
                    } else {
                        EnableWindow(GetDlgItem(hwnd, ID_EDIT2), 0);
                        EnableWindow(GetDlgItem(hwnd, ID_UPDOWN2), 0);
                        SendDlgItemMessage(hwnd, ID_UPDOWN2, UDM_SETPOS32, 1, (LPARAM)0);
                    }
                    break;
                }
                case ID_DEFAULTS:
                {
                    SendDlgItemMessage(hwnd, ID_VO_DRIVER, CB_SETCURSEL,
                                       (WPARAM)SendMessage(vo_driver, CB_FINDSTRING, -1, (LPARAM)"directx"), 0);

                    SendDlgItemMessage(hwnd, ID_AO_DRIVER, CB_SETCURSEL,
                                       (WPARAM)SendMessage(ao_driver, CB_FINDSTRING, -1, (LPARAM)"dsound"), 0);

                    SendDlgItemMessage(hwnd, ID_PRIO, CB_SETCURSEL,
                                       (WPARAM)SendMessage(prio, CB_FINDSTRING, -1, (LPARAM)"normal"), 0);

                    SendDlgItemMessage(hwnd, ID_TRACKBAR1, TBM_SETPOS, TRUE, (LPARAM)10.0);
                    SendDlgItemMessage(hwnd, ID_TRACKBAR2, TBM_SETPOS, TRUE, (LPARAM)0.0);
                    SendDlgItemMessage(hwnd, ID_UPDOWN1, UDM_SETPOS32, 0, (LPARAM)gtkCacheSize);
                    SendDlgItemMessage(hwnd, ID_UPDOWN2, UDM_SETPOS32, 0, (LPARAM)gtkAutoSync);
                    SendDlgItemMessage(hwnd, ID_DOUBLE, BM_SETCHECK, BST_CHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_DIRECT, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_IDLE, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_FRAMEDROP, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_NORMALIZE, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_SOFTMIX, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_EXTRASTEREO, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_CACHE, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_AUTOSYNC, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_VIDEOWINDOW, BM_SETCHECK, BST_CHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_NONE, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_OSD1, BM_SETCHECK, BST_CHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_OSD2, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendDlgItemMessage(hwnd, ID_OSD3, BM_SETCHECK, BST_UNCHECKED, 0);
                    break;
                }
                case ID_CANCEL:
                    DestroyWindow(hwnd);
                    return 0;
                case ID_APPLY:
                {
                    int idx, strl;
                    char *driver, *procprio, *caption;

                    /* Set the video driver */
                    idx = SendMessage(vo_driver, CB_GETCURSEL, 0, 0);
                    strl = SendMessage(vo_driver, CB_GETLBTEXTLEN, (WPARAM)idx, 0);
                    driver = malloc(strl + 1);
                    SendMessage(vo_driver, CB_GETLBTEXT, (WPARAM)idx, (LPARAM)driver);
                    listSet(&video_driver_list, driver);
                    free(driver);

                    /* Set the audio driver */
                    idx = SendMessage(ao_driver, CB_GETCURSEL, 0, 0);
                    strl = SendMessage(ao_driver, CB_GETLBTEXTLEN, (WPARAM)idx, 0);
                    driver = malloc(strl + 1);
                    SendMessage(ao_driver, CB_GETLBTEXT, (WPARAM)idx, (LPARAM)driver);
                    listSet(&audio_driver_list, driver);
                    free(driver);

                    /* Set the priority level */
                    idx = SendMessage(prio, CB_GETCURSEL, 0, 0);
                    strl = SendMessage(prio, CB_GETLBTEXTLEN, (WPARAM)idx, 0);
                    procprio = malloc(strl + 1);
                    SendMessage(prio, CB_GETLBTEXT, (WPARAM)idx, (LPARAM)procprio);
                    setdup(&proc_priority, get_priority(procprio, 1));
                    free(procprio);

                    /* double buffering */
                    if(SendDlgItemMessage(hwnd, ID_DOUBLE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        vo_doublebuffering = TRUE;
                    else vo_doublebuffering = FALSE;

                    /* direct rendering */
                    if(SendDlgItemMessage(hwnd, ID_DIRECT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        vo_directrendering = TRUE;
                    else vo_directrendering = FALSE;

                    /* quit after playing */
                    if(SendDlgItemMessage(hwnd, ID_IDLE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        player_idle_mode = FALSE;
                    else player_idle_mode = TRUE;

                    /* frame dropping */
                    if(SendDlgItemMessage(hwnd, ID_FRAMEDROP, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        frame_dropping = 1;
                    else frame_dropping = 0;

                    /* normalize */
                    if(SendDlgItemMessage(hwnd, ID_NORMALIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        gtkAONorm = TRUE;
                    else gtkAONorm = FALSE;

                    /* software mixer */
                    if(SendDlgItemMessage(hwnd, ID_SOFTMIX, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        soft_vol = TRUE;
                    else soft_vol = FALSE;

                    /* extra stereo */
                    if(SendDlgItemMessage(hwnd, ID_EXTRASTEREO, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        gtkAOExtraStereo = TRUE;
                    else {
                        gtkAOExtraStereo = FALSE;
                        gtkAOExtraStereoMul = 10.0f;
                    }
                    gtkAOExtraStereoMul = SendDlgItemMessage(hwnd, ID_TRACKBAR1, TBM_GETPOS, 0, 0) / 10.0;

                    /* audio delay */
                    audio_delay = SendDlgItemMessage(hwnd, ID_TRACKBAR2, TBM_GETPOS, 0, 0) / 10.0;

                    /* cache */
                    if(SendDlgItemMessage(hwnd, ID_CACHE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        gtkCacheOn = TRUE;
                    else gtkCacheOn = FALSE;
                    gtkCacheSize = SendDlgItemMessage(hwnd, ID_UPDOWN1, UDM_GETPOS32, 0, (LPARAM)NULL);

                    /* autosync */
                    if(SendDlgItemMessage(hwnd, ID_AUTOSYNC, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        gtkAutoSyncOn = TRUE;
                    else gtkAutoSyncOn = FALSE;
                    gtkAutoSync = SendDlgItemMessage(hwnd, ID_UPDOWN2, UDM_GETPOS32, 0, (LPARAM)NULL);

                    /* video window */
                    if(SendDlgItemMessage(hwnd, ID_VIDEOWINDOW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        video_window = TRUE;
                    else video_window = FALSE;

                    /* osd level */
                    if(SendDlgItemMessage(hwnd, ID_NONE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        osd_level = 0;
                    else if(SendDlgItemMessage(hwnd, ID_OSD1, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        osd_level = 1;
                    else if(SendDlgItemMessage(hwnd, ID_OSD2, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        osd_level = 2;
                    else if(SendDlgItemMessage(hwnd, ID_OSD3, BM_GETCHECK, 0, 0) == BST_CHECKED)
                        osd_level = 3;

                    if (guiInfo.Playing)
                    {
                        caption = strdup(acp(MSGTR_GUI_Information));
                        MessageBox(hwnd, acp(MSGTR_GUI_MSG_PlaybackNeedsRestart), caption, MB_OK | MB_ICONINFORMATION);
                        free(caption);
                    }

                    DestroyWindow(hwnd);
                    break;
                }
            }
            return 0;
        }
    }
    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void display_prefswindow(gui_t *gui)
{
    HWND hWnd;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc;
    int x, y;
    if(FindWindow(NULL, acp(MSGTR_GUI_Preferences))) return;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = PrefsWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
    wc.hIcon         = gui->icon;
    wc.hbrBackground = SOLID_GREY;
    wc.lpszClassName = acp(MSGTR_GUI_Preferences);
    wc.lpszMenuName  = NULL;
    RegisterClass(&wc);
    x = (GetSystemMetrics(SM_CXSCREEN) / 2) - (425 / 2);
    y = (GetSystemMetrics(SM_CYSCREEN) / 2) - (474 / 2);
    hWnd = CreateWindow(acp(MSGTR_GUI_Preferences),
                        acp(MSGTR_GUI_Preferences),
                        WS_POPUPWINDOW | WS_CAPTION,
                        x,
                        y,
                        425,
                        474,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);
   SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) gui);
   ShowWindow(hWnd, SW_SHOW);
   UpdateWindow(hWnd);
}
