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

// all the exported functions for bzfs plugins

#ifndef _BZFS_API_H_
#define _BZFS_API_H_

/* system interface headers */
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>


/* DO NOT INCLUDE ANY OTHER HEADERS IN THIS FILE */
/* PLUGINS NEED TO BE BUILT WITHOUT THE BZ SOURCE TREE */
/* JUST THIS ONE FILE */

#ifdef _WIN32
#ifdef INSIDE_BZ
#define BZF_API __declspec( dllexport )
#else
#define BZF_API __declspec( dllimport )
#endif
#define BZF_PLUGIN_CALL
#ifndef strcasecmp
#define strcasecmp stricmp
#endif
#else
#define BZF_API
#define BZF_PLUGIN_CALL extern "C"
#endif

class bz_Plugin;

#define BZ_API_VERSION	24

#define BZ_GET_PLUGIN_VERSION BZF_PLUGIN_CALL int bz_GetMinVersion ( void ) { return BZ_API_VERSION;}

#define BZ_PLUGIN(n)\
  BZF_PLUGIN_CALL bz_Plugin* bz_GetPlugin ( void ) { return new n;}\
  BZF_PLUGIN_CALL void bz_FreePlugin ( bz_Plugin* plugin ) { delete(plugin);}\
  BZF_PLUGIN_CALL int bz_GetMinVersion ( void ) { return BZ_API_VERSION;}

/** This is so we can use gcc's "format string vs arguments"-check
 * for various printf-like functions, and still maintain compatability.
 * Not tested on other platforms yet, but should work. */
#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5)
#  define __attribute__(Spec) /* empty */
# endif
/* The __-protected variants of `format' and `printf' attributes
 *    are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif

/** shorthand defines to make the code cleaner. */
#define _ATTRIBUTE34 __attribute__ ((__format__ (__printf__, 3, 4)))
#define _ATTRIBUTE23 __attribute__ ((__format__ (__printf__, 2, 3)))
#define _ATTRIBUTE12 __attribute__ ((__format__ (__printf__, 1, 2)))

#ifdef __cplusplus
#  ifndef DEFINED_FORCE_CAST
#    define DEFINED_FORCE_CAST
template<class To, class From>
inline To force_cast(From const & f)
{
  union {
    From f;
    To t;
  } fc;
  fc.f = f;
  return fc.t;
}
#  endif
#endif

//utility classes
class BZF_API bz_ApiString
{
public:
  bz_ApiString();
  bz_ApiString(const char* c);
  bz_ApiString(const std::string &s);
  bz_ApiString(const bz_ApiString &r);

  ~bz_ApiString();

  bz_ApiString& operator = ( const bz_ApiString& r );
  bz_ApiString& operator = ( const std::string& r );
  bz_ApiString& operator = ( const char* r );

  bool operator == ( const bz_ApiString&r );
  bool operator == ( const std::string& r );
  bool operator == ( const char* r );

  bool operator != ( const bz_ApiString&r );
  bool operator != ( const std::string& r );
  bool operator != ( const char* r );

  unsigned int size ( void ) const;

  const char* c_str(void) const;

  void format(const char* fmt, ...);

  void replaceAll ( const char* target, const char* with );

  void tolower ( void );
  void toupper ( void );
  void urlEncode ( void );

protected:
  class dataBlob;

  dataBlob	*data;
};

class BZF_API bz_APIIntList
{
public:
  bz_APIIntList();
  bz_APIIntList(const bz_APIIntList	&r);
  bz_APIIntList(const std::vector<int>	&r);

  ~bz_APIIntList();

  void push_back ( int value );
  int get ( unsigned int i );

  const int& operator[] (unsigned int i) const;
  bz_APIIntList& operator = ( const bz_APIIntList& r );
  bz_APIIntList& operator = ( const std::vector<int>& r );

  unsigned int size ( void );
  void clear ( void );

protected:
  class dataBlob;

  dataBlob *data;
};

BZF_API bz_APIIntList* bz_newIntList ( void );
BZF_API void bz_deleteIntList( bz_APIIntList * l );

class BZF_API bz_APIFloatList
{
public:
  bz_APIFloatList();
  bz_APIFloatList(const bz_APIFloatList	&r);
  bz_APIFloatList(const std::vector<float>	&r);

  ~bz_APIFloatList();

  void push_back ( float value );
  float get ( unsigned int i );

  const float& operator[] (unsigned int i) const;
  bz_APIFloatList& operator = ( const bz_APIFloatList& r );
  bz_APIFloatList& operator = ( const std::vector<float>& r );

  unsigned int size ( void );
  void clear ( void );

protected:
  class dataBlob;

  dataBlob *data;
};

BZF_API bz_APIFloatList* bz_newFloatList ( void );
BZF_API void bz_deleteFloatList( bz_APIFloatList * l );

class BZF_API bz_APIStringList
{
public:
  bz_APIStringList();
  bz_APIStringList(const bz_APIStringList	&r);
  bz_APIStringList(const std::vector<std::string>	&r);

  ~bz_APIStringList();

  void push_back ( const bz_ApiString &value );
  void push_back ( const std::string &value );
  bz_ApiString get ( unsigned int i );

  const bz_ApiString& operator[] (unsigned int i) const;
  bz_APIStringList& operator = ( const bz_APIStringList& r );
  bz_APIStringList& operator = ( const std::vector<std::string>& r );

