/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDuiServerList:
 *	User interface class for displaying the servers in a ServerList.
 */

#ifndef	__HUDUISERVERLIST_H__
#define	__HUDUISERVERLIST_H__

// ancestor class
#include "HUDuiScrollList.h"

#include "HUDuiServerListItem.h"
#include "ServerItem.h"
#include "ServerList.h"

#include "HUDuiFrame.h"

class HUDuiServerList : public HUDuiScrollList {
  public:
      HUDuiServerList();

    typedef enum {
      EmptyServer         = (1 << 0),
      FullServer          = (1 << 1),
      JumpingOn           = (1 << 2),
      JumpingOff          = (1 << 3),
      RicochetOn          = (1 << 4),
      RicochetOff         = (1 << 5),
      AntidoteFlagOn      = (1 << 6),
      AntidoteFlagOff     = (1 << 7),
      SuperFlagsOn        = (1 << 8),
      SuperFlagsOff       = (1 << 9),
      HandicapOn          = (1 << 10),
      HandicapOff         = (1 << 11),
      ClassicCTFGameMode  = (1 << 12),
      RabbitChaseGameMode = (1 << 13),
      OpenFFAGameMode     = (1 << 14),
      TeamFFAGameMode     = (1 << 15),
      LuaWorldOn          = (1 << 16),
      LuaWorldOff         = (1 << 17),
      LuaWorldReqOn       = (1 << 18),
      LuaWorldReqOff      = (1 << 19),
      EndOfFilterConstants
    } FilterConstants;

    typedef enum {
      Modes = 0,
      DomainName,
      ServerName,
      PlayerCount,
      Ping,
      NoSort
    } SortConstants;

    static float MODES_PERCENTAGE;
    static float DOMAIN_PERCENTAGE;
    static float SERVER_PERCENTAGE;
    static float PLAYER_PERCENTAGE;
    static float PING_PERCENTAGE;

    std::map<int, std::pair<std::string, float*> > columns;

    void addItem(ServerItem* item);
    void addItem(std::string key);
    void addItem(HUDuiControl* item);

    void removeItem(ServerItem* item);
    void clearList();

    void update();

    HUDuiServerListItem* get(size_t index);

    ServerItem* getSelectedServer();

    void sortBy(SortConstants sortType);

    SortConstants getSortMode() { return sortMode; }
    bool getReverseSort() { return reverseSort; }
    void setReverseSort(bool reverse);

    float getHeight() const;

    void applyFilters();
    void applyFilters(uint32_t filters);
    void serverNameFilter(std::string pattern);
    void domainNameFilter(std::string pattern);
    void toggleFilter(FilterConstants filter);

    uint32_t getFilterOptions() { return filterOptions; }
    std::pair<std::string, std::string> getFilterPatterns() { return filterPatterns; }

    void setFontSize(float size);
    void setFontFace(const LocalFontFace* face);
    void setSize(float width, float height);

    ServerList &dataList;

  protected:
    struct filter;
    struct search;
    template<int sortType> struct compare;

    static bool comp(HUDuiControl* first, HUDuiControl* second);
    static bool equal(HUDuiControl* first, HUDuiControl* second);

    size_t callbackHandler(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod);

    bool doKeyPress(const BzfKeyEvent& key);

    void setActiveColumn(int column);
    int getActiveColumn();

    void refreshNavQueue();

    void doRender();

    bool reverseSort;

  private:
    std::list<HUDuiControl*> originalItems;

    SortConstants sortMode;
    uint32_t filterOptions;
    std::pair<std::string, std::string> filterPatterns;

    int activeColumn;

    bool devInfo;
};

#endif // __HUDuiServerList_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
