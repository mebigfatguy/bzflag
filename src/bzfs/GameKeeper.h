/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __GAMEKEEPER_H__
#define __GAMEKEEPER_H__

// bzflag global header
#include "common.h"

// system headers
#include <iostream>
#include <vector>
#include <string>
#if defined(USE_THREADS)
#  include <pthread.h>
#endif

// common interface headers
#include "PlayerInfo.h"
#include "PlayerState.h"
#include "TimeKeeper.h"
#include "bzfsAPI.h"

// implementation-specific bzfs-specific headers
#include "CmdLineOptions.h"
#include "FlagHistory.h"
#include "Permissions.h"
#include "LagInfo.h"
#include "Score.h"
#include "RecordReplay.h"
#include "NetHandler.h"
#include "Authentication.h"
#include "messages.h"
#include "ShotUpdate.h"
#include "BufferedNetworkMessage.h"

class PlayerCaps
{
public:
	bool	canDownloadResources;
	bool	canPlayRemoteSounds;

	PlayerCaps ( void )
	{
		canPlayRemoteSounds = false;
		canDownloadResources = false;
	}
};

const int PlayerSlot = MaxPlayers + ReplayObservers;

typedef void (*tcpCallback)(NetHandler &netPlayer, int i, const RxStatus e);

/** This class is meant to be the container of all the global entity that lives
    into the game and methods to act globally on those.
    Up to now it contain players. Flag class is only there as a TODO
*/
class GameKeeper {
public:
  class Player {
  public:
    Player(int _playerIndex, NetHandler *_netHandler, tcpCallback _clientCallback);
    Player(int _playerIndex, bz_ServerSidePlayerHandler *handler);
    ~Player();

    int		   getIndex();
    static int     getFreeIndex(int min, int max);
    static Player *getPlayerByIndex(int _playerIndex);
    static Player *getFirstPlayer(NetHandler *_netHandler);
    static int     count();
    static void    updateLatency(float &waitTime);
    static void    dumpScore();
    static int     anointRabbit(int oldRabbit);
    static std::vector<int> allowed(PlayerAccessInfo::AccessPerm right,
				    int targetPlayer = -1);
    static int     getPlayerIDByName(const std::string &name);
    static void    reloadAccessDatabase();

    bool	   loadEnterData(uint16_t &rejectCode,
				 char *rejectMsg);
    void	  *packAdminInfo(void *buf);
    void	  packAdminInfo(BufferedNetworkMessage *msg);
    void	  *packPlayerInfo(void *buf);
    void	  packPlayerInfo(BufferedNetworkMessage *msg);
    void	  *packPlayerUpdate(void *buf);
    void	  packPlayerUpdate(BufferedNetworkMessage *msg);

    void	  setPlayerAddMessage ( PlayerAddMessage &msg );

    void	   signingOn(bool ctf);
    void	   close();
    static bool    clean();
    void	   handleTcpPacket(fd_set *set);
#if defined(USE_THREADS)
    void	   handleTcpPacketT();
#endif
    static void    passTCPMutex();
    static void    freeTCPMutex();

    // For hostban checking, to avoid check and check again
    static void    setAllNeedHostbanChecked(bool set);
    void	   setNeedThisHostbanChecked(bool set);
    bool	   needsHostbanChecked();

    // To handle player State
    void	   setPlayerState(float pos[3], float azimuth);
    void	   getPlayerState(float pos[3], float &azimuth);
    void	   setPlayerState(PlayerState state, float timestamp);

    void	   getPlayerCurrentPosRot(float pos[3], float &rot);

    void	   setBzIdentifier(const std::string& id);
    const std::string& getBzIdentifier() const;

	// to handle updating current postions;

    // When is the player's next GameTime?
    const TimeKeeper&	getNextGameTime() const;
    void		updateNextGameTime();

    // To handle shot
    static void    setShotSlots ( size_t maxShots );

    bool	   canShoot ( void );

    bool	   addShot ( int GUID, int localID, double startTime );
    bool	   removeShot ( int GUID );

    // find a shot based on a local ID
    int		   findShot ( int localID );

    // To handle Identify
    void    setLastIdFlag(int _idFlag);
    int	    getLastIdFlag();

    // to handle movement
    float getRealSpeed ( float input );

    // spawnability
    bool isSpawnable ( void ) {return canSpawn;}
    void setSpawnable ( bool spawn ) {canSpawn = spawn;}

    enum LSAState {
      start,
      notRequired,
      required,
      requesting,
      checking,
      timedOut,
      failed,
      verified,
      done
    } _LSAState;

