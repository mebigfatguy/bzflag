/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "playing.h"
#include "botplaying.h"

// system includes
#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#  include <shlobj.h>
#  include <direct.h>
#else
#  include <pwd.h>
#  include <dirent.h>
#  include <utime.h>
#endif

// common headers
#include "AccessList.h"
#include "AnsiCodes.h"
#include "AresHandler.h"
#include "AutoHunt.h"
#include "BaseBuilding.h"
#include "BZDBCache.h"
#include "BzfMedia.h"
#include "bzsignal.h"
#include "CacheManager.h"
#include "CollisionManager.h"
#include "CommandsStandard.h"
#include "DirectoryNames.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include "GameTime.h"
#include "KeyManager.h"
#include "LinkManager.h"
#include "ObstacleList.h"
#include "ObstacleMgr.h"
#include "PhysicsDriver.h"
#include "PlatformFactory.h"
#include "ServerList.h"
#include "TextUtils.h"
#include "TimeBomb.h"
#include "TimeKeeper.h"
#include "WordFilter.h"
#include "bz_md5.h"
#include "vectors.h"
#include "version.h"

// common client headers
#include "ClientIntangibilityManager.h"
#include "FlashClock.h"
#include "LocalPlayer.h"
#include "Roaming.h"
#include "RobotPlayer.h"
#include "Roster.h"
#include "ShotStats.h"
#include "SyncClock.h"
#include "WorldBuilder.h"
#include "WorldPlayer.h"
#include "World.h"

// local implementation headers
#include "Downloads.h"
#include "Frontend.h"
#include "Logger.h"
#include "RCLinkBackend.h"
#include "RCMessageFactory.h"
#include "RCRobotPlayer.h"
#include "ScriptLoaderFactory.h"

#include "bzflag.h"
#include "commands.h"
#include "motd.h"


bool headless = true;


static const float FlagHelpDuration = 60.0f;
static World *world = NULL;
static Team *teams = NULL;
static bool joinRequested = false;
static bool waitingDNS = false;
static bool serverError = false;
static bool serverDied = false;
static double epochOffset;
static double lastEpochOffset;
static std::vector<PlayingCallbackItem> playingCallbacks;

static FlashClock pulse;
static bool justJoined = false;

static LocalPlayer *myTank = NULL;

static MessageOfTheDay *motd = NULL;

RCLinkBackend *rcLink = NULL;


static void setTankFlags();
void handleMsgSetVars(void *msg);
void handleFlagTransferred(Player *fromTank, Player *toTank, int flagIndex, ShotType shotType);
static void enteringServer(void *buf);
static void joinInternetGame2();
static void cleanWorldCache();
static void markOld(std::string &fileName);
#ifdef ROBOT
static void setRobotTarget(RobotPlayer *robot);
#endif
static bool gotBlowedUp(BaseLocalPlayer *tank,
			BlowedUpReason reason,
			PlayerId killer,
			const ShotPath *hit = NULL,
			int physicsDriver = -1);

static double userTimeEpochOffset;

static bool entered = false;
static bool joiningGame = false;
static WorldBuilder *worldBuilder = NULL;
static std::string worldUrl;
static std::string worldHash;
static std::string worldCachePath;
static std::string md5Digest;
static uint32_t worldPtr = 0;
static char *worldDatabase = NULL;
static bool isCacheTemp;
static std::ostream *cacheOut = NULL;
static bool downloadingData = false;

static AresHandler ares;

static AccessList serverAccessList("ServerAccess.txt", NULL);

static const char AutoJoinContent[] = // FIXME
  "#\n"
  "# This file controls the auto-join confirmation requirements.\n"
  "# Patterns are attempted in order against both the hostname\n"
  "# and ip. The first matching pattern sets the state. If no\n"
  "# patterns are matched, then the server is authorized. There\n"
  "# are four types of matches:\n"
  "#\n"
  "#   simple globbing (* and ?)\n"
  "#     allow\n"
  "#     deny\n"
  "#\n"
  "#   regular expressions\n"
  "#     allow_regex\n"
  "#     deny_regex\n"
  "#\n"
  "\n"
  "#\n"
  "# To authorize all servers, remove the last 3 lines.\n"
  "#\n"
  "\n"
  "allow *.bzflag.bz\n"
  "allow *.bzflag.org\n"
  "deny *\n";
static AccessList autoJoinAccessList("AutoJoinAccess.txt", AutoJoinContent);


ThirdPersonVars::ThirdPersonVars()
{
}

ThirdPersonVars::~ThirdPersonVars()
{
}

void ThirdPersonVars::load(void)
{
}

void ThirdPersonVars::clear(void)
{
}

void ThirdPersonVars::bzdbCallback(const std::string& /*name*/, void */*data*/)
{
}

ThirdPersonVars thirdPersonVars;

#ifdef ROBOT
static void makeObstacleList();
static std::vector<BzfRegion*> obstacleList;  // for robots
#endif

// to simplify code shared between bzrobots and bzflag
// - in bzrobots, this just goes to the error console
void showError(const char *msg, bool flush)
{
  printError(msg);
  if(flush) {
    ;
  }
}


void printout(const std::string& line)
{
#ifdef _WIN32
  FILE *fp = fopen ("stdout.txt", "a+");
  if (fp) {
    fprintf(fp,"%s\n", stripAnsiCodes(line.c_str()));
    fclose(fp);
  }
#else
  BACKENDLOGGER << stripAnsiCodes(line.c_str()) << std::endl;
#endif
}

// - in bzrobots, this shows the error on the console
void showMessage(const std::string& line)
{
  printout(line);
}


void showMessage(const std::string& line, ControlPanel::MessageModes)
{
  printout(line);
}
// access silencePlayers from bzflag.cxx
std::vector<std::string> &getSilenceList()
{
  return silencePlayers;
}

void selectNextRecipient (bool, bool)
{
}
bool shouldGrabMouse()
{
  return false;
}

void warnAboutMainFlags()
{
}


void warnAboutRadarFlags()
{
}


void warnAboutRadar()
{
}


void warnAboutConsole()
{
}


BzfDisplay*		getDisplay()
{
  return NULL;
}

MainWindow*		getMainWindow()
{
  return NULL;
}

RadarRenderer* getRadarRenderer()
{
  return NULL;
}


void forceControls(bool, float, float)
{
}


void setSceneDatabase()
{
}

StartupInfo* getStartupInfo()
{
  return &startupInfo;
}


bool setVideoFormat(int, bool)
{
  return false;
}

void addShotExplosion(const fvec3&)
{
}

void addShotPuff(const fvec3&, const fvec3&)
{
}


void notifyBzfKeyMapChanged()
{
}

void dumpResources()
{
}


void setTarget()
{
}


void addPlayingCallback(PlayingCallback cb, void *data)
{
  PlayingCallbackItem item;
  item.cb = cb;
  item.data = data;
  playingCallbacks.push_back(item);
}

void removePlayingCallback(PlayingCallback _cb, void *data)
{
  std::vector<PlayingCallbackItem>::iterator it = playingCallbacks.begin();
  while (it != playingCallbacks.end()) {
    if (it->cb == _cb && it->data == data) {
      playingCallbacks.erase(it);
      break;
    }
    it++;
  }
}

static void callPlayingCallbacks()
{
  const size_t count = playingCallbacks.size();
  for (size_t i = 0; i < count; i++) {
    const PlayingCallbackItem &cb = playingCallbacks[i];
    (*cb.cb)(cb.data);
  }
}

void joinGame()
{
  if (joiningGame) {
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    if (worldDatabase) {
      delete[] worldDatabase;
      worldDatabase = NULL;
    }
    showError("Download stopped by user action");
    joiningGame = false;
  }
  joinRequested = true;
}

//
// handle signals that should kill me quickly
//

static void dying(int sig)
{
  bzSignal(sig, SIG_DFL);
  raise(sig);
}

//
// handle signals that should kill me nicely
//

static void suicide(int sig)
{
  bzSignal(sig, SIG_PF(suicide));
  CommandsStandard::quit();
}

//
// handle signals that should disconnect me from the server
//

static void hangup(int sig)
{
  bzSignal(sig, SIG_PF(hangup));
  serverDied = true;
  serverError = true;
}


static void updateNumPlayers()
{
  int i, numPlayers[NumTeams];
  for (i = 0; i < NumTeams; i++)
    numPlayers[i] = 0;
  for (i = 0; i < curMaxPlayers; i++)
    if (remotePlayers[i])
      numPlayers[remotePlayers[i]->getTeam()]++;
}

void updateHighScores()
{
  /* check scores to see if my team and/or have the high score.  change
   * `>= bestScore' to `> bestScore' if you want to share the number
   * one spot. */
  bool anyPlayers = false;
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (remotePlayers[i]) {
      anyPlayers = true;
      break;
    }
  if (!anyPlayers) {
    for (i = 0; i < numRobots; i++)
      if (robots[i]) {
	anyPlayers = true;
	break;
      }
  }
  if (!anyPlayers) {
    return;
  }
}


//
// server message handling
//
static Player* addPlayer(PlayerId id, void* msg, int showMessage)
{
  uint16_t team, type, wins, losses, tks;
  float rank;
  char callsign[CallSignLen];
  msg = nboUnpackUInt16 (msg, type);
  msg = nboUnpackUInt16 (msg, team);
  msg = nboUnpackFloat (msg, rank);
  msg = nboUnpackUInt16 (msg, wins);
  msg = nboUnpackUInt16 (msg, losses);
  msg = nboUnpackUInt16 (msg, tks);
  msg = nboUnpackString (msg, callsign, CallSignLen);

  // Strip any ANSI color codes
  strncpy(callsign, stripAnsiCodes(callsign), 32);

  // id is slot, check if it's empty
  const int i = id;

  // sanity check
  if (i < 0) {
    printError (TextUtils::format ("Invalid player identification (%d)", i));
    std::cerr << "WARNING: invalid player identification when adding player with id "
	      << i << std::endl;
    return NULL;
  }

  if (remotePlayers[i]) {
    // we're not in synch with server -> help! not a good sign, but not fatal.
    printError ("Server error when adding player, player already added");
    std::cerr << "WARNING: player already exists at location with id "
	      << i << std::endl;
    return NULL;
  }

  if (i >= curMaxPlayers) {
    curMaxPlayers = i + 1;
    if (World::getWorld()) {
      World::getWorld()->setCurMaxPlayers(curMaxPlayers);
    }
  }

  // add player
  if (PlayerType (type) == TankPlayer
      || PlayerType (type) == ComputerPlayer
      || PlayerType (type) == ChatPlayer) {
    remotePlayers[i] = new RemotePlayer (id, TeamColor (team), callsign, PlayerType (type));
    remotePlayers[i]->changeScore (rank, short (wins), short (losses), short (tks));
  }

#ifdef ROBOT
  if (PlayerType (type) == ComputerPlayer) {
    for (int j = 0; j < numRobots; j++) {
      if (robots[j] && !strncmp (robots[j]->getCallSign (), callsign, CallSignLen)) {
	robots[j]->setTeam (TeamColor (team));
	robots[j]->changeScore(rank, wins, losses, tks);
	break;
      }
    }
  }
#endif

  // show the message if we don't have the playerlist
  // permission.  if * we do, MsgAdminInfo should arrive
  // with more info.
  if (showMessage && !myTank->hasPlayerList ()) {
    std::string message ("joining as ");
    if (team == ObserverTeam) {
      message += "an observer";
    } else {
      switch (PlayerType (type)) {
      case TankPlayer:
	message += "a tank";
	break;
      case ComputerPlayer:
	message += "a robot tank";
	break;
      default:
	message += "an unknown type";
	break;
      }
    }

    if (!remotePlayers[i]) {
      std::string name (callsign);
      name += ": " + message;
      message = name;
    }
    addMessage (remotePlayers[i], message);
  }
  completer.registerWord(callsign, true /* quote spaces */);

  return remotePlayers[i];
}


static void printIpInfo(const Player *player,
                        const Address &addr,
                        const std::string note)
{
  if (player == NULL) {
    return;
  }

  std::string colorStr;
  if (player->getId() < 200) {
    int color = player->getTeam();
    if (color == RabbitTeam || color < 0 || color > LastColor) {
      // non-teamed, rabbit are white (same as observer)
      color = WhiteColor;
    }
    colorStr = ColorStrings[color];
  } else {
    colorStr = ColorStrings[CyanColor]; // replay observers
  }

  const std::string addrStr = addr.getDotNotation();
  std::string message = ColorStrings[CyanColor]; // default color
  message += "IPINFO: ";
  if (BZDBCache::colorful) { message += colorStr; }
  message += player->getCallSign();
  if (BZDBCache::colorful) { message += ColorStrings[CyanColor]; }
  message += "\t from: ";
  if (BZDBCache::colorful) { message += colorStr; }
  message += addrStr;

  message += ColorStrings[WhiteColor];
  for (int i = 0; i < (17 - (int)addrStr.size()); i++) {
    message += " ";
  }

  message += note;

  printout(message);

  return;
}