  unsigned int size ( void );
  void clear ( void );

  void tokenize ( const char* in, const char* delims, int maxTokens = 0, bool useQuotes = false);
protected:
  class dataBlob;

  dataBlob *data;
};

BZF_API bz_APIStringList* bz_newStringList ( void );
BZF_API void bz_deleteStringList( bz_APIStringList * l );

// current time (leave method here, used in bz_EventData constructor)
BZF_API double bz_getCurrentTime(void);

// versioning
BZF_API int bz_APIVersion ( void );

// event stuff

typedef enum
{
  bz_eNullEvent = 0,
  bz_eCaptureEvent,
  bz_ePlayerDieEvent,
  bz_ePlayerSpawnEvent,
  bz_eZoneEntryEvent,
  bz_eZoneExitEvent,
  bz_ePlayerJoinEvent,
  bz_ePlayerPartEvent,
  bz_eRawChatMessageEvent,	// before filter
  bz_eFilteredChatMessageEvent,	// after filter
  bz_eUnknownSlashCommand,
  bz_eGetPlayerSpawnPosEvent,
  bz_eGetAutoTeamEvent,
  bz_eAllowPlayer,
  bz_eTickEvent,
  bz_eGetWorldEvent,
  bz_eGetPlayerInfoEvent,
  bz_eAllowSpawn,
  bz_eListServerUpdateEvent,
  bz_eBanEvent,
  bz_eHostBanModifyEvent,
  bz_eKickEvent,
  bz_eKillEvent,
  bz_ePlayerPausedEvent,
  bz_eMessageFilteredEvent,
  bz_eGameStartEvent,
  bz_eGameEndEvent,
  bz_eSlashCommandEvent,
  bz_ePlayerAuthEvent,
  bz_eServerMsgEvent,
  bz_eShotFiredEvent,
  bz_ePlayerUpdateEvent,
  bz_eNetDataSendEvent,
  bz_eNetDataReceiveEvent,
  bz_eLoggingEvent,
  bz_eShotEndedEvent,
  bz_eFlagTransferredEvent,
  bz_eFlagGrabbedEvent,
  bz_eFlagDroppedEvent,
  bz_eAllowCTFCaptureEvent,
  bz_eMsgDebugEvent,
  bz_eNewNonPlayerConnection,
  bz_eLastEvent    //this is never used as an event, just show it's the last one
}bz_eEventType;

// permision #defines
#define bz_perm_actionMessage  "actionMessage"
#define bz_perm_adminMessageReceive  "adminMessageReceive"
#define bz_perm_adminMessageSend  "adminMessageSend"
#define bz_perm_antiban   "antiban"
#define bz_perm_antideregister   "antideregister"
#define bz_perm_antikick   "antikick"
#define bz_perm_antikill   "antikill"
#define bz_perm_antipoll   "antipoll"
#define bz_perm_antipollban   "antipollban"
#define bz_perm_antipollkick   "antipollkick"
#define bz_perm_antipollkill   "antipollkill"
#define bz_perm_ban  "ban"
#define bz_perm_banlist  "banlist"
#define bz_perm_countdown  "countdown"
#define bz_perm_date  "date"
#define bz_perm_endGame  "endGame"
#define bz_perm_flagHistory  "flagHistory"
#define bz_perm_flagMod  "flagMod"
#define bz_perm_hideAdmin  "hideAdmin"
#define bz_perm_idleStats  "idleStats"
#define bz_perm_info  "info"
#define bz_perm_kick  "kick"
#define bz_perm_kill  "kill"
#define bz_perm_lagStats  "lagStats"
#define bz_perm_lagwarn  "lagwarn"
#define bz_perm_listPerms  "listPerms"
#define bz_perm_masterBan  "masterban"
#define bz_perm_mute  "mute"
#define bz_perm_playerList  "playerList"
#define bz_perm_poll  "poll"
#define bz_perm_pollBan  "pollBan"
#define bz_perm_pollKick  "pollKick"
#define bz_perm_pollKill  "pollKill"
#define bz_perm_pollSet  "pollSet"
#define bz_perm_pollFlagReset  "pollFlagReset"
#define bz_perm_privateMessage  "privateMessage"
#define bz_perm_record  "record"
#define bz_perm_rejoin  "rejoin"
#define bz_perm_removePerms  "removePerms"
#define bz_perm_replay  "replay"
#define bz_perm_requireIdentify  "requireIdentify"
#define bz_perm_say  "say"
#define bz_perm_sendHelp  "sendHelp"
#define bz_perm_setAll  "setAll"
#define bz_perm_setPassword  "setPassword"
#define bz_perm_setPerms  "setPerms"
#define bz_perm_setVar  "setVar"
#define bz_perm_showOthers  "showOthers"
#define bz_perm_shortBan  "shortBan"
#define bz_perm_shutdownServer  "shutdownServer"
#define bz_perm_spawn  "spawn"
#define bz_perm_superKill  "superKill"
#define bz_perm_talk  "talk"
#define bz_perm_unban  "unban"
#define bz_perm_unmute  "unmute"
#define bz_perm_veto  "veto"
#define bz_perm_viewReports  "viewReports"
#define bz_perm_vote  "vote"