    // players
    PlayerInfo	      player;
    // Net Handler
    NetHandler       *netHandler;
    // player lag info
    LagInfo	      lagInfo;
    // player access
    PlayerAccessInfo  accessInfo;
    // Last known position, vel, etc
    PlayerState       lastState;
    float	      stateTimeStamp;

    ShotType	      effectiveShotType;

    bool	      canSpawn;

   class StateDRRecord
    {
    public:
      StateDRRecord(Player*p);

      float	      pos[3];
      float	      vel[3];
      float	      rot;
      float	      angVel;
    };

   // does the meat of the DR but does not update a state
    void doDRLogic ( float delta,  StateDRRecord &drData );

    // calls doDRLogic and updates to the state
    void doPlayerDR ( float time = (float)TimeKeeper::getCurrent().getSeconds() );
    float	      currentPos[3];
    float	      curentVel[3];
    float	      currentRot;
    float	      currentAngVel;

    PlayerState	getCurrentStateAsState ( void );

    void*	packCurrentState (void* buf, uint16_t& code, bool increment);

    // GameTime update
    float	      gameTimeRate;
    TimeKeeper	      gameTimeNext;
    // FlagHistory
    FlagHistory       flagHistory;
    // Score
    Score	      score;
    // Authentication
    Authentication    authentication;
    // Capabilities
    PlayerCaps			caps;

    // flag to let us know the player is on it's way out
    bool isParting;

    // logic class for server side players
    bz_ServerSidePlayerHandler	*playerHandler;

  private:
    static Player    *playerList[PlayerSlot];
    int		      playerIndex;
    bool	      closed;
    tcpCallback       clientCallback;
    std::string	      bzIdentifier;
#if defined(USE_THREADS)
    pthread_t		   thread;
    static pthread_mutex_t mutex;
    int			   refCount;
#endif
    bool	      needThisHostbanChecked;
    // In case you want recheck all condition on all players
    static bool       allNeedHostbanChecked;

    // shot tracking
    class ShotSlot {
    public:
      ShotSlot() : present(false) {};

      float      expireTime;
      bool       present;
      bool       running;
    };

    std::vector<ShotSlot> shotSlots;

    // list of the live shots, and the temp IDs they used when they were shot
    std::map<int,int>	  liveShots;

    int			    idFlag;
    TimeKeeper		    agilityTime;
  };

  class Flag {
  };
};

inline int GameKeeper::Player::getIndex()
{
  return playerIndex;
}

inline GameKeeper::Player *GameKeeper::Player::getPlayerByIndex(int
								_playerIndex)
{
  if (_playerIndex < 0 || _playerIndex >= PlayerSlot)
    return NULL;
  if (!playerList[_playerIndex])
    return NULL;
  if (playerList[_playerIndex]->closed)
    return NULL;
  return playerList[_playerIndex];
}

void *PackPlayerInfo(void *buf, int playerIndex, uint8_t properties );
void PackPlayerInfo(BufferedNetworkMessage *msg, int playerIndex, uint8_t properties );

#if defined(USE_THREADS)
inline void GameKeeper::Player::handleTcpPacket(fd_set *)
{
return;
}
#endif

inline void GameKeeper::Player::passTCPMutex()
{
#if defined(USE_THREADS)
  int result = pthread_mutex_lock(&mutex);
  if (result)
    std::cerr << "Could not lock mutex" << std::endl;
#endif
}

inline void GameKeeper::Player::freeTCPMutex()
{
#if defined(USE_THREADS)
  int result = pthread_mutex_unlock(&mutex);
  if (result)
    std::cerr << "Could not unlock mutex" << std::endl;
#endif
}

// For hostban checking, to avoid check and check again
inline void GameKeeper::Player::setAllNeedHostbanChecked(bool set)
{
  allNeedHostbanChecked = set;
}

inline void GameKeeper::Player::setNeedThisHostbanChecked(bool set)
{
  needThisHostbanChecked = set;
}

inline bool GameKeeper::Player::needsHostbanChecked()
{
  return (allNeedHostbanChecked || needThisHostbanChecked);
}


inline void GameKeeper::Player::setBzIdentifier(const std::string& id)
{
  bzIdentifier = id;
}

inline const std::string& GameKeeper::Player::getBzIdentifier() const
{
  return bzIdentifier;
}


inline const TimeKeeper& GameKeeper::Player::getNextGameTime() const
{
  return gameTimeNext;
}


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