static bool removePlayer (PlayerId id)
{
  int playerIndex = lookupPlayerIndex(id);

  if (playerIndex < 0)
    return false;

  Player *p = getPlayerByIndex(playerIndex);

  Address addr;
  std::string msg = "signing off";
  if (!p->getIpAddress(addr)) {
    addMessage(p, "signing off");
  } else {
    msg += " from ";
    msg += addr.getDotNotation();
    p->setIpAddress(addr);
    addMessage(p, msg);
    if (BZDB.evalInt("showips") > 1) {
      printIpInfo (p, addr, "(leave)");
    }
  }

  if (myTank->getRecipient() == p)
    myTank->setRecipient(0);
  if (myTank->getNemesis() == p)
    myTank->setNemesis(0);

  completer.unregisterWord(p->getCallSign());

  delete remotePlayers[playerIndex];
  remotePlayers[playerIndex] = NULL;

  while ((playerIndex >= 0) &&
         (playerIndex+1 == curMaxPlayers) &&
         (remotePlayers[playerIndex] == NULL)) {
    playerIndex--;
    curMaxPlayers--;
  }

  World *_world = World::getWorld();
  if (!_world) {
    return false;
  }

  _world->setCurMaxPlayers(curMaxPlayers);

  updateNumPlayers();

  return true;
}


static bool isCached(char *hexDigest)
{
  std::istream *cachedWorld;
  bool cached    = false;
  worldHash = hexDigest;
  worldCachePath = getCacheDirName();
  worldCachePath += hexDigest;
  worldCachePath += ".bwc";
  if ((cachedWorld = FILEMGR.createDataInStream(worldCachePath, true))) {
    cached = true;
    delete cachedWorld;
  }
  return cached;
}


int curlProgressFunc(void * /*clientp*/,
		     double dltotal, double dlnow,
		     double /*ultotal*/, double /*ulnow*/)
{
  // FIXME: beaucoup cheeze here in the aborting style
  // we should be using async dns and multi-curl

  // run an inner main loop to check if we should abort
  /*
  BzfEvent event;
  if (display->isEventPending() && display->peekEvent(event)) {
    switch (event.type) {
    case BzfEvent::Quit:
      return 1; // terminate the curl call
    case BzfEvent::KeyDown:
      display->getEvent(event); // flush the event
      if (event.keyDown.chr == 27)
	return 1; // terminate the curl call

      break;
    case BzfEvent::KeyUp:
      display->getEvent(event); // flush the event
      break;
    case BzfEvent::MouseMove:
      display->getEvent(event); // flush the event
      break;
    case BzfEvent::Unset:
    case BzfEvent::Map:
    case BzfEvent::Unmap:
    case BzfEvent::Redraw:
    case BzfEvent::Resize:
      // leave the event, it might be important
      break;
    }
  }
  */
  
  // update the status
  double percentage = 0.0;
  if ((int)dltotal > 0)
    percentage = 100.0 * dlnow / dltotal;

  showError(TextUtils::format("%2.1f%% (%i/%i)", percentage, (int) dlnow, (int) dltotal).c_str());

  return 0;
}


static void loadCachedWorld()
{
  // can't get a cache from nothing
  if (worldCachePath.size() == 0) {
    joiningGame = false;
    return;
  }

  // lookup the cached world
  std::istream *cachedWorld = FILEMGR.createDataInStream(worldCachePath, true);
  if (!cachedWorld) {
    showError("World cache files disappeared.  Join canceled",true);
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }

  // status update
  showError("Loading world into memory...",true);

  // get the world size
  cachedWorld->seekg(0, std::ios::end);
  std::streampos size = cachedWorld->tellg();
  unsigned long charSize = std::streamoff(size);

  // load the cached world
  cachedWorld->seekg(0);
  char *localWorldDatabase = new char[charSize];
  if (!localWorldDatabase) {
    showError("Error loading cached world.  Join canceled",true);
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }
  cachedWorld->read(localWorldDatabase, charSize);
  delete cachedWorld;

  // verify
  showError("Verifying world integrity...",true);
  MD5 md5;
  md5.update((unsigned char *)localWorldDatabase, charSize);
  md5.finalize();
  std::string digest = md5.hexdigest();
  if (digest != md5Digest) {
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    delete[] localWorldDatabase;
    showError("Error on md5. Removing offending file.");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }

  // make world
  showError("Preparing world...",true);
  if (!worldBuilder->unpack(localWorldDatabase)) {
    // world didn't make for some reason
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    delete[] localWorldDatabase;
    showError("Error unpacking world database. Join canceled.");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }
  delete[] localWorldDatabase;

  // return world
  if (worldBuilder) {
    world = worldBuilder->getWorld();
    world->setMapHash(worldHash);
    delete worldBuilder;
    worldBuilder = NULL;
  }

  showError("Downloading files...");

  const bool doDownloads = BZDB.isTrue("doDownloads");
  const bool updateDownloads =  BZDB.isTrue("updateDownloads");
  Downloads::instance().startDownloads(doDownloads, updateDownloads, false);
  downloadingData  = true;
}


class WorldDownLoader : private cURLManager {
  public:
    void start(char *hexDigest);

  private:
    void askToBZFS();
    virtual void finalization(char *data, unsigned int length, bool good);
};


void WorldDownLoader::start(char *hexDigest)
{
  if (isCached(hexDigest)) {
    loadCachedWorld();
  } else if (worldUrl.size()) {
    showError(("Loading world from " + worldUrl).c_str());
    setURL(worldUrl);
    addHandle();
    worldUrl = ""; // clear the state
  } else {
    askToBZFS();
  }
}


void WorldDownLoader::finalization(char *data, unsigned int length, bool good)
{
  if (good) {
    worldDatabase = data;
    theData       = NULL;
    MD5 md5;
    md5.update((unsigned char *)worldDatabase, length);
    md5.finalize();
    std::string digest = md5.hexdigest();
    if (digest != md5Digest) {
      showError("Download from URL failed");
      askToBZFS();
    } else {
      std::ostream *cache =
	FILEMGR.createDataOutStream(worldCachePath, true, true);
      if (cache != NULL) {
	cache->write(worldDatabase, length);
	delete cache;
	loadCachedWorld();
      } else {
	showError("Problem writing cache");
	askToBZFS();
      }
    }
  } else {
    askToBZFS();
  }
}


void WorldDownLoader::askToBZFS()
{
  // Why do we use the error status for this?
  showError("Downloading World...");
  char message[MaxPacketLen];
  // ask for world
  nboPackUInt32(message, 0);
  serverLink->send(MsgGetWorld, sizeof(uint32_t), message);
  worldPtr = 0;
  if (cacheOut)
    delete cacheOut;
  cacheOut = FILEMGR.createDataOutStream(worldCachePath, true, true);
}


static WorldDownLoader* worldDownLoader = NULL;


static void dumpMissingFlag(char *buf, uint16_t len)
{
  int i;
  int nFlags = len/2;
  std::string flags;

  for (i = 0; i < nFlags; i++) {
    /* We can't use FlagType::unpack() here, since it counts on the
     * flags existing in our flag database.
     */
    if (i)
      flags += ", ";
    flags += buf[0];
    if (buf[1])
      flags += buf[1];
    buf += 2;
  }

  showError(TextUtils::format("Flags not supported by this client: %s",flags.c_str()).c_str());
}


static bool processWorldChunk(void *buf, uint16_t len, int bytesLeft)
{
  int totalSize = worldPtr + len + bytesLeft;
  int doneSize  = worldPtr + len;
  if (cacheOut)
    cacheOut->write((char *)buf, len);
  showError(TextUtils::format("Downloading World (%2d%% complete/%d kb remaining)...",
    (100 * doneSize / totalSize),bytesLeft / 1024).c_str());
  return bytesLeft == 0;
}


void handleResourceFetch (void *)
{
}


void handleCustomSound(void *)
{
}


void handleJoinServer(void *msg)
{
  // FIXME: MsgJoinServer notes ...
  //        - fix whatever is broken
  //        - add notifications
  //        - add an AutoJoinAccess.txt file to decided
  //          if confirmation queries or required

  std::string addr;
  int32_t port;
  int32_t team;
  std::string referrer;
  std::string message;

  msg = nboUnpackStdString(msg, addr);
  msg = nboUnpackInt32(msg, port);
  msg = nboUnpackInt32(msg, team);
  msg = nboUnpackStdString(msg, referrer);
  msg = nboUnpackStdString(msg, message);

  if (addr.empty()) {
    return;
  }
  if ((port < 0) || (port > 65535)) {
    return;
  }

  if (!BZDB.isTrue("autoJoin")) {
    addMessage(NULL,
      TextUtils::format("ignored autoJoin to %s:%i", addr.c_str(), port)
    );
    return;
  }

  autoJoinAccessList.reload();
  std::vector<std::string> nameAndIp;
  nameAndIp.push_back(addr);
//FIXME  nameAndIp.push_back(serverAddress.getDotNotation());
  if (!autoJoinAccessList.authorized(nameAndIp)) {
    showError("Auto Join Denied Locally");
    std::string warn = ColorStrings[WhiteColor];
    warn += "NOTE: ";
    warn += ColorStrings[GreyColor];
    warn += "auto joining is controlled by ";
    warn += ColorStrings[YellowColor];
    warn += autoJoinAccessList.getFileName();
    addMessage(NULL, warn);
    return;
  }

  StartupInfo& info = startupInfo;

  strncpy(info.serverName, addr.c_str(), ServerNameLen - 1);
  info.serverName[ServerNameLen - 1] = 0;

  strncpy(info.referrer, referrer.c_str(), ReferrerLen - 1);
  info.referrer[ReferrerLen - 1] = 0;

  info.serverPort = port;
  if (team == NoTeam) {
    // leave it alone, player can select using the menu
  } else {
    info.team = (TeamColor)team;
  }

  printf("AutoJoin: %s %u %i \"%s\" \"%s\"\n", // FIXME
         addr.c_str(), port, team, referrer.c_str(), message.c_str());

  joinGame();
}


void handleSuperKill(void *msg)
{
  uint8_t id;
  nboUnpackUInt8(msg, id);
  if (!myTank || myTank->getId() == id || id == 0xff) {
    serverError = true;
    printError("Server forced a disconnect");
#ifdef ROBOT
  } else {
    int i;
    for (i = 0; i < MAX_ROBOTS; i++) {
      if (robots[i] && robots[i]->getId() == id)
	break;
    }
    if (i >= MAX_ROBOTS)
      return;
    delete robots[i];
    robots[i] = NULL;
    numRobots--;
#endif
  }
}


void handleRejectMessage(void *msg)
{
  void *buf;
  char buffer[MessageLen];
  uint16_t rejcode;
  std::string reason;

  buf = nboUnpackUInt16 (msg, rejcode); // filler for now
  buf = nboUnpackString (buf, buffer, MessageLen);
  buffer[MessageLen - 1] = '\0';
  reason = buffer;
  printError(reason);
}


void handleFlagNegotiation(void *msg, uint16_t len)
{
  if (len > 0) {
    dumpMissingFlag((char *)msg, len);
    return;
  }
  serverLink->send(MsgWantSettings, 0, NULL);
}


void handleFlagType(void *msg)
{
FlagType* typ = NULL;
      FlagType::unpackCustom(msg, typ);
      logDebugMessage(1, "Got custom flag type from server: %s\n",
                         typ->information().c_str());
}


void handleGameSettings(void *msg)
{
  if (worldBuilder) {
    delete worldBuilder;
    worldBuilder = NULL;
  }
  worldBuilder = new WorldBuilder;
  worldBuilder->unpackGameSettings(msg);
  serverLink->send(MsgWantWHash, 0, NULL);
  showError("Requesting World Hash...");
}


void handleCacheURL(void *msg, uint16_t len)
{
  char *cacheURL = new char[len];
  nboUnpackString(msg, cacheURL, len);
  worldUrl = cacheURL;
  delete [] cacheURL;
}


void handleWantHash(void *msg, uint16_t len)
{
  char *hexDigest = new char[len];
  nboUnpackString(msg, hexDigest, len);
  isCacheTemp = hexDigest[0] == 't';
  md5Digest = &hexDigest[1];

  worldDownLoader->start(hexDigest);
  delete [] hexDigest;
}