typedef enum
{
  eNoTeam = -1,
  eRogueTeam = 0,
  eRedTeam,
  eGreenTeam,
  eBlueTeam,
  ePurpleTeam,
  eRabbitTeam,
  eHunterTeam,
  eObservers,
  eAdministrators
}bz_eTeamType;

#define BZ_SERVER		-2
#define BZ_ALLUSERS		-1
#define BZ_NULLUSER		-3

#define BZ_BZDBPERM_NA		0
#define BZ_BZDBPERM_USER	1
#define BZ_BZDBPERM_SERVER	2
#define BZ_BZDBPERM_CLIENT	3

typedef enum
{
  eFFAGame= 0,
  eCTFGame,
  eRabbitGame
}bz_eGameType;

typedef enum {
  eDead,		// not alive, not paused, etc.
  eAlive,		// player is alive
  ePaused,		// player is paused
  eExploding,		// currently blowing up
  eTeleporting,		// teleported recently
  eInBuilding		// has OO and is in a building
} bz_ePlayerStatus;

typedef struct bz_PlayerUpdateState {
  bz_ePlayerStatus	status;			// special states
  bool			falling;		// not driving on the ground or an obstacle
  bool			crossingWall;		// crossing an obstacle wall
  bool			inPhantomZone;		// zoned
  float			pos[3];			// position of tank
  float			velocity[3];		// velocity of tank
  float			rotation;		// orientation of tank
  float			angVel;			// angular velocity of tank
  int			phydrv;			// physics driver
} bz_PlayerUpdateState;

class bz_BasePlayerRecord;
BZF_API bool bz_freePlayerRecord ( bz_BasePlayerRecord *playerRecord );

// event data types
class BZF_API bz_EventData
{
public:
  bz_EventData(bz_eEventType type =  bz_eNullEvent)
    : version(1), eventType(type), eventTime( bz_getCurrentTime() )
  {
  }
  virtual ~bz_EventData() {}
  virtual void update() {}

  int version;
  bz_eEventType eventType;
  double eventTime;
};

class BZF_API bz_CTFCaptureEventData_V1 : public bz_EventData
{
public:
  bz_CTFCaptureEventData_V1() : bz_EventData(bz_eCaptureEvent)
    , teamCapped(eNoTeam), teamCapping(eNoTeam), playerCapping(-1)
    , rot(0.0)
  {
  }

  bz_eTeamType teamCapped;
  bz_eTeamType teamCapping;
  int playerCapping;

  float pos[3];
  float rot;
};

class BZF_API bz_PlayerDieEventData_V1 : public bz_EventData
{
public:
  bz_PlayerDieEventData_V1() : bz_EventData(bz_ePlayerDieEvent)
    , playerID(-1), team(eNoTeam), killerID(-1), killerTeam(eNoTeam)
    , shotID(-1) 
  {
  }

  int playerID;
  bz_eTeamType team;
  int killerID;
  bz_eTeamType killerTeam;
  bz_ApiString flagKilledWith;
  int shotID;

   bz_PlayerUpdateState state;
};

class BZF_API bz_PlayerSpawnEventData_V1 : public bz_EventData
{
public:
  bz_PlayerSpawnEventData_V1() : bz_EventData(bz_ePlayerSpawnEvent)
    , playerID(-1), team(eNoTeam)
  {
  }

  int playerID;
  bz_eTeamType team;
  bz_PlayerUpdateState state;
};

class BZF_API bz_ChatEventData_V1 : public bz_EventData
{
public:
  bz_ChatEventData_V1() : bz_EventData(bz_eRawChatMessageEvent)
    , from(-1), to(-1), team(eNoTeam)
  {
  }

  int from;
  int to;
  bz_eTeamType team;
  bz_ApiString message;
};

class BZF_API bz_PlayerJoinPartEventData_V1 : public bz_EventData
{
public:
  bz_PlayerJoinPartEventData_V1() : bz_EventData(bz_ePlayerJoinEvent)
    , playerID(-1), record(0)
  {
  }
  ~bz_PlayerJoinPartEventData_V1()
  {
    bz_freePlayerRecord(record);
  }

  int playerID;
  bz_BasePlayerRecord* record;
  bz_ApiString reason;
};

class BZF_API bz_UnknownSlashCommandEventData_V1 : public bz_EventData
{
public:
  bz_UnknownSlashCommandEventData_V1() : bz_EventData(bz_eUnknownSlashCommand)
    , from(-1), handled(false)
  {
  }

  int from;

  bool handled;
  bz_ApiString message;
};

class BZF_API bz_GetPlayerSpawnPosEventData_V1 : public bz_EventData
{
public:
  bz_GetPlayerSpawnPosEventData_V1() : bz_EventData(bz_eGetPlayerSpawnPosEvent)
    , playerID(-1), team(eNoTeam), handled(false)
    , rot(0.0)
  {
    pos[0] = pos[1] = pos[2] = 0.0f;
  }

  int playerID;
  bz_eTeamType team;

  bool handled;

  float pos[3];
  float rot;
};

class BZF_API bz_AllowPlayerEventData_V1 : public bz_EventData
{
public:
  bz_AllowPlayerEventData_V1() : bz_EventData(bz_eAllowPlayer)
    , playerID(-1)
    , allow(true)
  {
  }

