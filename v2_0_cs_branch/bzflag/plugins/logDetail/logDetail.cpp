/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// logDetail.cpp : Plugin module for logging server events to stdout
//

#include <iostream>
#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

using namespace std;

class logDetail : public bz_EventHandler
{
public:
  logDetail() {};
  virtual ~logDetail() {};
  virtual void process( bz_EventData *eventData );
private:
  void displayPlayerPrivs( int playerID );
  void displayCallsign( bzApiString callsign );
  void displayCallsign( int playerID );
  void displayTeam( bz_eTeamType team );
};

logDetail logDetailHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_registerEvent(bz_eSlashCommandEvent, &logDetailHandler);
  bz_registerEvent(bz_eChatMessageEvent, &logDetailHandler);
  bz_registerEvent(bz_eServerMsgEvent, &logDetailHandler);
  bz_registerEvent(bz_ePlayerJoinEvent, &logDetailHandler);
  bz_registerEvent(bz_ePlayerPartEvent, &logDetailHandler);
  bz_registerEvent(bz_ePlayerAuthEvent, &logDetailHandler);
  bz_debugMessage(4, "logDetail plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_eSlashCommandEvent, &logDetailHandler);
  bz_removeEvent(bz_eChatMessageEvent, &logDetailHandler);
  bz_removeEvent(bz_eServerMsgEvent, &logDetailHandler);
  bz_removeEvent(bz_ePlayerJoinEvent, &logDetailHandler);
  bz_removeEvent(bz_ePlayerPartEvent, &logDetailHandler);
  bz_removeEvent(bz_ePlayerAuthEvent, &logDetailHandler);
  bz_debugMessage(4, "logDetail plugin unloaded");
  return 0;
}

void logDetail::process( bz_EventData *eventData )
{
  bz_ChatEventData *chatData = (bz_ChatEventData *) eventData;
  bz_ServerMsgEventData *serverMsgData = (bz_ServerMsgEventData *) eventData;
  bz_SlashCommandEventData *cmdData = (bz_SlashCommandEventData *) eventData;
  bz_PlayerJoinPartEventData *joinPartData = (bz_PlayerJoinPartEventData *) eventData;
  bz_PlayerAuthEventData *authData = (bz_PlayerAuthEventData *) eventData;
  char temp[9] = {0};

  if (eventData) {
    switch (eventData->eventType) {
      case bz_eSlashCommandEvent:
	// Slash commands are case insensitive
	// Tokenize the stream and check the first word
	// /report -> MSG-REPORT
	// anything -> MSG-COMMAND

	strncpy(temp, cmdData->message.c_str(), 8);

	if (strcasecmp( temp, "/REPORT ") == 0) {
	  cout << "MSG-REPORT ";
	  displayCallsign( cmdData->from );
	  cout << " " << cmdData->message.c_str()+8 << endl;
	} else {
	  cout << "MSG-COMMAND ";
	  displayCallsign( cmdData->from );
	  cout << " " << cmdData->message.c_str()+1 << endl;
	}
	break;
      case bz_eChatMessageEvent:
	if ((chatData->to == BZ_ALLUSERS) and (chatData->team == eNoTeam)) {
	  cout << "MSG-BROADCAST ";
	  displayCallsign( chatData->from );
	  cout << " " << chatData->message.c_str() << endl;
	} else if (chatData->to == BZ_NULLUSER) {
	  if (chatData->team == eAdministrators) {
	    cout << "MSG-ADMIN ";
	    displayCallsign( chatData->from );
	    cout << " " << chatData->message.c_str() << endl;
	  } else {
	    cout << "MSG-TEAM ";
	    displayCallsign( chatData->from );
	    displayTeam( chatData->team );
	    cout << " " << chatData->message.c_str() << endl;
	  }
	} else {
	  cout << "MSG-DIRECT ";
	  displayCallsign( chatData->from );
	  cout << " ";
	  displayCallsign( chatData->to );
	  cout << " " << chatData->message.c_str() << endl;
	}
	break;
      case bz_eServerMsgEvent:
	if ((serverMsgData->to == BZ_ALLUSERS) and (serverMsgData->team == eNoTeam)) {
	  cout << "MSG-BROADCAST 6:SERVER";
	  cout << " " << serverMsgData->message.c_str() << endl;
	} else if (serverMsgData->to == BZ_NULLUSER) {
	  if (serverMsgData->team == eAdministrators) {
	    cout << "MSG-ADMIN 6:SERVER";
	    cout << " " << serverMsgData->message.c_str() << endl;
	  } else {
	    cout << "MSG-TEAM 6:SERVER";
	    displayTeam( serverMsgData->team );
	    cout << " " << chatData->message.c_str() << endl;
	  }
	} else {
	  cout << "MSG-DIRECT 6:SERVER";
	  cout << " ";
	  displayCallsign( serverMsgData->to );
	  cout << " " << serverMsgData->message.c_str() << endl;
	}
	break;
      case bz_ePlayerJoinEvent:
	{
	  bz_PlayerRecord *player = bz_getPlayerByIndex( joinPartData->playerID );
	  if (player) {
	    cout << "PLAYER-JOIN ";
	    displayCallsign( player->callsign );
	    cout << " #" << joinPartData->playerID;
	    displayTeam( joinPartData->team );
	    displayPlayerPrivs( joinPartData->playerID );
	    cout << endl;
	  }
	}
	break;
      case bz_ePlayerPartEvent:
	cout << "PLAYER-PART ";
	displayCallsign( joinPartData->playerID );
	cout << " #" << joinPartData->playerID;
	cout << " " << joinPartData->reason.c_str();
	cout << endl;
	break;
      case bz_ePlayerAuthEvent:
	cout << "PLAYER-AUTH ";
	displayCallsign( authData->playerID );
	displayPlayerPrivs( authData->playerID );
	cout << endl;
	break;
      default :
	break;
    }
  }
}

void logDetail::displayPlayerPrivs( int playerID )
{
  bz_PlayerRecord *player = bz_getPlayerByIndex( playerID );
  if (player) {
    cout << " IP:" << player->ipAddress.c_str();
    if (player->verified ) cout << " VERIFIED";
    if (player->globalUser ) cout << " GLOBALUSER";
    if (player->admin ) cout << " ADMIN";
    if (player->op ) cout << " OPERATOR";
  } else {
    cout << " IP:0.0.0.0";
  }
}

void logDetail::displayCallsign( bzApiString callsign )
{
  cout << strlen( callsign.c_str() ) << ":";
  cout << callsign.c_str();
}

void logDetail::displayCallsign( int playerID )
{
  bz_PlayerRecord *player = bz_getPlayerByIndex( playerID );
  if (player) {
    cout << strlen( player->callsign.c_str() ) << ":";
    cout << player->callsign.c_str();
  } else {
    cout << "7:UNKNOWN";
  }
}


void logDetail::displayTeam( bz_eTeamType team )
{
  // Display the player team
  switch ( team ) {
    case eRogueTeam:
      cout << " ROGUE";
      break;
    case eRedTeam:
      cout << " RED";
      break;
    case eGreenTeam:
      cout << " GREEN";
      break;
    case eBlueTeam:
      cout << " BLUE";
      break;
    case ePurpleTeam:
      cout << " PURPLE";
      break;
    case eRabbitTeam:
      cout << " RABBIT";
      break;
    case eHunterTeam:
      cout << " HUNTER";
      break;
    case eObservers:
      cout << " OBSERVER";
      break;
    default :
      cout << " NOTEAM";
      break;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