void handleGetWorld(void *msg, uint16_t len)
{
  // create world
  uint32_t bytesLeft;
  void *buf = nboUnpackUInt32(msg, bytesLeft);
  bool last = processWorldChunk(buf, len - 4, bytesLeft);
  if (!last) {
    char message[MaxPacketLen];
    // ask for next chunk
    worldPtr += len - 4;
    nboPackUInt32(message, worldPtr);
    serverLink->send(MsgGetWorld, sizeof(uint32_t), message);
    return;
  }

  if (cacheOut)
    delete cacheOut;
  cacheOut = NULL;
  loadCachedWorld();
  if (isCacheTemp)
    markOld(worldCachePath);
}


void handleGameTime(void *msg)
{
  GameTime::unpack(msg);
  GameTime::update();
}


void handleTimeUpdate(void *msg)
{
  int32_t timeLeft;
  msg = nboUnpackInt32(msg, timeLeft);
  //hud->setTimeLeft(timeLeft);
  if (timeLeft == 0) {
    gameOver = true;
    myTank->explodeTank();
    showMessage("Time Expired");
    //hud->setAlert(0, "Time Expired", 10.0f, true);
#ifdef ROBOT
    for (int i = 0; i < numRobots; i++)
      if (robots[i])
	robots[i]->explodeTank();
#endif
  } else if (timeLeft < 0) {
    //hud->setAlert(0, "Game Paused", 10.0f, true);
  }
}


void handleScoreOver(void *msg)
{
  // unpack packet
  PlayerId id;
  uint16_t team;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, team);
  Player *player = lookupPlayer(id);

  // make a message
  std::string msg2;
  if (team == (uint16_t)NoTeam) {
    // a player won
    if (player) {
      msg2 = TextUtils::format("%s (%s) won the game",
      player->getCallSign(),
      Team::getName(player->getTeam()));
    } else {
      msg2 = "[unknown player] won the game";
    }
  } else {
    msg2 = TextUtils::format("%s won the game",
      Team::getName(TeamColor(team)));
  }

  gameOver = true;
  myTank->explodeTank();

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    if (robots[i])
      robots[i]->explodeTank();
  }
#endif
}


void handleAddPlayer(void* msg, bool& checkScores)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

#if defined(FIXME) && defined(ROBOT)
  saveRobotInfo(id, msg);
#endif

  if (id == myTank->getId()) {
    enteringServer(msg); // it's me!  should be the end of updates
  } else {
    //addPlayer(id, msg, entered);
    addPlayer(id, msg, true);
    updateNumPlayers();
    checkScores = true;

    if (myTank->getId() >= 200) {
      setTankFlags(); // update the tank flags when in replay mode.
    }
  }
}


void handleRemovePlayer(void *msg, bool &checkScores)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  if (removePlayer(id)) {
    checkScores = true;
  }
}


void handleFlagUpdate(void *msg, size_t len)
{
  uint16_t count = 0;
  uint16_t flagIndex;
  if (len >= 2)
    msg = nboUnpackUInt16(msg, count);

  size_t perFlagSize = 2 + 55;

  if (len >= (2 + (perFlagSize*count)))
    for (int i = 0; i < count; i++) {
      msg = nboUnpackUInt16(msg, flagIndex);
      msg = world->getFlag(int(flagIndex)).unpack(msg);
      world->initFlag(int(flagIndex));
    }
}


void handleTeamUpdate(void *msg, bool &checkScores)
{
  uint8_t  numTeams;
  uint16_t team;

  msg = nboUnpackUInt8(msg, numTeams);
  for (int i = 0; i < numTeams; i++) {
    msg = nboUnpackUInt16(msg, team);
    msg = teams[int(team)].unpack(msg);
  }
  updateNumPlayers();
  checkScores = true;
}


void handleAliveMessage(void *msg)
{
  PlayerId id;
  fvec3 pos;
  float forward;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackFVec3(msg, pos);
  msg = nboUnpackFloat(msg, forward);
  int playerIndex = lookupPlayerIndex(id);

  if ((playerIndex < 0) && (playerIndex != -2)) {
    return;
  }
  Player *tank = getPlayerByIndex(playerIndex);
  if (tank == NULL) {
    return;
  }

  if (tank == myTank) {
    //wasRabbit = tank->getTeam() == RabbitTeam;
    myTank->restart(pos, forward);
    myTank->setShotType(StandardShot);
    //firstLife = false;
    justJoined = false;

    //if (!myTank->isAutoPilot())
    //  mainWindow->warpMouse();

    //hud->setAltitudeTape(myTank->canJump());
  }
#ifdef ROBOT
  else if (tank->getPlayerType() == ComputerPlayer) {
    for (int r = 0; r < numRobots; r++) {
      if (robots[r] && robots[r]->getId() == playerIndex) {
        robots[r]->restart(pos, forward);
        setRobotTarget(robots[r]);
        break;
      }
    }
  }
#endif

  static const fvec3 zero(0.0f, 0.0f, 0.0f);
  tank->setStatus(tank->getStatus() | PlayerState::Alive);
  tank->move(pos, forward);
  tank->setVelocity(zero);
  tank->setAngularVelocity(0.0f);
  tank->setDeadReckoning(syncedClock.GetServerSeconds());
  tank->spawnEffect();
  if (tank == myTank) {
    myTank->setSpawning(false);
  }

  //SOUNDSYSTEM.play(SFX_POP, pos, true, isViewTank(tank));
}


void handleAutoPilot(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  uint8_t autopilot;
  nboUnpackUInt8(msg, autopilot);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  tank->setAutoPilot(autopilot != 0);
  addMessage(tank, autopilot ? "Roger taking controls" : "Roger releasing controls");
}

void handleAllow(void *msg)
{
  PlayerId id;
  LocalPlayer *localtank = NULL;
  msg = nboUnpackUInt8(msg, id);

  uint8_t allow;
  nboUnpackUInt8(msg, allow);

  Player *tank = NULL;
  if (myTank && myTank->getId() == id) {
    tank = localtank = myTank;
  } else {
#ifdef ROBOT
    for (int i = 0; i < MAX_ROBOTS; i++) {
      if (robots[i] && robots[i]->getId() == id) {
	tank = localtank = robots[i];
      }
    }
#endif
    if (!tank)
      tank = lookupPlayer(id);

  }
  if (!tank) return;

  if (localtank) {
    localtank->setDesiredSpeed(0.0);
    localtank->setDesiredAngVel(0.0);
    // drop any team flag we may have, as would happen if we paused
    const FlagType *flagd = localtank->getFlag();
    if (flagd->flagTeam != NoTeam) {
      serverLink->sendDropFlag(localtank->getPosition());
      localtank->setShotType(StandardShot);
    }
  }

  // FIXME - this is currently too noisy
  tank->setAllow(allow);
  addMessage(tank, allow & AllowShoot ? "Shooting allowed" : "Shooting forbidden");
  addMessage(tank, allow & (AllowMoveForward  |
                            AllowMoveBackward |
                            AllowTurnLeft     |
                            AllowTurnRight) ? "Movement allowed"
                                            : "Movement restricted");
  addMessage(tank, allow & AllowJump ? "Jumping allowed" : "Jumping forbidden");
}


void handleKilledMessage(void *msg, bool /*human*/, bool &checkScores)
{
  PlayerId victim, killer;
  FlagType *flagType;
  int16_t shotId, reason;
  int phydrv = -1;
  msg = nboUnpackUInt8(msg, victim);
  msg = nboUnpackUInt8(msg, killer);
  msg = nboUnpackInt16(msg, reason);
  msg = nboUnpackInt16(msg, shotId);
  msg = FlagType::unpack(msg, flagType);
  if (reason == (int16_t)PhysicsDriverDeath) {
    int32_t inPhyDrv;
    msg = nboUnpackInt32(msg, inPhyDrv);
    phydrv = int(inPhyDrv);
  }
  BaseLocalPlayer *victimLocal = getLocalPlayer(victim);
  BaseLocalPlayer *killerLocal = getLocalPlayer(killer);
  Player *victimPlayer = lookupPlayer(victim);
  Player *killerPlayer = lookupPlayer(killer);

  if (victimPlayer == myTank) {
    // uh oh, i'm dead
    if (myTank->isAlive()) {
      serverLink->sendDropFlag(myTank->getPosition());
      myTank->setShotType(StandardShot);
      //handleMyTankKilled(reason);
    }
  }

  if (victimLocal) {
    // uh oh, local player is dead
    if (victimLocal->isAlive()) {
      gotBlowedUp(victimLocal, GotKilledMsg, killer);
      rcLink->pushEvent(new DeathEvent());
    }
  }
  else if (victimPlayer) {
    victimPlayer->setExplode(TimeKeeper::getTick());
  }

  if (killerLocal) {
    // local player did it
    if (shotId >= 0)
      killerLocal->endShot(shotId, true);	// terminate the shot
  }

#ifdef ROBOT
  // blow up robots on victim's team if shot was genocide
  if (killerPlayer && victimPlayer && shotId >= 0) {
    const ShotPath *shot = killerPlayer->getShot(int(shotId));
    if (shot && shot->getFlag() == Flags::Genocide) {
      for (int i = 0; i < numRobots; i++) {
	if (robots[i] && victimPlayer != robots[i] && victimPlayer->getTeam() == robots[i]->getTeam() && robots[i]->getTeam() != RogueTeam)
	  gotBlowedUp(robots[i], GenocideEffect, killerPlayer->getId());
      }
    }
  }
#endif

  checkScores = true;
}


void handleGrabFlag(void *msg)
{
  PlayerId id;
  uint16_t flagIndex;
  unsigned char shot;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  Flag& flag = world->getFlag(int(flagIndex));
  msg = flag.unpack(msg);
  msg = nboUnpackUInt8(msg, shot);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  // player now has flag
  tank->setFlag(flag.type);
  tank->setShotType((ShotType)shot);

  std::string message("grabbed ");
  message += tank->getFlag()->flagName;
  message += " flag";

  addMessage(tank, message);
}


void handleDropFlag(void *msg)
{
  PlayerId id;
  uint16_t flagIndex;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  Flag& flag = world->getFlag(int(flagIndex));
  msg = flag.unpack(msg);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  handleFlagDropped(tank);
}


void handleCaptureFlag(void *msg, bool &checkScores)
{
  PlayerId id;
  uint16_t flagIndex, team;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  msg = nboUnpackUInt16(msg, team);
  Player *capturer = lookupPlayer(id);

  Flag& capturedFlag = world->getFlag(int(flagIndex));

  if (capturedFlag.type == Flags::Null)
    return;

  int capturedTeam = capturedFlag.type->flagTeam;

  // player no longer has flag
  if (capturer) {
    capturer->setFlag(Flags::Null);
    //if (capturer == myTank)
    //  updateFlag(Flags::Null);

    // add message
    if (int(capturer->getTeam()) == capturedTeam) {
      std::string message("took my flag into ");
      message += Team::getName(TeamColor(team));
      message += " territory";
      addMessage(capturer, message);
    } else {
      std::string message("captured ");
      message += Team::getName(TeamColor(capturedTeam));
      message += "'s flag";
      addMessage(capturer, message);
    }
  }

#ifdef ROBOT
  //kill all my robots if they are on the captured team
  for (int r = 0; r < numRobots; r++) {
    if (robots[r] && robots[r]->getTeam() == capturedTeam)
      gotBlowedUp(robots[r], GotCaptured, robots[r]->getId());
  }
#endif

  checkScores = true;
}



// changing the rabbit

void handleNewRabbit(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);
  Player *rabbit = lookupPlayer(id);

  // new mode option,
  unsigned char mode;
  msg = nboUnpackUInt8(msg, mode);

  // mode 0 == swap the current rabbit with this rabbit
  // mode 1 == add this person as a rabbit
  // mode 2 == remove this person from the rabbit list

  // we don't need to mod the hunters if we aren't swaping
  if (mode == 0) {
    for (int i = 0; i < curMaxPlayers; i++) {
      if (remotePlayers[i])
	remotePlayers[i]->setHunted(false);
      if (i != id && remotePlayers[i] &&
          remotePlayers[i]->getTeam() != RogueTeam &&
          remotePlayers[i]->getTeam() != ObserverTeam)
	remotePlayers[i]->changeTeam(HunterTeam);
    }
  }

  if (rabbit != NULL) {
    if (mode != 2) {
      rabbit->changeTeam(RabbitTeam);

      if (mode == 0)
	addMessage(rabbit, "is now the rabbit", ControlPanel::MessageMisc, true);
      else
	addMessage(rabbit, "is now a rabbit", ControlPanel::MessageMisc, true);
    } else {
      rabbit->changeTeam(HunterTeam);
      addMessage(rabbit, "is no longer a rabbit", ControlPanel::MessageMisc, true);
    }
  }