  int playerID;
  bz_ApiString callsign;
  bz_ApiString ipAddress;

  bz_ApiString reason;
  bool allow;
};

class BZF_API bz_TickEventData_V1 : public bz_EventData
{
public:
  bz_TickEventData_V1() : bz_EventData(bz_eTickEvent)
  {
  }
};

class BZF_API bz_GetWorldEventData_V1 : public bz_EventData
{
public:
  bz_GetWorldEventData_V1() : bz_EventData(bz_eGetWorldEvent)
    , generated(false)
    , ctf(false)
    , rabbit(false)
    , openFFA(false)
    , worldBlob(NULL)
  {
  }

  bool generated;
  bool ctf;
  bool rabbit;
  bool openFFA;

  bz_ApiString worldFile;
  char* worldBlob; // if assigned, the world will be read from this NUL
  // terminated string. BZFS does not free this memory,
  // so the plugin must do so (this can be done in the
  // WorldFinalize event)
};

class BZF_API bz_GetPlayerInfoEventData_V1 : public bz_EventData
{
public:
  bz_GetPlayerInfoEventData_V1() : bz_EventData(bz_eGetPlayerInfoEvent)
    , playerID(-1), team(eNoTeam)
    , admin(false), verified(false), registered(false)
  {
  }

  int playerID;
  bz_ApiString callsign;
  bz_ApiString ipAddress;

  bz_eTeamType team;

  bool admin;
  bool verified;
  bool registered;
};

class BZF_API bz_GetAutoTeamEventData_V1 : public bz_EventData
{
public:
  bz_GetAutoTeamEventData_V1() : bz_EventData(bz_eGetAutoTeamEvent)
    , playerID(-1), team(eNoTeam)
    , handled(false)
  {
  }

  int playerID;
  bz_ApiString callsign;
  bz_eTeamType team;

  bool handled;
};

class BZF_API bz_AllowSpawnData_V1 : public bz_EventData
{
public:
  bz_AllowSpawnData_V1() : bz_EventData(bz_eAllowSpawn)
    , playerID(-1), team(eNoTeam)
    , handled(false), allow(true)
  {
  }

  int playerID;
  bz_eTeamType team;

  bool handled;
  bool allow;
};

class BZF_API bz_ListServerUpdateEvent_V1 : public bz_EventData
{
public:
  bz_ListServerUpdateEvent_V1() : bz_EventData(bz_eListServerUpdateEvent)
    , handled(false)
  {
  }

  bz_ApiString address;
  bz_ApiString description;
  bz_ApiString groups;

  bool handled;
};

class BZF_API bz_BanEventData_V1 : public bz_EventData
{
public:
  bz_BanEventData_V1() : bz_EventData(bz_eBanEvent)
    , bannerID(-1), banneeID(-1), duration(-1)
  {
  }

  int bannerID;
  int banneeID;
  int duration;
  bz_ApiString ipAddress;
  bz_ApiString reason;
};

class BZF_API bz_HostBanEventData_V1 : public bz_EventData
{
public:
  bz_HostBanEventData_V1() : bz_EventData(bz_eHostBanModifyEvent)
    , bannerID(-1), duration(-1)
  {
  }

  int bannerID;
  int duration;
  bz_ApiString hostPattern;
  bz_ApiString reason;
};

class BZF_API bz_KickEventData_V1 : public bz_EventData
{
public:
  bz_KickEventData_V1() : bz_EventData(bz_eKickEvent)
    , kickerID(-1), kickedID(-1)
  {
  }

  int kickerID;
  int kickedID;
  bz_ApiString reason;
};

class BZF_API bz_KillEventData_V1 : public bz_EventData
{
public:
  bz_KillEventData_V1() : bz_EventData(bz_eKillEvent)
    , killerID(-1), killedID(-1)
  {
  }

  int killerID;
  int killedID;
  bz_ApiString reason;
};

class BZF_API bz_PlayerPausedEventData_V1 : public bz_EventData
{
public:
  bz_PlayerPausedEventData_V1() : bz_EventData(bz_ePlayerPausedEvent)
    , playerID(-1), pause(false)
  {
  }

  int playerID;
  bool pause;
};

class BZF_API bz_MessageFilteredEventData_V1 : public bz_EventData
{
public:
  bz_MessageFilteredEventData_V1() : bz_EventData(bz_eMessageFilteredEvent)
    , playerID(-1)
  {
  }

  int playerID;

  bz_ApiString rawMessage;
  bz_ApiString filteredMessage;
};

class BZF_API bz_GameStartEndEventData_V1 : public bz_EventData
{
public:
  bz_GameStartEndEventData_V1() : bz_EventData(bz_eGameStartEvent)
    , duration(0.0)
  {
  }

  double duration;
};

class BZF_API bz_SlashCommandEventData_V1 : public bz_EventData
{
public:
  bz_SlashCommandEventData_V1() : bz_EventData(bz_eSlashCommandEvent)
    , from(-1)
  {
  }

  int from;

  bz_ApiString message;
};

class BZF_API bz_PlayerAuthEventData_V1 : public bz_EventData
{
public:
  bz_PlayerAuthEventData_V1() : bz_EventData(bz_ePlayerAuthEvent)
    , playerID(-1), password(false), globalAuth(false)
  {
  }

  int playerID;
  bool password;
  bool globalAuth;
};

