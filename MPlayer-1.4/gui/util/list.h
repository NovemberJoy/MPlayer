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

#ifndef MPLAYER_GUI_LIST_H
#define MPLAYER_GUI_LIST_H

/// listMgr() commands
enum {
    GAINLIST_ITEM_INSERT,
    GAINLIST_ITEM_FIND,
    PLAYLIST_GET,
    PLAYLIST_ITEM_APPEND,
    PLAYLIST_ITEM_INSERT,
    PLAYLIST_ITEM_FIND,
    PLAYLIST_ITEM_SET_CURR,
    PLAYLIST_ITEM_GET_CURR,
    PLAYLIST_ITEM_GET_POS,
    PLAYLIST_ITEM_GET_PREV,
    PLAYLIST_ITEM_GET_NEXT,
    PLAYLIST_ITEM_GET_LAST,
    PLAYLIST_ITEM_DEL_CURR,
    PLAYLIST_DELETE,
    PLITEM_COPY,
    PLITEM_FREE,
    URLLIST_GET,
    URLLIST_ITEM_ADD,
    URLLIST_DELETE
};

typedef struct gainItem {
    char *filename;
    float replay_gain;
    struct gainItem *next;
} gainItem;

typedef struct plItem {
    char *path;
    char *name;
    char *title;
    int start;
    int stop;
    struct plItem *prev, *next;
} plItem;

typedef struct urlItem {
    char *url;
    struct urlItem *next;
} urlItem;

/// @name List manager (playlist, URL list)
void *listMgr(int cmd, void *data);

/// @name String list operations
//@{
char **listDup(const char * const *list);
void listFree(char ***list);
void listRepl(char ***list, const char *search, const char *replace);
void listSet(char ***list, const char *entry);
//@}

/// @name High-level list operations
//@{
int add_to_gui_playlist(const char *what, int how);
//@}

#endif /* MPLAYER_GUI_LIST_H */