#ifdef ROBOT
  for (int r = 0; r < numRobots; r++) {
    if (robots[r]) {
      if (robots[r]->getId() == id) {
	robots[r]->changeTeam(RabbitTeam);
      } else {
	robots[r]->changeTeam(HunterTeam);
      }
    }
  }
#endif
}


void handleSetTeam(void *msg, uint16_t len)
{
  if (len < 2) {
    return;
  }

  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  uint8_t team;
  msg = nboUnpackUInt8(msg, team);

  Player *p = lookupPlayer(id);

  p->changeTeam((TeamColor)team);
}


void handleNearFlag(void *msg)
{
  fvec3 pos;
  std::string flagName;
  msg = nboUnpackFVec3(msg, pos);
  msg = nboUnpackStdString(msg, flagName);

  std::string fullMessage = "Closest Flag: " + flagName;
  std::string colorMsg;
  colorMsg += ColorStrings[YellowColor];
  colorMsg += fullMessage;
  colorMsg += ColorStrings[DefaultColor];

  addMessage(NULL, colorMsg, ControlPanel::MessageServer, false, NULL);

  if (myTank) {
    //hud->setAlert(0, fullMessage.c_str(), 5.0f, false);
  }
}


void handleWhatTimeIsIt(void *msg)
{
  double time = -1;
  unsigned char tag = 0;

  msg = nboUnpackUInt8(msg, tag);
  msg = nboUnpackDouble(msg, time);
  syncedClock.timeMessage(tag, time);
}


void handleSetShotType(BufferedNetworkMessage *msg)
{
  PlayerId id = msg->unpackUInt8();
  unsigned char shotType = msg->unpackUInt8();

  Player *p = lookupPlayer(id);
  if (!p)
    return;
  p->setShotType((ShotType)shotType);
}


void handleShotBegin(bool /*human*/, void *msg)
{
  PlayerId shooterid;
  uint16_t id;

  msg = nboUnpackUInt8(msg, shooterid);
  msg = nboUnpackUInt16(msg, id);

  FiringInfo firingInfo;
  msg = firingInfo.unpack(msg);
  firingInfo.shot.player = shooterid;
  firingInfo.shot.id     = id;

  if (shooterid >= playerSize) {
    return;
  }

  if (shooterid == myTank->getId()) {
    // the shot is ours, find the shot we made, and kill it
    // then rebuild the shot with the info from the server
    myTank->updateShot(firingInfo, id, firingInfo.timeSent);
  }
  else {
    RemotePlayer *shooter = remotePlayers[shooterid];

    if (shooterid != ServerPlayer) {
      if (shooter && remotePlayers[shooterid]->getId() == shooterid) {
	shooter->addShot(firingInfo);
      }
    }
  }
}


void handleWShotBegin (void *msg)
{
  FiringInfo firingInfo;
  msg = firingInfo.unpack(msg);

  WorldPlayer *worldWeapons = world->getWorldWeapons();

  worldWeapons->addShot(firingInfo);
  //playShotSound(firingInfo, false);
}


void handleShotEnd(void *msg)
{
  PlayerId id;
  int16_t shotId;
  uint16_t reason;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackInt16(msg, shotId);
  msg = nboUnpackUInt16(msg, reason);
  BaseLocalPlayer *localPlayer = getLocalPlayer(id);

  if (localPlayer) {
    localPlayer->endShot(int(shotId), false, reason == 0);
  } else {
    Player *pl = lookupPlayer(id);
    if (pl)
      pl->endShot(int(shotId), false, reason == 0);
  }
}


void handleHandicap(void *msg)
{
  PlayerId id;
  uint8_t numHandicaps;
  int16_t handicap;
  msg = nboUnpackUInt8(msg, numHandicaps);
  for (uint8_t s = 0; s < numHandicaps; s++) {
    msg = nboUnpackUInt8(msg, id);
    msg = nboUnpackInt16(msg, handicap);
    Player *sPlayer = NULL;
    if (id == myTank->getId()) {
      sPlayer = myTank;
    } else {
      int i = lookupPlayerIndex(id);
      if (i >= 0)
	sPlayer = getPlayerByIndex(i);
      else
	logDebugMessage(1, "Received handicap update for unknown player!\n");
    }
    if (sPlayer) {
      // a relative score of -50 points will provide maximum handicap
      float normalizedHandicap = float(handicap)
	/ BZDB.eval(StateDatabase::BZDB_HANDICAPSCOREDIFF);

      /* limit how much of a handicap is afforded, and only provide
      * handicap advantages instead of disadvantages.
      */
      if (normalizedHandicap > 1.0f)
	// advantage
	normalizedHandicap  = 1.0f;
      else if (normalizedHandicap < 0.0f)
	// disadvantage
	normalizedHandicap  = 0.0f;

      sPlayer->setHandicap(normalizedHandicap);
    }
  }
}


void handleScore(void *msg)
{
  uint8_t numScores;
  PlayerId id;
  float rank;
  uint16_t wins, losses, tks;
  msg = nboUnpackUInt8(msg, numScores);

  for (uint8_t s = 0; s < numScores; s++) {
    msg = nboUnpackUInt8(msg, id);
    msg = nboUnpackFloat(msg, rank);
    msg = nboUnpackUInt16(msg, wins);
    msg = nboUnpackUInt16(msg, losses);
    msg = nboUnpackUInt16(msg, tks);

    Player *sPlayer = NULL;
    if (id == myTank->getId()) {
      sPlayer = myTank;
    } else {
      int i = lookupPlayerIndex(id);
      if (i >= 0)
	sPlayer = getPlayerByIndex(i);
      else
	logDebugMessage(1, "Received score update for unknown player!\n");
    }
    if (sPlayer) {
      if (sPlayer == myTank) {
	/*ExportInformation &ei = ExportInformation::instance();
	ei.setInformation("Score",
	  TextUtils::format("%d (%d-%d) [%d]",
	  wins - losses, wins, losses, tks),
	  ExportInformation::eitPlayerStatistics,
	  ExportInformation::eipPrivate);*/
      }
      sPlayer->changeScore(rank, wins, losses, tks);
    }
  }
}


void handleTeleport(void *msg)
{
  PlayerId id;
  uint16_t srcID, dstID;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, srcID);
  msg = nboUnpackUInt16(msg, dstID);
  Player *tank = lookupPlayer(id);
  if (tank) {
    if (tank != myTank) {
      tank->setTeleport(TimeKeeper::getTick(), short(srcID), short(dstID));
      /*const MeshFace* linkDst = linkManager.getLinkDstFace(dstID);
      const MeshFace* linkSrc = linkManager.getLinkSrcFace(srcID);
      if (linkDst && linkSrc) {
        const fvec3& pos = linkDst->getPosition();
        if (!linkSrc->linkSrcNoSound()) {
          SOUNDSYSTEM.play(SFX_TELEPORT, pos, false, false);
        }
      }*/
    }
  }
}


void handleMessage(void*/*msg*/)
{
}


void handleTransferFlag(void *msg)
{
  PlayerId fromId, toId;
  unsigned short flagIndex;
  msg = nboUnpackUInt8(msg, fromId);
  msg = nboUnpackUInt8(msg, toId);
  msg = nboUnpackUInt16(msg, flagIndex);
  msg = world->getFlag(int(flagIndex)).unpack(msg);
  unsigned char t = 0;
  msg = nboUnpackUInt8(msg, t);
  Player *fromTank = lookupPlayer(fromId);
  Player *toTank = lookupPlayer(toId);
  handleFlagTransferred(fromTank, toTank, flagIndex, (ShotType)t);
}


void handleReplayReset(void *msg, bool &checkScores)
{
  int i;
  unsigned char lastPlayer;
  msg = nboUnpackUInt8(msg, lastPlayer);

  // remove players up to 'lastPlayer'
  // any PlayerId above lastPlayer is a replay observers
  for (i = 0 ; i < lastPlayer ; i++)
    if (removePlayer (i))
      checkScores = true;

  // remove all of the flags
  for (i = 0 ; i < numFlags; i++) {
    Flag &flag = world->getFlag(i);
    flag.owner = (PlayerId) -1;
    flag.status = FlagNoExist;
    world->initFlag(i);
  }
}


void handleAdminInfo(void *msg)
{
  uint8_t numIPs;
  msg = nboUnpackUInt8(msg, numIPs);

  /* if we're getting this, we have playerlist perm */
  myTank->setPlayerList(true);

  // replacement for the normal MsgAddPlayer message
  if (numIPs == 1) {
    uint8_t ipsize;
    uint8_t index;
    Address ip;
    void *tmpMsg = msg; // leave 'msg' pointing at the first entry

    tmpMsg = nboUnpackUInt8(tmpMsg, ipsize);
    tmpMsg = nboUnpackUInt8(tmpMsg, index);
    tmpMsg = ip.unpack(tmpMsg);
    int playerIndex = lookupPlayerIndex(index);
    Player *tank = getPlayerByIndex(playerIndex);
    if (!tank)
      return;

    std::string name(tank->getCallSign());
    std::string message("joining as ");
    if (tank->getTeam() == ObserverTeam) {
      message += "an observer";
    } else {
      switch (tank->getPlayerType()) {
case TankPlayer:
  message += "a tank";
  break;
case ComputerPlayer:
  message += "a robot tank";
  break;
default:
  message += "an unknown type";
  break;
      }
    }
    message += " from " + ip.getDotNotation();
    tank->setIpAddress(ip);
    addMessage(tank, message);
  }

  // print fancy version to be easily found
  if ((numIPs != 1) || (BZDB.evalInt("showips") > 0)) {
    uint8_t playerId;
    uint8_t addrlen;
    Address addr;

    for (int i = 0; i < numIPs; i++) {
      msg = nboUnpackUInt8(msg, addrlen);
      msg = nboUnpackUInt8(msg, playerId);
      msg = addr.unpack(msg);

      int playerIndex = lookupPlayerIndex(playerId);
      Player *player = getPlayerByIndex(playerIndex);
      if (!player) continue;
      printIpInfo(player, addr, "(join)");
      player->setIpAddress(addr); // save for the signoff message
    } // end for loop
  }
}


void handlePlayerInfo(void *msg)
{
  uint8_t numPlayers;
  int i;
  msg = nboUnpackUInt8(msg, numPlayers);
  for (i = 0; i < numPlayers; ++i) {
    PlayerId id;
    msg = nboUnpackUInt8(msg, id);
    Player *p = lookupPlayer(id);
    uint8_t info;
    // parse player info bitfield
    msg = nboUnpackUInt8(msg, info);
    if (!p)
      continue;
    p->setAdmin((info & IsAdmin) != 0);
    p->setRegistered((info & IsRegistered) != 0);
    p->setVerified((info & IsVerified) != 0);
  }
}


void handleNewPlayer(void *msg)
{
  uint8_t id;
  msg = nboUnpackUInt8(msg, id);
#ifdef ROBOT
  int i;
  for (i = 0; i < MAX_ROBOTS; i++)
    if (!robots[i])
      break;
  if (i >= MAX_ROBOTS) {
    logDebugMessage(1, "Too many bots requested\n");
    return;
  }
  robots[i] = new RobotPlayer(id,
    TextUtils::format("%s%2.2d", myTank->getCallSign(),
    i).c_str(),
    serverLink);
  robots[i]->setTeam(AutomaticTeam);
  serverLink->sendEnter(id, ComputerPlayer, NoUpdates, robots[i]->getTeam(),
    robots[i]->getCallSign(), "", "");
  if (!numRobots) {
    makeObstacleList();
    RobotPlayer::setObstacleList(&obstacleList);
  }
  numRobots++;
#endif
}


void handlePlayerData(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  std::string key, value;
  msg = nboUnpackStdString(msg, key);
  msg = nboUnpackStdString(msg, value);

  Player *p = lookupPlayer(id);
  if (p && key.size())
    p->customData[key] = value;
}


void handleMovementUpdate(uint16_t code, void *msg)
{
  double timestamp;
  PlayerId id;
  int32_t order;
  void *buf = msg;

  buf = nboUnpackUInt8(buf, id); // peek! don't update the msg pointer
  buf = nboUnpackDouble(buf, timestamp); // peek

  Player *tank = lookupPlayer(id);
  if (!tank || tank == myTank)
    return;

  nboUnpackInt32(buf, order); // peek
  if (order <= tank->getOrder())
    return;
  short oldStatus = tank->getStatus();

  tank->unpack(msg, code); // now read
  short newStatus = tank->getStatus();

  if ((oldStatus & short(PlayerState::Paused)) != (newStatus & short(PlayerState::Paused)))
    addMessage(tank, (tank->getStatus() & PlayerState::Paused) ? "Paused" : "Resumed");

  if ((oldStatus & short(PlayerState::Exploding)) == 0 && (newStatus & short(PlayerState::Exploding)) != 0) {
    // player has started exploding and we haven't gotten killed
    // message yet -- set explosion now, play sound later (when we
    // get killed message).  status is already !Alive so make player
    // alive again, then call setExplode to kill him.
    tank->setStatus(newStatus | short(PlayerState::Alive));
    tank->setExplode(TimeKeeper::getTick());
    // ROBOT -- play explosion now
  }
}