class BZF_API bz_ServerMsgEventData_V1 : public bz_EventData
{
public:
  bz_ServerMsgEventData_V1() : bz_EventData(bz_eServerMsgEvent)
    , to(-1), team(eNoTeam)
  {
  }

  int to;
  bz_eTeamType team;
  bz_ApiString message;
};

class BZF_API bz_ShotFiredEventData_V1 : public bz_EventData
{
public:
  bz_ShotFiredEventData_V1() : bz_EventData(bz_eShotFiredEvent)
    , changed(false)
    , playerID(-1)
    , shotID(-1)
  {
    pos[0] = pos[1] = pos[2] = 0.0f;
    vel[0] = vel[1] = vel[2] = 0.0f;
  }

  bool changed;
  float pos[3];
  float vel[3];
  bz_ApiString type;
  int playerID;
  int shotID;
};

class BZF_API bz_PlayerUpdateEventData_V1 : public bz_EventData
{
public:
  bz_PlayerUpdateEventData_V1() : bz_EventData(bz_ePlayerUpdateEvent)
    , playerID(-1), stateTime(0.0)
  {
  }

  int playerID;
  bz_PlayerUpdateState state;
  double stateTime;
};

class BZF_API bz_NetTransferEventData_V1 : public bz_EventData
{
public:
  bz_NetTransferEventData_V1() : bz_EventData(bz_eNetDataReceiveEvent)
    , send(false), udp(false), iSize(0), playerID(-1)
    , data(NULL)
  {
  }

  bool send;
  bool udp;
  unsigned int iSize;
  int playerID;

  // DON'T CHANGE THIS!!!
  unsigned char* data;
};

class BZF_API bz_LoggingEventData_V1 : public bz_EventData
{
public:
  bz_LoggingEventData_V1() : bz_EventData(bz_eLoggingEvent)
    , level(0)
  {
  }

  int level;
  bz_ApiString message;
};

class BZF_API bz_ShotEndedEventData_V1 : public bz_EventData
{
public:

  bz_ShotEndedEventData_V1() : bz_EventData(bz_eShotEndedEvent)
    , playerID(-1)
    , shotID(-1)
    , explode(false)
  {
  }

  int playerID;
  int shotID;
  bool explode;
};

class BZF_API bz_FlagTransferredEventData_V1 : public bz_EventData
{
public:
  enum Action {
    ContinueSteal = 0,
    CancelSteal = 1,
    DropThief = 2
  };

  bz_FlagTransferredEventData_V1() : bz_EventData(bz_eFlagTransferredEvent)
    , fromPlayerID(0), toPlayerID(0), flagType(NULL), action(ContinueSteal)
  {
  }

  int fromPlayerID;
  int toPlayerID;
  const char* flagType;
  enum Action action;
};

class BZF_API bz_FlagGrabbedEventData_V1 : public bz_EventData
{
public:

  bz_FlagGrabbedEventData_V1() : bz_EventData(bz_eFlagGrabbedEvent)
    , playerID(-1), flagID(-1)
    , flagType(NULL)
  {
    pos[0] = pos[1] = pos[2] = 0;
  }

  int playerID;
  int flagID;

  const char* flagType;
  float pos[3];
};

class BZF_API bz_FlagDroppedEventData_V1 : public bz_EventData
{
public:

  bz_FlagDroppedEventData_V1() : bz_EventData(bz_eFlagDroppedEvent)
    , playerID(-1), flagID(-1), flagType(NULL)
  {
    pos[0] = pos[1] = pos[2] = 0;
  }

  int playerID;
  int flagID;

  const char* flagType;
  float pos[3];
};

class BZF_API bz_AllowCTFCaptureEventData_V1 : public bz_EventData
{
 public:
  bz_AllowCTFCaptureEventData_V1() : bz_EventData(bz_eAllowCTFCaptureEvent)
    , teamCapped(eNoTeam), teamCapping(eNoTeam), playerCapping(-1)
    , rot(0.0)
    , allow(false), killTeam(true)
    {
    }

  bz_eTeamType teamCapped;
  bz_eTeamType teamCapping;
  int playerCapping;

  float pos[3];
  float rot;

  bool allow;
  bool killTeam;
};

class BZF_API bz_MsgDebugEventData_V1 : public bz_EventData
{
public:
  bz_MsgDebugEventData_V1(): bz_EventData(bz_eMsgDebugEvent)
    , receive(true)
    , playerID(-1)
  {
  }

  char code[2];
  size_t len;
  unsigned char* msg;

  bool	    receive;
  int playerID;
};

class BZF_API bz_NewNonPlayerConnectionEventData_V1 : public bz_EventData
{
public:

  bz_NewNonPlayerConnectionEventData_V1() : bz_EventData(bz_eNewNonPlayerConnection)
    , connectionID(-1)
    , data(0), size(0)
  {
  }

  int connectionID;
  void* data;
  unsigned int size;
};

// plug-in registration

class BZF_API bz_Plugin
{
public:
  bz_Plugin();
  virtual ~bz_Plugin();

  virtual const char* Name () = 0;
  virtual void Init ( const char* config) = 0;
  virtual void Cleanup (){Flush();}
  virtual void Event ( bz_EventData *eventData ){return;}

