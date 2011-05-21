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

// implementation header
#include "CommandCompleter.h"


CommandCompleter::CommandCompleter() {
  reset();
}

void CommandCompleter::reset() {
  words.clear();
  registerWord("/ban ");
  registerWord("/banlist");
  registerWord("/calc ");
  registerWord("/checkip ");
  registerWord("/countdown");
  registerWord("/clientquery");
  registerWord("/connect"); // /hub sub-command
  registerWord("/date");
  registerWord("/debug ");
  registerWord("disable");
  registerWord("/disconnect"); // /hub sub-command
  registerWord("/dumpvars");
  registerWord("/flag ");
  registerWord("reset");
  registerWord("up");
  registerWord("show");
  registerWord("/flaghistory");
  registerWord("/gameover");
  registerWord("/grouplist");
  registerWord("/groupperms");
  registerWord("/help");
  registerWord("/highlight ");
  registerWord("/hostban ");
  registerWord("/hostunban ");
  registerWord("/hostbanlist");
  registerWord("/hub ");
  registerWord("/idban ");
  registerWord("/idunban ");
  registerWord("/idbanlist");
  registerWord("/idlist");
  registerWord("/idlestats");
  registerWord("/jitterdrop");
  registerWord("/jitterwarn");
  registerWord("/packetlossdrop");
  registerWord("/packetlosswarn");
  registerWord("/kick ");
  registerWord("/kill ");
  registerWord("/lagdrop");
  registerWord("/lagstats");
  registerWord("/lagwarn ");
  registerWord("/localset ");
  registerWord("/luauser ");
  registerWord("/luabzorg ");
  registerWord("/luaworld ");
  registerWord("/luarules ");
  registerWord("/luaserver ");
  registerWord("/luacheck");
  registerWord("/mute ");
  registerWord("/owner");
  registerWord("/password ");
  registerWord("/playerlist");
  registerWord("/poll ");
  registerWord("ban");
  registerWord("kick");
  registerWord("kill");
  registerWord("/quit");
  registerWord("/record ");
  registerWord("/replay ");
  registerWord("reload");
  registerWord("start");
  registerWord("stop");
  registerWord("size");
  registerWord("rate");
  registerWord("stats");
  registerWord("file");
  registerWord("save");
  registerWord("/reload");
  registerWord("/masterban"); // also uses list
  registerWord("reload");
  registerWord("flush");
  registerWord("/removegroup ");
  registerWord("list");
  registerWord("load");
  registerWord("play");
  registerWord("skip");
  registerWord("/report ");
  registerWord("/reset");
  registerWord("/retexture");
  registerWord("/roampos ");
  registerWord("/saveworld ");
  registerWord("/serverdebug ");
  registerWord("/serverquery");
  registerWord("/set");
  registerWord("/setgroup ");
  registerWord("/showgroup ");
  registerWord("/showperms ");
  registerWord("/shutdownserver");
  registerWord("/silence ");
  registerWord("/unsilence ");
  registerWord("/status"); // /hub sub-command
  registerWord("/superkill");
  registerWord("/time");
  registerWord("/unban ");
  registerWord("/unmute ");
  registerWord("/uptime");
  registerWord("/veto");
  registerWord("/viewreports");
  registerWord("/vote");
  registerWord("/worldinfo");
  registerWord("/loadplugin");
  registerWord("/listplugins");
  registerWord("/unloadplugin");
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