void handleGMUpdate(void *msg)
{
  ShotUpdate shot;
  msg = shot.unpack(msg);
  Player *tank = lookupPlayer(shot.player);
  if (!tank || tank == myTank)
    return;

  RemotePlayer *remoteTank = (RemotePlayer*)tank;
  RemoteShotPath *shotPath = (RemoteShotPath*)remoteTank->getShot(shot.id);
  if (shotPath)
    shotPath->update(shot, msg);
  PlayerId targetId;
  msg = nboUnpackUInt8(msg, targetId);
  Player *targetTank = getPlayerByIndex(targetId);

  if (targetTank && (targetTank == myTank) && (myTank->isAlive()))
  {
    static TimeKeeper lastLockMsg;
    if (TimeKeeper::getTick() - lastLockMsg > 0.75)
    {
      //SOUNDSYSTEM.play(SFX_LOCK, shot.pos, false, false);
      lastLockMsg = TimeKeeper::getTick();
      addMessage(tank, "locked on me");
    }
  }
}


void handleTangUpdate(uint16_t len, void *msg)
{
  if (len >= 5) {
    unsigned int objectID = 0;
    msg = nboUnpackUInt32(msg, objectID);
    unsigned char tang = 0;
    msg = nboUnpackUInt8(msg, tang);

    ClientIntangibilityManager::instance().setWorldObjectTangibility(objectID, tang);
  }
}


void handleTangReset(void)
{
  ClientIntangibilityManager::instance().resetTangibility();
}


void handleAllowSpawn(uint16_t len, void *msg)
{
  if (len >= 1) {
    unsigned char allow = 0;
    msg = nboUnpackUInt8(msg, allow);

    canSpawn = allow != 0;
  }
}


void handleLimboMessage(void *msg)
{
  nboUnpackStdString(msg, customLimboMessage);
}


//
// message handling
//

class ClientReceiveCallback : public NetworkMessageTransferCallback
{
  public:
    virtual size_t receive(BufferedNetworkMessage *message)
    {
      if (!serverLink || serverError) {
        return 0;
      }

      int e = 0;

      e = serverLink->read(message, 0);

      if (e == -2) {
        printError("Server communication error");
        serverError = true;
        return 0;
      }

      if (e == 1) {
        return message->size() + 4;
      }

      return 0;
    }
};


static void doMessages()
{
  static ClientReceiveCallback clientMessageCallback;

  BufferedNetworkMessageManager::MessageList messageList;

  MSGMGR.update();
  MSGMGR.receiveMessages(&clientMessageCallback, messageList);

  BufferedNetworkMessageManager::MessageList::iterator itr = messageList.begin();

  while (itr != messageList.end()) {
    if (!handleServerMessage(true, *itr))
      handleServerMessage(true, (*itr)->getCode(), (uint16_t)(*itr)->size(), (*itr)->buffer());

    delete *itr;
    itr++;
  }
}


void handleMsgSetVars(void *msg)
{
  uint16_t numVars;
  uint8_t nameLen, valueLen;

  char name[MaxPacketLen];
  char value[MaxPacketLen];

  msg = nboUnpackUInt16(msg, numVars);
  for (int i = 0; i < numVars; i++) {
    msg = nboUnpackUInt8(msg, nameLen);
    msg = nboUnpackString(msg, name, nameLen);
    name[nameLen] = '\0';

    msg = nboUnpackUInt8(msg, valueLen);
    msg = nboUnpackString(msg, value, valueLen);
    value[valueLen] = '\0';

    BZDB.set(name, value);
    BZDB.setPersistent(name, false);
    BZDB.setPermission(name, StateDatabase::Locked);
  }
}

void handleFlagDropped(Player* tank)
{
  // skip it if player doesn't actually have a flag
  if (tank->getFlag() == Flags::Null) return;

  // add message
  std::string message("dropped ");
  message += tank->getFlag()->flagName;
  message += " flag";
  addMessage(tank, message);

  // player no longer has flag
  tank->setFlag(Flags::Null);
}


void handleFlagTransferred(Player *fromTank, Player *toTank, int flagIndex, ShotType shotType)
{
  Flag& f = world->getFlag(flagIndex);

  fromTank->setShotType(StandardShot);
  fromTank->setFlag(Flags::Null);
  toTank->setShotType(shotType);
  toTank->setFlag(f.type);

  std::string message(toTank->getCallSign());
  message += " stole ";
  message += fromTank->getCallSign();
  message += "'s flag";
  addMessage(toTank, message);
}

static bool gotBlowedUp(BaseLocalPlayer* tank, BlowedUpReason reason, PlayerId killer,
			const ShotPath* hit, int phydrv)
{
  if (tank && (tank->getTeam() == ObserverTeam || !tank->isAlive()))
    return false;

  int shotId = -1;
  FlagType* flagType = Flags::Null;
  if (hit) {
    shotId = hit->getShotId();
    flagType = hit->getFlag();
  }

  // you can't take it with you
  const FlagType* flag = tank->getFlag();
  if (flag != Flags::Null) {

    // tell other players I've dropped my flag
    serverLink->sendDropFlag(tank->getPosition());

    // drop it
    handleFlagDropped(tank);
  }

  // take care of explosion business -- don't want to wait for
  // round trip of killed message.  waiting would simplify things,
  // but the delay (2-3 frames usually) can really fool and irritate
  // the player.  we have to be careful to ignore our own Killed
  // message when it gets back to us -- do this by ignoring killed
  // message if we're already dead.
  // don't die if we had the shield flag and we've been shot.
  if (reason != GotShot || flag != Flags::Shield) {
    // blow me up
    tank->explodeTank();

    // tell server I'm dead in case it doesn't already know
    if (reason == GotShot || reason == GotRunOver ||
	reason == GenocideEffect || reason == SelfDestruct ||
	reason == WaterDeath || reason == PhysicsDriverDeath)
      serverLink->sendKilled(tank->getId(), killer, reason, shotId, flagType,
			     phydrv);
  }

  // make sure shot is terminated locally (if not globally) so it can't
  // hit me again if I had the shield flag.  this is important for the
  // shots that aren't stopped by a hit and so may stick around to hit
  // me on the next update, making the shield useless.
  return (reason == GotShot && flag == Flags::Shield && shotId != -1);
}


static inline bool tankHasShotType(const Player* tank, const FlagType* ft)
{
  const int maxShots = tank->getMaxShots();
  for (int i = 0; i < maxShots; i++) {
    const ShotPath* sp = tank->getShot(i);
    if ((sp != NULL) && (sp->getFlag() == ft))
      return true;
  }
  return false;
}


//
// some robot stuff
//

static void addObstacle(std::vector<BzfRegion*>& rgnList, const Obstacle& obstacle)
{
  fvec2 p[4];
  const float* c = obstacle.getPosition();
  const float tankRadius = BZDBCache::tankRadius;

  if (BZDBCache::tankHeight < c[2])
    return;

  const float a = obstacle.getRotation();
  const float w = obstacle.getWidth() + tankRadius;
  const float h = obstacle.getBreadth() + tankRadius;
  const float xx =  w * cosf(a);
  const float xy =  w * sinf(a);
  const float yx = -h * sinf(a);
  const float yy =  h * cosf(a);
  p[0][0] = c[0] - xx - yx;
  p[0][1] = c[1] - xy - yy;
  p[1][0] = c[0] + xx - yx;
  p[1][1] = c[1] + xy - yy;
  p[2][0] = c[0] + xx + yx;
  p[2][1] = c[1] + xy + yy;
  p[3][0] = c[0] - xx + yx;
  p[3][1] = c[1] - xy + yy;

  unsigned int numRegions = (unsigned int)rgnList.size();
  for (unsigned int k = 0; k < numRegions; k++) {
    BzfRegion* region = rgnList[k];
    int side[4];
    if ((side[0] = region->classify(p[0], p[1])) == 1 ||
	(side[1] = region->classify(p[1], p[2])) == 1 ||
	(side[2] = region->classify(p[2], p[3])) == 1 ||
	(side[3] = region->classify(p[3], p[0])) == 1)
      continue;
    if (side[0] == -1 && side[1] == -1 && side[2] == -1 && side[3] == -1) {
      rgnList[k] = rgnList[numRegions-1];
      rgnList[numRegions-1] = rgnList[rgnList.size()-1];
      rgnList.pop_back();
      numRegions--;
      k--;
      delete region;
      continue;
    }
    for (int j = 0; j < 4; j++) {
      if (side[j] == -1) continue;		// to inside
      // split
      const fvec2& p1 = p[j];
      const fvec2& p2 = p[(j+1)&3];
      BzfRegion* newRegion = region->orphanSplitRegion(p2, p1);
      if (!newRegion) continue;		// no split
      if (region != rgnList[k]) rgnList.push_back(region);
      region = newRegion;
    }
    if (region != rgnList[k]) delete region;
  }
}

static void makeObstacleList()
{
  const float tankRadius = BZDBCache::tankRadius;
  int i;
  for (std::vector<BzfRegion*>::iterator itr = obstacleList.begin();
       itr != obstacleList.end(); ++itr)
    delete (*itr);
  obstacleList.clear();

  // FIXME -- shouldn't hard code game area
  fvec2 gameArea[4];
  float worldSize = BZDBCache::worldSize;
  gameArea[0][0] = -0.5f * worldSize + tankRadius;
  gameArea[0][1] = -0.5f * worldSize + tankRadius;
  gameArea[1][0] =  0.5f * worldSize - tankRadius;
  gameArea[1][1] = -0.5f * worldSize + tankRadius;
  gameArea[2][0] =  0.5f * worldSize - tankRadius;
  gameArea[2][1] =  0.5f * worldSize - tankRadius;
  gameArea[3][0] = -0.5f * worldSize + tankRadius;
  gameArea[3][1] =  0.5f * worldSize - tankRadius;
  obstacleList.push_back(new BzfRegion(4, gameArea));

  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  const int numBoxes = boxes.size();
  for (i = 0; i < numBoxes; i++) {
    addObstacle(obstacleList, *boxes[i]);
  }
  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  const int numPyramids = pyramids.size();
  for (i = 0; i < numPyramids; i++) {
    addObstacle(obstacleList, *pyramids[i]);
  }
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  const int numMeshes = meshes.size();
  for (i = 0; i < numMeshes; i++) {
    addObstacle(obstacleList, *meshes[i]);
  }
  if (World::getWorld()->allowTeamFlags()) {
    const ObstacleList& bases = OBSTACLEMGR.getBases();
    const int numBases = bases.size();
    for (i = 0; i < numBases; i++) {
      const BaseBuilding* base = (const BaseBuilding*) bases[i];
      if ((base->getHeight() != 0.0f) || (base->getPosition()[2] != 0.0f)) {
	addObstacle(obstacleList, *base);
      }
    }
  }
}

static void setRobotTarget(RobotPlayer* robot)
{
  Player* bestTarget = NULL;
  float bestPriority = 0.0f;
  for (int j = 0; j < curMaxPlayers; j++)
    if (remotePlayers[j] &&
	remotePlayers[j]->getId() != robot->getId() &&
	remotePlayers[j]->isAlive() &&
	((robot->getTeam() == RogueTeam && !World::getWorld()->allowRabbit()) ||
	 remotePlayers[j]->getTeam() != robot->getTeam())) {

      if (remotePlayers[j]->isPhantomZoned() && !robot->isPhantomZoned())
	continue;

      if (World::getWorld()->allowTeamFlags() &&
	  ((robot->getTeam() == RedTeam && remotePlayers[j]->getFlag() == Flags::RedTeam) ||
	   (robot->getTeam() == GreenTeam && remotePlayers[j]->getFlag() == Flags::GreenTeam) ||
	   (robot->getTeam() == BlueTeam && remotePlayers[j]->getFlag() == Flags::BlueTeam) ||
	   (robot->getTeam() == PurpleTeam && remotePlayers[j]->getFlag() == Flags::PurpleTeam))) {
	bestTarget = remotePlayers[j];
	break;
      }

      const float priority = robot->getTargetPriority(remotePlayers[j]);
      if (priority > bestPriority) {
	bestTarget = remotePlayers[j];
	bestPriority = priority;
      }
    }
  robot->setTarget(bestTarget);
}