  // used for inter plugin communication
  virtual void GeneralCallback ( const char* name, void * data){return;}

  float MaxWaitTime;

protected:
  bool Register (bz_eEventType eventType);
  bool Remove (bz_eEventType eventType);
  void Flush ();
};

BZF_API bool bz_pluginExists(const char* name);
BZF_API bz_Plugin* bz_getPlugin(const char* name);

// non player data handlers
class bz_NonPlayerConnectionHandler
{
public:
  virtual ~bz_NonPlayerConnectionHandler() {}
  virtual void pending(int connectionID, void *data, unsigned int size) = 0;
  virtual void disconnect(int connectionID) { if (connectionID) return; }
};

BZF_API bool bz_registerNonPlayerConnectionHandler(int connectionID, bz_NonPlayerConnectionHandler* handler);
BZF_API bool bz_removeNonPlayerConnectionHandler(int connectionID, bz_NonPlayerConnectionHandler* handler);
BZF_API bool bz_sendNonPlayerData(int connectionID, const void *data, unsigned int size);
BZF_API bool bz_disconnectNonPlayerConnection(int connectionID);
BZF_API unsigned int bz_getNonPlayerConnectionOutboundPacketCount(int connectionID);

BZF_API const char* bz_getNonPlayerConnectionIP(int connectionID);
BZF_API const char* bz_getNonPlayerConnectionHost(int connectionID);

// player info

class bz_BasePlayerRecord;

BZF_API bool bz_getPlayerIndexList ( bz_APIIntList *playerList );
BZF_API bz_BasePlayerRecord *bz_getPlayerByIndex ( int index );
BZF_API bool bz_updatePlayerData ( bz_BasePlayerRecord *playerRecord );
BZF_API bool bz_hasPerm ( int playerID, const char* perm );
BZF_API bool bz_grantPerm ( int playerID, const char* perm );
BZF_API bool bz_revokePerm ( int playerID, const char* perm );

BZF_API const char* bz_getPlayerFlag( int playerID );

BZF_API bool bz_isPlayerPaused( int playerID );

// player lag info
BZF_API int bz_getPlayerLag( int playerId );
BZF_API int bz_getPlayerJitter( int playerId );
BZF_API float bz_getPlayerPacketloss( int playerId );

class BZF_API bz_BasePlayerRecord
{
public:
  bz_BasePlayerRecord()
    : version(1), playerID(-1), team(eNoTeam)
    , currentFlagID(-1)
    , lastUpdateTime(0.0)
    , spawned(false), verified(false), globalUser(false)
    , admin(false), op(false), canSpawn(false)
    , lag(0), jitter(0), packetLoss(0.0)
    , rank(0.0), wins(0), losses(0), teamKills(0)
  {}

  ~bz_BasePlayerRecord() {}

  void update(void) { bz_updatePlayerData(this); } // call to update with current data

  bool hasPerm(const char* perm) { return bz_hasPerm(playerID,perm); }
  bool grantPerm(const char* perm) { return bz_grantPerm(playerID,perm); }
  bool revokePerm(const char* perm) { return bz_revokePerm(playerID,perm); }

  const char *getCustomData ( const char* key);
  bool setCustomData ( const char* key, const char* data);

  int version;
  int playerID;
  bz_ApiString callsign;

  bz_eTeamType team;

  bz_ApiString ipAddress;

  int currentFlagID;
  bz_ApiString currentFlag;
  bz_APIStringList flagHistory;

  double lastUpdateTime;
  bz_PlayerUpdateState lastKnownState;

  bz_ApiString clientVersion;
  bool spawned;
  bool verified;
  bool globalUser;
  bz_ApiString bzID;
  bool admin;
  bool op;
  bool canSpawn;
  bz_APIStringList groups;

  int lag;
  int jitter;
  float packetLoss;

  float rank;
  int wins;
  int losses;
  int teamKills;

};

BZF_API bool bz_setPlayerOperator ( int playerId );

// team info
BZF_API unsigned int bz_getTeamPlayerLimit ( bz_eTeamType team );

// player score
BZF_API bool bz_setPlayerWins (int playerId, int wins);
BZF_API bool bz_setPlayerLosses (int playerId, int losses);
BZF_API bool bz_setPlayerTKs (int playerId, int tks);

BZF_API bool bz_resetPlayerScore(int playerId);

// groups API
BZF_API bz_APIStringList* bz_getGroupList ( void );
BZF_API bz_APIStringList* bz_getGroupPerms ( const char* group );
BZF_API bool bz_groupAllowPerm ( const char* group, const char* perm );

// message API
BZF_API bool bz_sendTextMessage (int from, int to, const char* message);
BZF_API bool bz_sendTextMessage (int from, bz_eTeamType to, const char* message);
BZF_API bool bz_sendTextMessagef(int from, int to, const char* fmt, ...);
BZF_API bool bz_sendTextMessagef(int from, bz_eTeamType to, const char* fmt, ...);
BZF_API bool bz_sentFetchResMessage ( int playerID,  const char* URL );

// world weapons
BZF_API bool bz_fireWorldWep ( const char* flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID , float dt );
BZF_API int bz_fireWorldGM ( int targetPlayerID, float lifetime, float *pos, float tilt, float direction, float dt);

typedef struct
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  bool daylightSavings;
}bz_localTime;

BZF_API void bz_getLocaltime ( bz_localTime	*ts );

// BZDB API
BZF_API double bz_getBZDBDouble ( const char* variable );
BZF_API bz_ApiString bz_getBZDBString( const char* variable );
BZF_API bool bz_getBZDBBool( const char* variable );
BZF_API int bz_getBZDBInt( const char* variable );

BZF_API int bz_getBZDBItemPerms( const char* variable );
BZF_API bool bz_getBZDBItemPesistent( const char* variable );
BZF_API bool bz_BZDBItemExists( const char* variable );

BZF_API bool bz_setBZDBDouble ( const char* variable, double val, int perms = 0, bool persistent = false );
BZF_API bool bz_setBZDBString( const char* variable, const char *val, int perms = 0, bool persistent = false  );
BZF_API bool bz_setBZDBBool( const char* variable, bool val, int perms = 0, bool persistent = false  );
BZF_API bool bz_setBZDBInt( const char* variable, int val, int perms = 0, bool persistent = false  );

BZF_API int bz_getBZDBVarList( bz_APIStringList	*varList );

BZF_API void bz_resetBZDBVar( const char* variable );
BZF_API void bz_resetALLBZDBVars( void );

// logging
BZF_API void bz_debugMessage ( int debugLevel, const char* message );
BZF_API void bz_debugMessagef( int debugLevel, const char* fmt, ... );
BZF_API int bz_getDebugLevel ( void );
BZF_API int bz_setDebugLevel ( int debugLevel );

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify );
BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int time, const char* reason );
BZF_API bool bz_IPUnbanUser ( const char* ip );
BZF_API  bz_APIStringList *bz_getReports( void );

// lagwarn
BZF_API int bz_getLagWarn( void );
BZF_API bool bz_setLagWarn( int lagwarn );

// timelimit
BZF_API bool bz_setTimeLimit( float timeLimit );
BZF_API float bz_getTimeLimit( void );
BZF_API bool bz_isTimeManualStart( void );

// countdown
BZF_API bool bz_isCountDownActive( void );
BZF_API bool bz_isCountDownInProgress( void );

// polls
BZF_API bool bz_pollVeto( void );

// help
BZF_API bz_APIStringList *bz_getHelpTopics( void );
BZF_API bz_APIStringList *bz_getHelpTopic( std::string name );

// custom commands

class bz_CustomSlashCommandHandler
{
public:
  virtual ~bz_CustomSlashCommandHandler(){};
  virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params ) = 0;

};

BZF_API bool bz_registerCustomSlashCommand ( const char* command, bz_CustomSlashCommandHandler *handler );
BZF_API bool bz_removeCustomSlashCommand ( const char* command );

// spawning
BZF_API bool bz_getStandardSpawn ( int playerID, float pos[3], float *rot );

// dying
BZF_API bool bz_killPlayer ( int playerID, bool spawnOnBase, int killerID = -1, const char* flagID = NULL );

// flags
BZF_API bool bz_givePlayerFlag ( int playerID, const char* flagType, bool force );
BZF_API bool bz_removePlayerFlag ( int playerID );
BZF_API void bz_resetFlags ( bool onlyUnused );

BZF_API unsigned int bz_getNumFlags( void );
BZF_API const bz_ApiString bz_getName( int flag );
BZF_API bool bz_resetFlag ( int flag );
BZF_API bool bz_moveFlag ( int flag, float pos[3] );
BZF_API int bz_flagPlayer ( int flag );
BZF_API bool bz_getFlagPosition ( int flag, float* pos );

// world
typedef struct
{
  bool	driveThru;
  bool	shootThru;
}bz_WorldObjectOptions;

typedef struct
{
  bz_ApiString		texture;
  bool		useAlpha;
  bool		useColorOnTexture;
  bool		useSphereMap;
  int			combineMode;
}bz_MaterialTexture;

class BZF_API bzAPITextureList
{
public:
  bzAPITextureList();
  bzAPITextureList(const bzAPITextureList	&r);

  ~bzAPITextureList();

  void push_back ( bz_MaterialTexture &value );
  bz_MaterialTexture get ( unsigned int i );

  const bz_MaterialTexture& operator[] (unsigned int i) const;
  bzAPITextureList& operator = ( const bzAPITextureList& r );

  unsigned int size ( void );
  void clear ( void );

protected:
  class dataBlob;

  dataBlob *data;
};

typedef struct bz_MaterialInfo
{
  bz_ApiString name;
  bzAPITextureList textures;

  float		ambient[4];
  float		diffuse[4];
  float		specular[4];
  float		emisive[4];
  float		shine;

  float		alphaThresh;
  bool		culling;
  bool		sorting;
}bz_MaterialInfo;

// have bz make you a new material
bz_MaterialInfo* bz_anewMaterial ( void );
// tell bz you are done with a material
void bz_deleteMaterial ( bz_MaterialInfo *material );