static void updateRobots(float dt)
{
  static float newTargetTimeout = 1.0f;
  static float clock = 0.0f;
  bool pickTarget = false;
  int i;

  // see if we should look for new targets
  clock += dt;
  if (clock > newTargetTimeout) {
    while (clock > newTargetTimeout) {
      clock -= newTargetTimeout;
    }
    pickTarget = true;
  }

  // start dead robots
  for (i = 0; i < numRobots; i++) {
    if (!gameOver && robots[i]
	&& !robots[i]->isAlive() && !robots[i]->isExploding() && pickTarget) {
      serverLink->sendAlive(robots[i]->getId());
    }
  }

  if (!rcLink) {
    // retarget robots
    for (i = 0; i < numRobots; i++) {
      if (robots[i] && robots[i]->isAlive()
	  && (pickTarget || !robots[i]->getTarget()
	      || !robots[i]->getTarget()->isAlive())) {
	setRobotTarget(robots[i]);
      }
    }
  }

  // do updates
  for (i = 0; i < numRobots; i++)
    if (robots[i]) {
      robots[i]->update();
      if (robots[i]->hasHitWall())
	rcLink->pushEvent(new HitWallEvent(0.0f)); // TODO: Fix 0.0f to be actual bearing (or heading?)
    }
}


static void		checkEnvironment(RobotPlayer* tank)
{
  // skip this if i'm dead or paused
  if (!tank->isAlive() || tank->isPaused()) return;

  // see if i've been shot
  const ShotPath* hit = NULL;
  float minTime = Infinity;
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i]) {
      tank->checkHit(remotePlayers[i], hit, minTime);
    }
  }

  // Check Server Shots
  tank->checkHit( World::getWorld()->getWorldWeapons(), hit, minTime);

  // Check if I've been tagged (freeze tag).  Note that we alternate the
  // direction that we go through the list to avoid a pathological case.
  static int upwards = 1;
  for (i = 0; i < curMaxPlayers; i ++) {
    int tankid;
    if (upwards)
      tankid = i;
    else
      tankid = curMaxPlayers - 1 - i;

    Player *othertank = lookupPlayer(tankid);

    if (othertank && othertank->getTeam() != ObserverTeam &&
	tankid != tank->getId())
      tank->checkCollision(othertank);
  }
  // swap direction for next time:
  upwards = upwards ? 0 : 1;

  float waterLevel = World::getWorld()->getWaterLevel();

  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      serverLink->sendHit(tank->getId(), hit->getPlayer(), hit->getShotId());

    FlagType* killerFlag = hit->getFlag();
    bool stopShot;

    if (killerFlag == Flags::Thief) {
      if (tank->getFlag() != Flags::Null)
	serverLink->sendTransferFlag(tank->getId(), hit->getPlayer());

      stopShot = true;
    } else {
      stopShot = gotBlowedUp(tank, GotShot, hit->getPlayer(), hit);
    }

    if (stopShot || hit->isStoppedByHit()) {
      Player* hitter = lookupPlayer(hit->getPlayer());
      if (hitter)
	hitter->endShot(hit->getShotId());
    }
  } else if (tank->getDeathPhysicsDriver() >= 0) {
    // if not dead yet, see if i'm sitting on death
    gotBlowedUp(tank, PhysicsDriverDeath, ServerPlayer, NULL,
		tank->getDeathPhysicsDriver());
  } else if ((waterLevel > 0.0f) && (tank->getPosition()[2] <= waterLevel)) {
    // if not dead yet, see if the robot dropped below the death level
    gotBlowedUp(tank, WaterDeath, ServerPlayer);
  } else {
    // if not dead yet, see if i got run over by the steamroller
    bool dead = false;
    const float* myPos = tank->getPosition();
    const float myRadius = tank->getRadius();
    for (i = 0; !dead && i < curMaxPlayers; i++) {
      if (remotePlayers[i] && !remotePlayers[i]->isPaused() &&
	  ((remotePlayers[i]->getFlag() == Flags::Steamroller) ||
	   ((tank->getFlag() == Flags::Burrow) && remotePlayers[i]->isAlive() &&
	    !remotePlayers[i]->isPhantomZoned()))) {
	const float* pos = remotePlayers[i]->getPosition();
	if (pos[2] < 0.0f) continue;
	const float radius = myRadius +
	  (BZDB.eval(StateDatabase::BZDB_SRRADIUSMULT) * remotePlayers[i]->getRadius());
	const float distSquared =
	  hypotf(hypotf(myPos[0] - pos[0],
			myPos[1] - pos[1]), (myPos[2] - pos[2]) * 2.0f);
	if (distSquared < radius) {
	  gotBlowedUp(tank, GotRunOver, remotePlayers[i]->getId());
	  dead = true;
	}
      }
    }
  }
}

static void		checkEnvironmentForRobots()
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i])
      checkEnvironment(robots[i]);
}

void getObsCorners(const Obstacle *obstacle, bool addTankRadius, float corners[4][2])
{
  const float* c = obstacle->getPosition();
  const float a = obstacle->getRotation();
  float w;
  float h;

  if (addTankRadius) {
    const float tankRadius = BZDBCache::tankRadius;
    w = obstacle->getWidth() + tankRadius;
    h = obstacle->getBreadth() + tankRadius;
  } else {
    w = obstacle->getWidth();
    h = obstacle->getBreadth();
  }
  const float xx =  w * cosf(a);
  const float xy =  w * sinf(a);
  const float yx = -h * sinf(a);
  const float yy =  h * cosf(a);

  // TODO, make sure these go consistently in clockwise or counter-clockwise
  // order.
  corners[0][0] = c[0] - xx - yx;
  corners[0][1] = c[1] - xy - yy;
  corners[1][0] = c[0] + xx - yx;
  corners[1][1] = c[1] + xy - yy;
  corners[2][0] = c[0] + xx + yx;
  corners[2][1] = c[1] + xy + yy;
  corners[3][0] = c[0] - xx + yx;
  corners[3][1] = c[1] - xy + yy;
}

/*
// Gaussian RVS using Box-Muller Transform
static float		gauss(float mu, float sigma)
{
  float x, y, z;

  // z is sampled from a gaussian with mu=0 and sigma=1
  x = (float)bzfrand();
  y = (float)bzfrand();
  z = cos(x * 2 * M_PI) * sqrt(-2 * log(1 - y));
  return mu + z * sigma;
}

static void		sendBase(BaseBuilding *base, const char *teamname)
{
  float corners[4][2];
  getObsCorners(base, false, corners);

  rcLink->sendf("base %s", teamname);

  for (int i=0; i < 4; i++) {
    float* point = corners[i];
    rcLink->sendf(" %f %f", point[0], point[1]);
  }

  rcLink->send("\n");
}

static void		sendBasesList()
{
  //if (World::getWorld()->allowTeamFlags()) {
  const ObstacleList& bases = OBSTACLEMGR.getBases();
  const int numBases = bases.size();

  rcLink->send("begin\n");

  for (int i = 0; i < numBases; i++) {
    BaseBuilding* base = (BaseBuilding*) bases[i];
    TeamColor color = (TeamColor)base->getTeam();
    sendBase(base, Team::getShortName(color));
  }

  rcLink->send("end\n");
}

static void		sendObstacle(Obstacle *obs)
{
  float corners[4][2];
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());

  getObsCorners(obs, true, corners);

  rcLink->send("obstacle");

  for (int i=0; i < 4; i++) {
    float* point = corners[i];
    rcLink->sendf(" %f %f", gauss(point[0], posnoise), \
		  gauss(point[1], posnoise));
  }

  rcLink->send("\n");
}

static void		sendObsList()
{
  int i;
  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  const int numBoxes = boxes.size();
  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  const int numPyramids = pyramids.size();
  const int numTeleporters = teleporters.size();
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  const int numMeshes = meshes.size();

  rcLink->send("begin\n");

  for (i = 0; i < numBoxes; i++) {
    sendObstacle(boxes[i]);
  }
  for (i = 0; i < numPyramids; i++) {
    sendObstacle(pyramids[i]);
  }
  for (i = 0; i < numTeleporters; i++) {
    sendObstacle(teleporters[i]);
  }
  for (i = 0; i < numMeshes; i++) {
    sendObstacle(meshes[i]);
  }

  rcLink->send("end\n");
}

static void		sendTeamList()
{
  const Team& redteam = world->getTeam(RedTeam);
  const Team& greenteam = world->getTeam(GreenTeam);
  const Team& blueteam = world->getTeam(BlueTeam);
  const Team& purpleteam = world->getTeam(PurpleTeam);

  rcLink->send("begin\n");

  // Note that we only look at the first base

  if (redteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(RedTeam),
		  redteam.size);
  }
  if (greenteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(GreenTeam),
		  greenteam.size);
  }
  if (blueteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(BlueTeam),
		  blueteam.size);
  }
  if (purpleteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(PurpleTeam),
		  purpleteam.size);
  }

  rcLink->send("end\n");
}

static void		sendFlagList()
{
  rcLink->send("begin\n");

  for (int i=0; i<numFlags; i++) {
    Flag& flag = world->getFlag(i);
    FlagType *flagtype = flag.type;
    Player *possessplayer = lookupPlayer(flag.owner);

    const char *flagteam = Team::getShortName(flagtype->flagTeam);
    const char *possessteam;

    if (possessplayer != NULL) {
      possessteam = Team::getShortName(possessplayer->getTeam());
    } else {
      possessteam = Team::getShortName(NoTeam);
    }

    if (flag.status != FlagNoExist) {
      rcLink->sendf("flag %s %s %f %f\n",
		    flagteam, possessteam,
		    flag.position[0], flag.position[1]);
    }
  }

  rcLink->send("end\n");
}

static void		sendShotList()
{
  rcLink->send("begin\n");

  for (int i=0; i<curMaxPlayers; i++) {
    Player* tank = remotePlayers[i];
    if (!tank) continue;

    int count = tank->getMaxShots();
    for (int j=0; j<count; j++) {
      ShotPath* shot = tank->getShot(j);
      if (!shot || shot->isExpired() || shot->isExpiring()) continue;

      const float* position = shot->getPosition();
      const float* velocity = shot->getVelocity();

      rcLink->sendf("shot %f %f %f %f\n", position[0], position[1],
		    velocity[0], velocity[1]);
    }

  }

  rcLink->send("end\n");
}

static void		sendMyTankList()
{
  RobotPlayer* bot;
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());
  float angnoise = atof(BZDB.get("bzrcAngNoise").c_str());
  float velnoise = atof(BZDB.get("bzrcVelNoise").c_str());

  rcLink->send("begin\n");

  const int numShots = World::getWorld()->getMaxShots();

  for (int i=0; i<numRobots; i++) {
    bot = robots[i];
    const char* callsign = bot->getCallSign();
    char *statstr;
    if (bot->isPaused()) {
      statstr = "paused";
    } else if (bot->isFlagActive()) {
      statstr = "super";
    } else if (bot->isAlive()) {
      if (bot->canMove()) {
	statstr = "normal";
      } else {
	statstr = "frozen";
      }
    } else {
      statstr = "dead";
    }

    int shots_avail = numShots;
    for (int shot=0; shot<numShots; shot++) {
      if (bot->getShot(shot)) shots_avail--;
    }

    float reloadtime = bot->getReloadTime();

    FlagType* flagtype = bot->getFlag();
    char *flagname;
    if (flagtype == Flags::Null) {
      flagname = "none";
    } else if (flagtype == Flags::RedTeam) {
      flagname = "red";
    } else if (flagtype == Flags::GreenTeam) {
      flagname = "green";
    } else if (flagtype == Flags::BlueTeam) {
      flagname = "blue";
    } else if (flagtype == Flags::PurpleTeam) {
      flagname = "purple";
    } else {
      flagname = "other";
    }

    const float *pos = bot->getPosition();
    const float angle = bot->getAngle();
    const float *vel = bot->getVelocity();
    const float angvel = bot->getAngularVelocity();

    float noisy_angle = gauss(angle, angnoise);
    if (noisy_angle > M_PI) {
      noisy_angle -= 2 * M_PI;
    } else if (noisy_angle <= -M_PI) {
      noisy_angle += 2 * M_PI;
    }

    rcLink->sendf("mytank %d %s %s %d %f %s %f %f %f %f %f %f\n",
		  i, callsign, statstr, shots_avail, reloadtime, flagname,
		  gauss(pos[0], posnoise), gauss(pos[1], posnoise),
		  noisy_angle, gauss(vel[0], velnoise),
		  gauss(vel[1], velnoise), gauss(angvel, angnoise));
  }

  rcLink->send("end\n");
}

static void		sendOtherTankList()
{
  Player* tank;
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());
  float angnoise = atof(BZDB.get("bzrcAngNoise").c_str());

  rcLink->send("begin\n");

  for (int i=0; i<curMaxPlayers; i++) {
    tank = remotePlayers[i];
    if (!tank) continue;

    TeamColor team = tank->getTeam();
    if (team == ObserverTeam) continue;
    if (team == startupInfo.team && startupInfo.team != AutomaticTeam) {
      continue;
    }

    const char* callsign = tank->getCallSign();

    const char* colorname = Team::getShortName(team);

    char *statstr;
    if (tank->isPaused()) {
      statstr = "paused";
    } else if (tank->isFlagActive()) {
      statstr = "super";
    } else if (tank->isAlive()) {
      if (tank->canMove()) {
	statstr = "normal";
      } else {
	statstr = "frozen";
      }
    } else {
      statstr = "dead";
    }

    FlagType* flagtype = tank->getFlag();
    char *flagname;
    if (flagtype == Flags::Null) {
      flagname = "none";
    } else if (flagtype == Flags::RedTeam) {
      flagname = "red";
    } else if (flagtype == Flags::GreenTeam) {
      flagname = "green";
    } else if (flagtype == Flags::BlueTeam) {
      flagname = "blue";
    } else if (flagtype == Flags::PurpleTeam) {
      flagname = "purple";
    } else {
      flagname = "other";
    }

    const float *pos = tank->getPosition();
    const float angle = tank->getAngle();

    rcLink->sendf("othertank %s %s %s %s %f %f %f\n",
		  callsign, colorname, statstr, flagname,
		  gauss(pos[0], posnoise), gauss(pos[1], posnoise),
		  gauss(angle, angnoise));
  }

  rcLink->send("end\n");
}

static void		sendConstList()
{
  rcLink->send("begin\n");

  rcLink->sendf("constant team %s\n", Team::getShortName(startupInfo.team));
  rcLink->sendf("constant worldsize %f\n", BZDBCache::worldSize);
  rcLink->send("constant hoverbot 0\n");

  rcLink->send("end\n");
}
*/