BZF_API bool bz_addWorldBox ( float *pos, float rot, float* scale, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldPyramid ( float *pos, float rot, float* scale, bool fliped, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldBase( float *pos, float rot, float* scale, bz_eTeamType team, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldTeleporter ( float *pos, float rot, float* scale, float border, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldWaterLevel( float level, bz_MaterialInfo *material );
BZF_API bool bz_addWorldWeapon( const char* flagType, float *pos, float rot, float tilt, float initDelay, bz_APIFloatList &delays );

BZF_API bool bz_setWorldSize( float size, float wallHeight = -1.0 );
BZF_API void bz_setClientWorldDownloadURL( const char* URL );
BZF_API const bz_ApiString bz_getClientWorldDownloadURL( void );
BZF_API bool bz_saveWorldCacheFile( const char* file );


// custom map objects

typedef struct bz_CustomMapObjectInfo
{
  bz_ApiString name;
  bz_APIStringList data;
}bz_CustomMapObjectInfo;

class bz_CustomMapObjectHandler
{
public:
  virtual ~bz_CustomMapObjectHandler(){};
  virtual bool handle ( bz_ApiString object, bz_CustomMapObjectInfo *data ) = 0;

};

BZF_API bool bz_registerCustomMapObject ( const char* object, bz_CustomMapObjectHandler *handler );
BZF_API bool bz_removeCustomMapObject ( const char* object );


// public server info
BZF_API bool bz_getPublic( void );
BZF_API bz_ApiString bz_getPublicAddr( void );
BZF_API bz_ApiString bz_getPublicDescription( void );

// custom client sounds
BZF_API bool bz_sendPlayCustomLocalSound ( int playerID, const char* soundName );

class bz_APIPluginHandler
{
public:
  virtual ~bz_APIPluginHandler(){};
  virtual bool handle ( bz_ApiString plugin, bz_ApiString param ) = 0;
};
// custom pluginHandler
BZF_API bool bz_registerCustomPluginHandler ( const char* extension, bz_APIPluginHandler * handler );
BZF_API bool bz_removeCustomPluginHandler ( const char* extension, bz_APIPluginHandler * handler );

// team info
BZF_API int bz_getTeamCount (bz_eTeamType team );
BZF_API int bz_getTeamScore (bz_eTeamType team );
BZF_API int bz_getTeamWins (bz_eTeamType team );
BZF_API int bz_getTeamLosses (bz_eTeamType team );

BZF_API void bz_setTeamWins (bz_eTeamType team, int wins );
BZF_API void bz_setTeamLosses (bz_eTeamType team, int losses );

BZF_API void bz_resetTeamScore (bz_eTeamType team );
BZF_API void bz_resetTeamScores ( void );

// list server
BZF_API void bz_updateListServer ( void );

// url API
class bz_URLHandler
{
public:
  virtual ~bz_URLHandler(){};
  virtual void done ( const char* URL, void * data, unsigned int size, bool complete ) = 0;
  virtual void timeout ( const char* /*URL*/, int /*errorCode*/ ){};
  virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ ){};
};

BZF_API bool bz_addURLJob ( const char* URL, bz_URLHandler* handler = NULL, const char* postData = NULL );
BZF_API bool bz_removeURLJob ( const char* URL );
BZF_API bool bz_stopAllURLJobs ( void );

// inter plugin communication
BZF_API bool bz_clipFieldExists ( const char *name );
BZF_API const char* bz_getclipFieldString ( const char *name );
BZF_API float bz_getclipFieldFloat ( const char *name );
BZF_API int bz_getclipFieldInt( const char *name );

BZF_API bool bz_setclipFieldString ( const char *name, const char* data );
BZF_API bool bz_setclipFieldFloat ( const char *name, float data );
BZF_API bool bz_setclipFieldInt( const char *name, int data );

class bz_ClipFieldNotifier
{
public:
  virtual ~bz_ClipFieldNotifier(){};
  virtual void fieldChange ( const char* /*field*/) = 0;
};

BZF_API bool bz_addClipFieldNotifier ( const char *name, bz_ClipFieldNotifier *cb );
BZF_API bool bz_removeClipFieldNotifier ( const char *name, bz_ClipFieldNotifier *cb );

// path checks
BZF_API bz_ApiString bz_filterPath ( const char* path );

// Record-Replay
BZF_API bool bz_saveRecBuf( const char * _filename, int seconds  = 0);
BZF_API bool bz_startRecBuf( void );
BZF_API bool bz_stopRecBuf( void );

// cheap Text Utils
BZF_API const char *bz_format(const char* fmt, ...);
BZF_API const char *bz_toupper(const char* val );
BZF_API const char *bz_tolower(const char* val );
BZF_API const char *bz_urlEncode(const char* val );

// game countdown
BZF_API void bz_pauseCountdown ( const char *pausedBy );
BZF_API void bz_resumeCountdown ( const char *resumedBy );
BZF_API void bz_startCountdown ( int delay, float limit, const char *byWho );

// server control
BZF_API void bz_shutdown();
BZF_API void bz_superkill();
BZF_API void bz_gameOver(int playerID, bz_eTeamType team = eNoTeam);
BZF_API bool bz_restart ( void );

BZF_API void bz_reloadLocalBans();
BZF_API void bz_reloadMasterBans();
BZF_API void bz_reloadGroups();
BZF_API void bz_reloadUsers();
BZF_API void bz_reloadHelp();

// info about the world
BZF_API bz_eTeamType bz_checkBaseAtPoint ( float pos[3] );

bz_eTeamType convertTeam ( int team );
int convertTeam( bz_eTeamType team );

// game type info
BZF_API	bz_eGameType bz_getGameType ( void  );

#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