static void doBotRequests()
{
  RCRequest* req;

  if (numRobots < 1)
    return;

  while ((req = rcLink->peekRequest()) != NULL) {
    if (!req->process((RCRobotPlayer*)robots[0]))
      return;

    rcLink->popRequest(); // Discard it, we're done with this one.
    rcLink->sendAck(req);
  }
}

static void sendRobotUpdates()
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i] && robots[i]->isDeadReckoningWrong()) {
      serverLink->sendPlayerUpdate(robots[i]);
    }
}

static void addRobots()
{
  int  j;
  for (j = 0; j < MAX_ROBOTS; j++)
    robots[j] = NULL;
}

static void enteringServer(void *buf)
{
#if defined(ROBOT)
  int i;
  for (i = 0; i < numRobotTanks; i++) {
    if(robots[i])
      serverLink->sendNewPlayer(robots[i]->getId());
  }
  numRobots = 0;
#endif
  // the server sends back the team the player was joined to
  void *tmpbuf = buf;
  uint16_t team, type;
  tmpbuf = nboUnpackUInt16(tmpbuf, type);
  tmpbuf = nboUnpackUInt16(tmpbuf, team);

  setTankFlags();

  // clear now invalid token
  startupInfo.token[0] = '\0';

  // add robot tanks
#if defined(ROBOT)
  addRobots();
#endif

  // initialize some other stuff
  updateNumPlayers();
  updateHighScores();

  entered = true;
}


static void setTankFlags()
{
  // scan through flags and, for flags on
  // tanks, tell the tank about its flag.
  const int maxFlags = world->getMaxFlags();
  for (int i = 0; i < maxFlags; i++) {
    const Flag& flag = world->getFlag(i);
    if (flag.status == FlagOnTank) {
      for (int j = 0; j < curMaxPlayers; j++) {
	if (remotePlayers[j] && remotePlayers[j]->getId() == flag.owner) {
	  remotePlayers[j]->setFlag(flag.type);
	  break;
	}
      }
    }
  }
}


static void cleanWorldCache()
{
  // setup the world cache limit
  int cacheLimit = (10 * 1024 * 1024);
  if (BZDB.isSet("worldCacheLimit")) {
    const int dbCacheLimit = BZDB.evalInt("worldCacheLimit");
    // the old limit was 100 Kbytes, too small
    if (dbCacheLimit == (100 * 1024)) {
      BZDB.setInt("worldCacheLimit", cacheLimit);
    } else {
      cacheLimit = dbCacheLimit;
    }
  } else {
    BZDB.setInt("worldCacheLimit", cacheLimit);
  }

  const std::string worldPath = getCacheDirName();

  while (true) {
    char *oldestFile = NULL;
    int oldestSize = 0;
    int totalSize = 0;

#ifdef _WIN32
    std::string pattern = worldPath + "*.bwc";
    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFile(pattern.c_str(), &findData);
    if (h != INVALID_HANDLE_VALUE) {
      FILETIME oldestTime;
      while (FindNextFile(h, &findData)) {
	if ((oldestFile == NULL) ||
	    (CompareFileTime(&oldestTime, &findData.ftLastAccessTime) > 0)) {
	  if (oldestFile) {
	    free(oldestFile);
	  }
	  oldestFile = strdup(findData.cFileName);
	  oldestSize = findData.nFileSizeLow;
	  oldestTime = findData.ftLastAccessTime;
	}
	totalSize += findData.nFileSizeLow;
      }
      FindClose(h);
    }
#else
    DIR *directory = opendir(worldPath.c_str());
    if (directory) {
      struct dirent* contents;
      struct stat statbuf;
      time_t oldestTime = time(NULL);
      while ((contents = readdir(directory))) {
	const std::string filename = contents->d_name;
	const std::string fullname = worldPath + filename;
	stat(fullname.c_str(), &statbuf);
	if (S_ISREG(statbuf.st_mode) && (filename.size() > 4) &&
	    (filename.substr(filename.size() - 4) == ".bwc")) {
	  if ((oldestFile == NULL) || (statbuf.st_atime < oldestTime)) {
	    if (oldestFile) {
	      free(oldestFile);
	    }
	    oldestFile = strdup(contents->d_name);
	    oldestSize = statbuf.st_size;
	    oldestTime = statbuf.st_atime;
	  }
	  totalSize += statbuf.st_size;
	}
      }
      closedir(directory);
    }
#endif

    // any valid cache files?
    if (oldestFile == NULL) {
      return;
    }

    // is the cache small enough?
    if (totalSize < cacheLimit) {
      if (oldestFile != NULL) {
	free(oldestFile);
	oldestFile = NULL;
      }
      return;
    }

    // remove the oldest file
    logDebugMessage(1, "cleanWorldCache: removed %s\n", oldestFile);
    remove((worldPath + oldestFile).c_str());
    free(oldestFile);
    totalSize -= oldestSize;
  }
}


static void markOld(std::string &fileName)
{
#ifdef _WIN32
  FILETIME ft;
  HANDLE h = CreateFile(fileName.c_str(),
			FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h != INVALID_HANDLE_VALUE) {
    SYSTEMTIME st;
    memset(&st, 0, sizeof(st));
    st.wYear = 1900;
    st.wMonth = 1;
    st.wDay = 1;
    SystemTimeToFileTime(&st, &ft);
    SetFileTime(h, &ft, &ft, &ft);
    GetLastError();
    CloseHandle(h);
  }
#else
  struct utimbuf times;
  times.actime = 0;
  times.modtime = 0;
  utime(fileName.c_str(), &times);
#endif
}


static void sendFlagNegotiation()
{
  char msg[MaxPacketLen];
  FlagTypeMap::iterator i;
  char *buf = msg;

  /* Send MsgNegotiateFlags to the server with
   * the abbreviations for all the flags we support.
   */
  for (i = FlagType::getFlagMap().begin();
       i != FlagType::getFlagMap().end(); i++) {
    buf = (char*) i->second->pack(buf);
  }
  serverLink->send(MsgNegotiateFlags, (int)(buf - msg), msg);
}


#if defined(FIXME) && defined(ROBOT)
static void saveRobotInfo(Playerid id, void *msg)
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i]) {
      void *tmpbuf = msg;
      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      tmpbuf = nboUnpackUInt16(tmpbuf, type);
      tmpbuf = nboUnpackUInt16(tmpbuf, team);
      tmpbuf = nboUnpackUInt16(tmpbuf, wins);
      tmpbuf = nboUnpackUInt16(tmpbuf, losses);
      tmpbuf = nboUnpackUInt16(tmpbuf, tks);
      tmpbuf = nboUnpackString(tmpbuf, callsign, CallSignLen);
      std::cerr << "id " << id.port << ':' <<
	id.number << ':' <<
	callsign << ' ' <<
	robots[i]->getId().port << ':' <<
	robots[i]->getId().number << ':' <<
	robots[i]->getCallsign() << std::endl;
      if (strncmp(robots[i]->getCallSign(), callsign, CallSignLen)) {
	// check for real robot id
	char buffer[100];
	snprintf(buffer, 100, "id test %p %p %p %8.8x %8.8x\n",
		 robots[i], tmpbuf, msg, *(int *)tmpbuf, *((int *)tmpbuf + 1));
	std::cerr << buffer;
	if (tmpbuf < (char *)msg + len) {
	  PlayerId id;
	  tmpbuf = nboUnpackUInt8(tmpbuf, id);
	  robots[i]->id.serverHost = id.serverHost;
	  robots[i]->id.port = id.port;
	  robots[i]->id.number = id.number;
	  robots[i]->server->send(MsgIdAck, 0, NULL);
	}
      }
    }
}
#endif

static void resetServerVar(const std::string& name, void*)
{
  // reset server-side variables
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    const std::string defval = BZDB.getDefault(name);
    BZDB.set(name, defval);
  }
}

void leaveGame()
{
  entered = false;
  joiningGame = false;

  // shut down robot connections
  int i;
  for (i = 0; i < numRobots; i++) {
    if (robots[i])
      serverLink->sendExit();
    delete robots[i];
    robots[i] = NULL;
  }
  numRobots = 0;

  for (std::vector<BzfRegion*>::iterator itr = obstacleList.begin();
       itr != obstacleList.end(); ++itr)
    delete (*itr);
  obstacleList.clear();

  // delete world
  World::setWorld(NULL);
  delete world;
  world = NULL;
  teams = NULL;
  curMaxPlayers = 0;
  numFlags = 0;
  remotePlayers = NULL;

  // shut down server connection
  serverLink->sendExit();
  ServerLink::setServer(NULL);
  delete serverLink;
  serverLink = NULL;

  // reset some flags
  gameOver = false;
  serverError = false;
  serverDied = false;

  // reset the BZDB variables
  BZDB.iterate(resetServerVar, NULL);

  return;
}


static void joinInternetGame(const struct in_addr *inAddress)
{
  // get server address
  Address serverAddress(*inAddress);
  if (serverAddress.isAny()) {
    showError("Server not found");
    return;
  }

  // check for a local server block
  serverAccessList.reload();
  std::vector<std::string> nameAndIp;
  nameAndIp.push_back(startupInfo.serverName);
  nameAndIp.push_back(serverAddress.getDotNotation());
  if (!serverAccessList.authorized(nameAndIp)) {
    showError("Server Access Denied Locally");
    std::string msg = ColorStrings[WhiteColor];
    msg += "NOTE: ";
    msg += ColorStrings[GreyColor];
    msg += "server access is controlled by ";
    msg += ColorStrings[YellowColor];
    msg += serverAccessList.getFileName();
    addMessage(NULL, msg);
    return;
  }

  syncedClock.update(NULL);
  // open server
  ServerLink *_serverLink = new ServerLink(serverAddress,
    startupInfo.serverPort);

  serverLink = _serverLink;

  serverLink->sendVarRequest();

  // assume everything's okay for now
  serverDied = false;
  serverError = false;

  if (!serverLink) {
    showError("Memory error");
    return;
  }

  // check server
  if (serverLink->getState() != ServerLink::Okay) {
    switch (serverLink->getState()) {
      case ServerLink::BadVersion: {
        static char versionError[] = "Incompatible server version XXXXXXXX";
        strncpy(versionError + strlen(versionError) - 8,
          serverLink->getVersion(), 8);
        showError(versionError);
        break;
      }
      case ServerLink::Refused: {
        // you got banned
        const std::string &rejmsg = serverLink->getRejectionMessage();

        // add to the HUD
        std::string msg = ColorStrings[RedColor];
        msg += "You have been banned from this server";
        showError(msg.c_str());

        // add to the console
        msg = ColorStrings[RedColor];
        msg += "You have been banned from this server:";
        addMessage(NULL, msg);
        msg = ColorStrings[YellowColor];
        msg += rejmsg;
        addMessage(NULL, msg);

        break;
      }
      case ServerLink::Rejected: {
        // the server is probably full or the game is over.  if not then
        // the server is having network problems.
        showError("Game is full or over.  Try again later.");
        break;
      }
      case ServerLink::SocketError: {
        showError("Error connecting to server.");
        break;
      }
      case ServerLink::CrippledVersion: {
        // can't connect to (otherwise compatible) non-crippled server
        showError("Cannot connect to full version server.");
        break;
      }
      default: {
        showError(TextUtils::format
          ("Internal error connecting to server (error code %d).",
          serverLink->getState()).c_str());
        break;
      }
    }
    return;
  }
  // use parallel UDP if desired and using server relay
  serverLink->sendUDPlinkRequest();

  showError("Connection Established...");

  sendFlagNegotiation();
  joiningGame = true;
  //scoreboard->huntReset();
  GameTime::reset();
  syncedClock.update(serverLink);
}


static void addVarToAutoComplete(const std::string &name, void* /*userData*/)
{
  if ((name.size() <= 0) || (name[0] != '_'))
    return; // we're skipping "poll"

  if (BZDB.getPermission(name) == StateDatabase::Server)
    completer.registerWord(name);

  return;
}


static void joinInternetGame2()
{
  justJoined = true;

  printout("Entering game...");

  ServerLink::setServer(serverLink);
  World::setWorld(world);

  canSpawn = true;

  // prep teams
  teams = world->getTeams();

  // prep players
  curMaxPlayers = 0;
  remotePlayers = world->getPlayers();
  playerSize = world->getPlayersSize();

  // reset the autocompleter
  completer.reset();
  BZDB.iterate(addVarToAutoComplete, NULL);

  // prep flags
  numFlags = world->getMaxFlags();

  // create observer tank.  This is necessary because the server won't
  // send messages to a bot, but they will send them to an observer.
  myTank = new LocalPlayer(serverLink->getId(), startupInfo.callsign);
  myTank->setTeam(ObserverTeam);
  LocalPlayer::setMyTank(myTank);

  // tell the server that the observer tank wants to join
  serverLink->sendEnter(myTank->getId(), TankPlayer, NoUpdates, 
			myTank->getTeam(), myTank->getCallSign(),
			startupInfo.token, startupInfo.referrer);
  startupInfo.token[0] = '\0';
  startupInfo.referrer[0] = '\0';
  joiningGame = false;
}

void getAFastToken(void )
{
  // get token if we need to (have a password but no token)
  if ((startupInfo.token[0] == '\0') && (startupInfo.password[0] != '\0')) {
    ServerList* serverList = new ServerList;
    serverList->startServerPings(&startupInfo);
    // wait no more than 10 seconds for a token
    for (int j = 0; j < 40; j++) {
      serverList->checkEchos(getStartupInfo());
      cURLManager::perform();
      if (startupInfo.token[0] != '\0')
	break;
      TimeKeeper::sleep(0.25f);
    }
    delete serverList;
  }

  // don't let the bad token specifier slip through to the server,
  // just erase it
  if (strcmp(startupInfo.token, "badtoken") == 0)
    startupInfo.token[0] = '\0';
}


void handlePendingJoins(void)
{
  // try to join a game if requested.  do this *before* handling
  // events so we do a redraw after the request is posted and
  // before we actually try to join.
  if (!joinRequested) {
    return;
  }

  // if already connected to a game then first sign off
  if (myTank)
    leaveGame();

  getAFastToken();

  ares.queryHost(startupInfo.serverName);
  waitingDNS = true;

  // don't try again
  joinRequested = false;
}


bool dnsLookupDone(struct in_addr &inAddress)
{
  if (!waitingDNS)
    return false;

  fd_set readers, writers;
  int nfds = -1;
  struct timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 0;
  FD_ZERO(&readers);
  FD_ZERO(&writers);
  ares.setFd(&readers, &writers, nfds);
  nfds = select(nfds + 1, (fd_set*)&readers, (fd_set*)&writers, 0, &timeout);
  ares.process(&readers, &writers);

  AresHandler::ResolutionStatus status = ares.getHostAddress(&inAddress);
  if (status == AresHandler::Failed) {
    showError("Server not found");
    waitingDNS = false;
  } else if (status == AresHandler::HbNSucceeded) {
    waitingDNS = false;
    return true;
  }
  return false;
}

void checkForServerBail(void )
{
  // if server died then leave the game (note that this may cause
  // further server errors but that's okay).
  if (serverError || (serverLink && serverLink->getState() == ServerLink::Hungup)) {
    // if we haven't reported the death yet then do so now
    if (serverDied || (serverLink && serverLink->getState() == ServerLink::Hungup))
      printError("Server has unexpectedly disconnected");
    else
      printError("We were disconencted from the server, quitting.");
    CommandsStandard::quit();
  }
}

void updateShots(const float dt)
{
  // update other tank's shots
  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i])
      remotePlayers[i]->updateShots(dt);
  }

  // update servers shots
  const World *_world = World::getWorld();
  if (_world)
    _world->getWorldWeapons()->updateShots(dt);
}

void doTankMotions(const float /*dt*/)
{
  // do dead reckoning on remote players
  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i]) {
      const bool wasNotResponding = remotePlayers[i]->isNotResponding();
      remotePlayers[i]->doDeadReckoning();
      const bool isNotResponding = remotePlayers[i]->isNotResponding();

      if (!wasNotResponding && isNotResponding)
	addMessage(remotePlayers[i], "not responding");
      else if (wasNotResponding && !isNotResponding)
	addMessage(remotePlayers[i], "okay");
    }
  }
}

void updatePositions ( const float dt )
{
  updateShots(dt);

  doTankMotions(dt);

  if (entered)
    updateRobots(dt);
}

void checkEnvironment ( const float )
{
  if (entered)
    checkEnvironmentForRobots();
}

void doUpdates ( const float dt )
{
  updatePositions(dt);
  checkEnvironment(dt);

  // update AutoHunt
  AutoHunt::update();
}

void doNetworkStuff(void)
{
  // send my data
  if (myTank && myTank->isDeadReckoningWrong() &&
    (myTank->getTeam() != ObserverTeam))
    serverLink->sendPlayerUpdate(myTank); // also calls setDeadReckoning()

#ifdef ROBOT
  if (entered)
    sendRobotUpdates();
#endif

  cURLManager::perform();
}


bool checkForCompleteDownloads(void)
{
  // check if we are waiting for initial texture downloading
  if (!Downloads::instance().requestFinalized())
    return false;

  // downloading is terminated. go!
  Downloads::instance().finalizeDownloads();
  if (downloadingData) {
    downloadingData = false;
    return true;
  } else {
    //setSceneDatabase();
  }

  return false;
}

void doEnergySaver(void )
{
  static TimeKeeper lastTime = TimeKeeper::getCurrent();
  const float fpsLimit = 10000;

  if ((fpsLimit >= 1.0f) && !isnan(fpsLimit)) {
    const float elapsed = float(TimeKeeper::getCurrent() - lastTime);
    if (elapsed > 0.0f) {
      const float period = (1.0f / fpsLimit);
      const float remaining = (period - elapsed);

      if (remaining > 0.0f)
	TimeKeeper::sleep(remaining);
    }
  }
  lastTime = TimeKeeper::getCurrent();
}

//
// main playing loop
//

static void playingLoop()
{
  // start timing
  TimeKeeper::setTick();

  worldDownLoader = new WorldDownLoader;

  // main loop
  while (!CommandsStandard::isQuit()) {
    BZDBCache::update();

    canSpawn = true;

    // set this step game time
    GameTime::setStepTime();

    // get delta time
    TimeKeeper prevTime = TimeKeeper::getTick();
    TimeKeeper::setTick();
    const float dt = float(TimeKeeper::getTick() - prevTime);

    doMessages();    // handle incoming packets

    // have the synced clock do anything it has to do
    syncedClock.update(serverLink);

    if (world)
      world->checkCollisionManager(); // see if the world collision grid needs to be updated

    handlePendingJoins();

    struct in_addr inAddress;
    if (dnsLookupDone(inAddress))
      joinInternetGame(&inAddress);

    // Communicate with remote agent if necessary
    if (rcLink) {
      if (numRobots >= numRobotTanks)
	rcLink->tryAccept();

      rcLink->update();
      doBotRequests();
    }

    callPlayingCallbacks();    // invoke callbacks

    if (CommandsStandard::isQuit())     // quick out
      break;

    checkForServerBail();

    doUpdates(dt);

    doNetworkStuff();

    if (checkForCompleteDownloads())
      joinInternetGame2(); // we did the inital downloads, so we should join

    doEnergySaver();

    if (serverLink)
      serverLink->flush();

  } // end main client loop


  delete worldDownLoader;
}


static void defaultErrorCallback(const char *msg)
{
  std::string message = ColorStrings[RedColor];
  message += msg;
  showMessage(message);
}


void			botStartPlaying()
{
  // register some commands
  const std::vector<CommandListItem>& commandList = getCommandList();
  for (size_t c = 0; c < commandList.size(); ++c) {
    CMDMGR.add(commandList[c].name, commandList[c].func, commandList[c].help);
  }

  // normal error callback
  setErrorCallback(defaultErrorCallback);

  // initialize epoch offset (time)
  userTimeEpochOffset = (double)mktime(&userTime);
  epochOffset = userTimeEpochOffset;
  lastEpochOffset = epochOffset;

  // catch kill signals before changing video mode so we can
  // put it back even if we die.  ignore a few signals.
  bzSignal(SIGILL, SIG_PF(dying));
  bzSignal(SIGABRT, SIG_PF(dying));
  bzSignal(SIGSEGV, SIG_PF(dying));
  bzSignal(SIGTERM, SIG_PF(suicide));
#if !defined(_WIN32)
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGINT, SIG_PF(suicide));
  bzSignal(SIGPIPE, SIG_PF(hangup));
  bzSignal(SIGHUP, SIG_IGN);
  if (bzSignal(SIGQUIT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGQUIT, SIG_PF(dying));
#ifndef GUSI_20
  bzSignal(SIGBUS, SIG_PF(dying));
#endif
  bzSignal(SIGUSR1, SIG_IGN);
  bzSignal(SIGUSR2, SIG_IGN);
#endif /* !defined(_WIN32) */

  updateNumPlayers();

  // windows version can be very helpful in debug logs
#ifdef _WIN32
  if (debugLevel >= 1) {
    OSVERSIONINFO info;
    ZeroMemory(&info, sizeof(OSVERSIONINFO));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    logDebugMessage(1, "Running on Windows %s%d.%d %s\n",
		    (info.dwPlatformId == VER_PLATFORM_WIN32_NT) ? "NT " : "",
		    info.dwMajorVersion, info.dwMinorVersion,
		    info.szCSDVersion);
  }
#endif

  // print expiration
  if (timeBombString()) {
    // add message about date of expiration
    char bombMessage[80];
    snprintf(bombMessage, 80, "This release will expire on %s", timeBombString());
    showMessage(bombMessage);
  }

  // get current MOTD
  //if (!BZDB.isTrue("disableMOTD")) {
  //  motd = new MessageOfTheDay;
  //  motd->getURL(BZDB.get("motdServer"));
  //}

  // inform user of silencePlayers on startup
  for (unsigned int j = 0; j < silencePlayers.size(); j ++){
    std::string aString = silencePlayers[j];
    aString += " Silenced";
    if (silencePlayers[j] == "*")
      aString = "Silenced All Msgs";

    printError(aString);
  }

  int port;
  if (!BZDB.isSet("rcPort")) // Generate a random port between 1024 & 65536.
    port = (int)(bzfrand() * (65536 - 1024)) + 1024;
  else
    port = atoi(BZDB.get("rcPort").c_str());

  // here we register the various RCRequest-handlers for commands
  // that RCLinkBackend receives. :-)
  RCMessageFactory<RCRequest>::initialize();

  rcLink = new RCLinkBackend();
  rcLink->startListening(port);
  RCREQUEST.setLink(rcLink);

  if (!BZDB.isSet("robotScript")) {
    BACKENDLOGGER << "Missing script on commandline!\n" << std::endl;
    exit(EXIT_FAILURE);
  }

  ScriptLoaderFactory::initialize();
  if (!Frontend::run(BZDB.get("robotScript"), "localhost", port))
    return;

  // enter game if we have all the info we need, otherwise
  joinRequested    = true;
  printout("Trying...");

  // start game loop
  playingLoop();

  // clean up
  delete motd;
  leaveGame();
  setErrorCallback(NULL);
  World::done();
  cleanWorldCache();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
