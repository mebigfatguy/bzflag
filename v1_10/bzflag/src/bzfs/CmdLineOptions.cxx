/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4:4786)
#endif

/* this should be the only header necessary except for headers specific
 * to the class implementation (such as version.h)
 */
#include "CmdLineOptions.h"

// invoke persistent recompilation of this for build versioning
#include "version.h"

/* FIXME implementation specific header for global that should eventually go
 * away */
#include <vector>

/* data nasties */
extern float speedTolerance;
extern bool handlePings;
extern int numFlags;
extern std::string passFile;
extern std::string groupsFile;
extern std::string userDatabaseFile;
extern uint16_t maxPlayers;
extern uint16_t maxRealPlayers;
extern std::vector<FlagType*> allowedFlags;

const char *usageString =
"[-a <vel> <rot>] "
"[-admsg <text>] "
"[-autoTeam] "
"[-b] "
"[-badwords <filename>] "
"[-ban ip{,ip}*] "
"[-banfile <filename>] "
"[-c] "
"[-conf <filename>] "
"[-cr] "
"[-d] "
"[-density <num>] "
"[+f {good|<id>}] "
"[-f {bad|<id>}] "
"[-fb] "
"[-filterCallsigns] "
"[-filterChat] "
"[-filterSimple] "
"[-g] "
"[-groupdb <group file>]"
"[-h] "
"[-helpmsg <file> <name>]"
"[-i interface] "
"[-j] "
"[-lagdrop <num>] "
"[-lagwarn <time/ms>] "
"[-maxidle <time/s>] "
"[-mp {<count>|[<count>][,<count>][,<count>][,<count>][,<count>][,<count>]}] "
"[-mps <score>] "
"[-ms <shots>] "
"[-mts <score>] "
"[-p <port>] "
"[-passdb <password file>]"
"[-passwd <password>] "
#ifdef PRINTSCORE
"[-printscore] "
#endif
"[-prohibitBots] "
"[-public <server-description>] "
"[-publicaddr <server-hostname>[:<server-port>]] "
"[-publiclist <list-server-url>] "
"[-q] "
"[+r] "
"[-rabbit [score|killer|random]] "
"[-reportfile <filename>] "
"[-reportpipe <filename>] "
"[-requireudp] "
"[+s <flag-count>] "
"[-s <flag-count>] "
"[-sa] "
"[-sb] "
"[-sl <id> <num>]"
"[-speedtol <tolerance>]"
"[-srvmsg <text>] "
"[-st <time>] "
"[-sw <num>] "
"[-synctime] "
"[-t] "
"[-tftimeout <seconds>] "
#ifdef TIMELIMIT
"[-time <seconds>] "
"[-timemanual] "
#endif
"[-tk] "
"[-tkkr <percent>] "
"[-userdb <user permissions file>]"
"[-vars <filename>]"
"[-version] "
"[-vetoTime <seconds> ]"
"[-votePercentage <percentage>]"
"[-voteRepeatTime <seconds>]"
"[-votesRequired <num>]"
"[-voteTime <seconds> ]"
"[-world <filename>]"
"[-worldsize < world size>]";

const char *extraUsageString =
"\t-a: maximum acceleration settings\n"
"\t-admsg: specify a <msg> which will be broadcast every 15 minutes\n"
"\t-autoTeam: automatically assign players to teams when they join\n"
"\t-b: randomly oriented buildings\n"
"\t-badwords: bad-world file\n"
"\t-ban ip{,ip}*: ban players based on ip address\n"
"\t-banfile: specify a file to load and store the banlist in\n"
"\t-c: capture-the-flag style game,\n"
"\t-conf: configuration file\n"
"\t-cr: capture-the-flag style game with random world\n"
"\t-d: increase debugging level\n"
"\t-density: specify density of buildings for CTF or random worlds (default is 5)\n"
"\t+f: always have flag <id> available\n"
"\t-f: never randomly generate flag <id>\n"
"\t-fb: allow flags on box buildings\n"
"\t-filterCallsigns: filter callsigns to disallow inappropriate user names\n"
"\t-filterChat: filter chat messages\n"
"\t-filterSimple: perform simple exact matches with the bad word list\n"
"\t-g: serve one game and then exit\n"
"\t-groupdb: file to read for group permissions\n"
"\t-h: use random building heights\n"
"\t-helpmsg: show the lines in <file> on command /help <name>\n"
"\t-i: listen on <interface>\n"
"\t-j: allow jumping\n"
"\t-lagdrop: drop player after this many lag warnings\n"
"\t-lagwarn: lag warning threshhold time [ms]\n"
"\t-maxidle: idle kick threshhold [s]\n"
"\t-mp: maximum players total or per team\n"
"\t-mps: set player score limit on each game\n"
"\t-ms: maximum simultaneous shots per player\n"
"\t-mts: set team score limit on each game\n"
"\t-p: use alternative port (default is 5154)\n"
"\t-passdb: file to read for user passwords\n"
"\t-passwd: specify a <password> for operator commands\n"
#ifdef PRINTSCORE
"\t-printscore: write score to stdout whenever it changes\n"
#endif
"\t-prohibitBots: disallow clients from using autopilot or robots\n"
"\t-public <server-description>\n"
"\t-publicaddr <effective-server-hostname>[:<effective-server-port>]\n"
"\t-publiclist <list-server-url>\n"
"\t-q: don't listen for or respond to pings\n"
"\t+r: all shots ricochet\n"
"\t-rabbit [score|killer|random]: rabbit chase style\n"
"\t-reportfile <filename>: the file to store reports in\n"
"\t-reportpipe <filename>: the program to pipe reports through\n"
"\t-requireudp: require clients to use udp\n"
"\t+s: always have <num> super flags (default=16)\n"
"\t-s: allow up to <num> super flags (default=16)\n"
"\t-sa: insert antidote superflags\n"
"\t-sb: allow tanks to respawn on buildings\n"
"\t-sl: limit flag <id> to <num> shots\n"
"\t-speedtol: multiplyers over normal speed to auto kick at\n\t\tdefaults to 1.25, should not be less then 1.0\n"
"\t-srvmsg: specify a <msg> to print upon client login\n"
"\t-st: shake bad flags in <time> seconds\n"
"\t-sw: shake bad flags after <num> wins\n"
"\t-synctime: synchronize time of day on all clients\n"
"\t-t: allow teleporters\n"
"\t-tftimeout: set timeout for team flag zapping (default=30)\n"
#ifdef TIMELIMIT
"\t-time: set time limit on each game\n"
"\t-timemanual: countdown for timed games has to be started with /countdown\n"
#endif
"\t-tk: player does not die when killing a teammate\n"
"\t-tkkr: team killer to wins percentage (1-100) above which player is kicked\n"
"\t-userdb: file to read for user access permissions\n"
"\t-vars: file to read for worlds configuration variables\n"
"\t-vetoTime: maximum seconds an authorized user has to cancel a poll (default is 20)\n"
"\t-votePercentage: percentage of players required to affirm a poll (default is 50.1%)\n"
"\t-voteRepeatTime: minimum seconds required before a player may request\n\t\tanother vote (default is 300)\n"
"\t-votesRequired: minimum count of votes required to make a vote valid (default is 3)\n"
"\t-voteTime: maximum amount of time a player has to vote on a  poll (default is 60)\n"
"\t-version: print version and exit\n"
"\t-world: world file to load\n"
"\t-worldsize: numeric value for the size of the world ( def 400 )\n";


/* private */
static bool parsePlayerCount(const char *argv, CmdLineOptions &options)
{
  /* either a single number or 5 or 6 optional numbers separated by
   * 4 or 5 (mandatory) commas.
   */
  const char *scan = argv;
  while (*scan && *scan != ',') scan++;

  if (*scan == ',') {
    // okay, it's the comma separated list.  count commas
    int commaCount = 1;
    while (*++scan) {
      if (*scan == ',') {
	commaCount++;
      }
    }
    if (commaCount != 5) {
      std::cout << "improper player count list\n";
      return false;
    }

    // no team size limit by default for real teams, set it to max players
    int i;
    for (i = 0; i < CtfTeams ; i++) {
      options.maxTeam[i] = maxRealPlayers;
    }
    // allow 5 Observers by default
    options.maxTeam[ObserverTeam] = 5;

    // now get the new counts

    // number of counts given
    int countCount = 0;
    scan = argv;
    // ctf teams plus observers
    for (i = 0; i < CtfTeams + 1; i++) {
      char *tail;
      long count = strtol(scan, &tail, 10);
      if (tail != scan) {
	// got a number
	countCount++;
	if (count < 0) {
	  options.maxTeam[i] = 0;
	} else {
          if (count > maxRealPlayers) {
            if (i == ObserverTeam && count > MaxPlayers)
              options.maxTeam[i] = MaxPlayers;
            else
              options.maxTeam[i] = maxRealPlayers;
	  } else {
	    options.maxTeam[i] = uint8_t(count);
	  }
        }
      } // end if tail != scan
      while (*tail && *tail != ',') tail++;
      scan = tail + 1;
    } // end iteration over teams

    // if no/zero players were specified, allow a bunch of rogues
    bool allZero = true;
    for (i = 0; i < CtfTeams; i++) {
      if (options.maxTeam[i] != 0) {
	allZero = false;
      }
    }
    if (allZero) {
      options.maxTeam[RogueTeam] = MaxPlayers;
    }

    // if all counts explicitly listed then add 'em up and set maxRealPlayers
    // (unless max total players was explicitly set)
    if (countCount >= CtfTeams && maxRealPlayers == MaxPlayers) {
      maxRealPlayers = 0;
      for (i = 0; i < CtfTeams ; i++) {
	maxRealPlayers += options.maxTeam[i];
      }
    }

  } else {
    /* single number was provided instead of comma-separated */
    char *tail;
    long count = strtol(argv, &tail, 10);
    if (argv == tail) {
      std::cout << "improper player count\n";
      return false;
    }
    if (count < 1) {
      maxRealPlayers = 1;
    } else {
      if (count > MaxPlayers) {
	maxRealPlayers = MaxPlayers;
      } else {
       maxRealPlayers = uint8_t(count);
      }
    }
    // limit max team size to max players
    for (int i = 0; i < CtfTeams ; i++) {
      if (options.maxTeam[i] > maxRealPlayers)
        options.maxTeam[i] = maxRealPlayers;
    }
  } // end check if comm-separated list

  maxPlayers = maxRealPlayers + options.maxTeam[ObserverTeam];
  if (maxPlayers > MaxPlayers) {
    maxPlayers = MaxPlayers;
  }

  return true;
}


/* protected */

/* public: */

void printVersion()
{
  std::cout << "BZFlag server " << getAppVersion() << " (protocol " << getProtocolVersion() <<
  	       ") http://BZFlag.org/\n";
  std::cout << copyright << std::endl;
}

void usage(const char *pname)
{
  printVersion();
  std::cerr << "\nUsage: " << pname << ' ' << usageString << std::endl;
  exit(1);
}

void extraUsage(const char *pname)
{
  char buffer[64];
  printVersion();
  std::cout << "\nUsage: " << pname << ' ' << usageString << std::endl;
  std::cout << std::endl << extraUsageString << std::endl << "Flag codes:\n";
  for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin(); it != FlagType::getFlagMap().end(); ++it) {
    sprintf(buffer, "\t%2.2s %s\n", (*it->second).flagAbbv, (*it->second).flagName);
    std::cout << buffer;
  }
  exit(0);
}

char **parseConfFile( const char *file, int &ac)
{
  std::vector<std::string> tokens;
  ac = 0;

  std::ifstream confStrm(file);
  if (confStrm.is_open()) {
     char buffer[1024];
     confStrm.getline(buffer,1024);

     if (!confStrm.good()) {
       std::cerr << "configuration file not found\n";
       usage("bzfs");
     }

     while (confStrm.good()) {
       std::string line = buffer;
       int startPos = line.find_first_not_of("\t \r\n");
       while ((startPos >= 0) && (line.at(startPos) != '#')) {
	 int endPos;
	 if (line.at(startPos) == '"') {
	   startPos++;
	   endPos = line.find_first_of('"', startPos);
	 }
	 else
	   endPos = line.find_first_of("\t \r\n", startPos+1);
	 if (endPos < 0)
	    endPos = line.length();
	 tokens.push_back(line.substr(startPos,endPos-startPos));
	 startPos = line.find_first_not_of("\t \r\n", endPos+1);
       }
       confStrm.getline(buffer,1024);
     }
  }

  const char **av = new const char*[tokens.size()+1];
  av[0] = strdup("bzfs");
  ac = 1;
  for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    av[ac++] = strdup((*it).c_str());
  return (char **)av;
}


void parse(int argc, char **argv, CmdLineOptions &options)
{
  CmdLineOptions confOptions;
  delete[] flag;  flag = NULL;

  // prepare flag counts
  int i;
  bool allFlagsOut = false;
  bool teamFlagsAdded = false;

  // parse command line
  int playerCountArg = 0,playerCountArg2 = 0;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-a") == 0) {
      // momentum settings
      if (i + 2 >= argc) {
	std::cerr << "two arguments expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.linearAcceleration = (float)atof(argv[++i]);
      options.angularAcceleration = (float)atof(argv[++i]);
      if (options.linearAcceleration < 0.0f)
	options.linearAcceleration = 0.0f;
      if (options.angularAcceleration < 0.0f)
	options.angularAcceleration = 0.0f;
      options.gameStyle |= int(InertiaGameStyle);
    } else if (strcmp(argv[i], "-admsg") == 0) {
       if (++i == argc) {
	 std::cerr << "argument expected for -admsg\n";
	 usage(argv[0]);
       }
       options.advertisemsg = argv[i];
    } else if (strcmp(argv[i], "-autoTeam") == 0) {
      options.autoTeam = true;
    } else if (strcmp(argv[i], "-b") == 0) {
      // random rotation to boxes in capture-the-flag game
      options.randomBoxes = true;
    } else if (strcmp(argv[i], "-badwords") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -badwords\n";
	usage(argv[0]);
      }
      else {
	options.filterFilename = argv[i];
      }
    } else if (strcmp(argv[i], "-ban") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -ban\n";
	usage(argv[0]);
      }
      else
	options.acl.ban(argv[i]);
    } else if (strcmp(argv[i], "-banfile") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -banfile\n";
	usage(argv[0]);
      }
      else {
	options.acl.setBanFile(argv[i]);
	if (!options.acl.load()) {
	  std::cerr << "could not load banfile \"" << argv[i] << "\"\n";
	  usage(argv[0]);
	}
      }
    } else if (strcmp(argv[i], "-c") == 0) {
      // capture the flag style
      options.gameStyle |= int(TeamFlagGameStyle);
      if (options.gameStyle & int(RabbitChaseGameStyle)) {
	options.gameStyle &= ~int(RabbitChaseGameStyle);
	std::cerr << "Capture the flag incompatible with Rabbit Chase\n";
	std::cerr << "Capture the flag assumed\n";
      }
      if (!teamFlagsAdded) {
        for (int t = RedTeam; t <= PurpleTeam; t++)
          options.numTeamFlags[t] += 1;
	teamFlagsAdded = true;
      }
    } else if (strcmp(argv[i], "-conf") == 0) {
      if (++i == argc) {
	std::cerr << "filename expected for -conf\n";
	usage(argv[0]);
      } else {
	int ac;
	char **av;
	av = parseConfFile(argv[i], ac);
	// Theoretically we could merge the options specified in the conf file after parsing
	// the cmd line options. But for now just override them on the spot
	parse(ac, av, options);

	options.numAllowedFlags = 0;

	// These strings need to stick around for -world, -servermsg, etc
	//for (int i = 0; i < ac; i++)
	//  delete[] av[i];
	delete[] av;
      }
    } else if (strcmp(argv[i], "-cr") == 0) {
      // CTF with random world
      options.randomCTF = true;
      // capture the flag style
      options.gameStyle |= int(TeamFlagGameStyle);
      if (options.gameStyle & int(RabbitChaseGameStyle)) {
	options.gameStyle &= ~int(RabbitChaseGameStyle);
	std::cerr << "Capture the flag incompatible with Rabbit Chase\n";
	std::cerr << "Capture the flag assumed\n";
      }
      if (!teamFlagsAdded) {
        for (int t = RedTeam; t <= PurpleTeam; t++)
          options.numTeamFlags[t] += 1;
	teamFlagsAdded = true;
      }
    } else if (strcmp(argv[i], "-d") == 0) {
      // increase debug level
      int count = 0;
      char *scan;
      for (scan = argv[i]+1; *scan == 'd'; scan++) count++;
      if (*scan != '\0') {
	std::cerr << "bad argument \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      debugLevel += count;
    } else if (strcmp(argv[i], "-density") ==0) {
      if (i+1 != argc && isdigit(*argv[i+1])) {
	options.citySize = atoi(argv[i+1]);
	i++;
      }
      else {
	std::cerr << "integer argument expected for -density\n";
	usage(argv[0]);
      }
    } else if (strcmp(argv[i], "-f") == 0) {
      // disallow given flag
      if (++i == argc) {
	std::cerr << "argument expected for -f\n";
	usage(argv[0]);
      }
      if (strcmp(argv[i], "bad") == 0) {
	FlagSet badFlags = Flag::getBadFlags();
	for (FlagSet::iterator it = badFlags.begin(); it != badFlags.end(); it++)
	  options.flagDisallowed[*it] = true;
      } else if (strcmp(argv[i], "good") == 0) {
	FlagSet goodFlags = Flag::getGoodFlags();
	for (FlagSet::iterator it = goodFlags.begin(); it != goodFlags.end(); it++)
	  options.flagDisallowed[*it] = true;
      } else {
	FlagType* fDesc = Flag::getDescFromAbbreviation(argv[i]);
	if (fDesc == Flags::Null) {
	  std::cerr << "invalid flag \"" << argv[i] << "\"\n";
	  usage(argv[0]);
	}
	options.flagDisallowed[fDesc] = true;
      }
    } else if (strcmp(argv[i], "+f") == 0) {
      // add required flag
      if (++i == argc) {
	std::cerr << "argument expected for +f\n";
	usage(argv[0]);
      }

      char *repeatStr = strchr(argv[i], '{');
      int rptCnt = 1;
      if (repeatStr != NULL) {
	*(repeatStr++) = 0;
	rptCnt = atoi(repeatStr);
	if (rptCnt <= 0)
	  rptCnt = 1;
      }
      if (strcmp(argv[i], "good") == 0) {
	FlagSet goodFlags = Flag::getGoodFlags();
	for (FlagSet::iterator it = goodFlags.begin(); it != goodFlags.end(); it++)
	  options.flagCount[*it] += rptCnt;
      } else if (strcmp(argv[i], "bad") == 0) {
	FlagSet badFlags = Flag::getBadFlags();
	for (FlagSet::iterator it = badFlags.begin(); it != badFlags.end(); it++)
	  options.flagCount[*it] += rptCnt;
      } else if (strcmp(argv[i], "team") == 0) {
	for (int t = RedTeam; t <= PurpleTeam; t++) 
	  options.numTeamFlags[t] += rptCnt;
      } else {
	FlagType *fDesc = Flag::getDescFromAbbreviation(argv[i]);
	if (fDesc == Flags::Null) {
	  std::cerr << "invalid flag \"" << argv[i] << "\"\n";
	  usage(argv[0]);
	} else if (fDesc->flagTeam != NoTeam) {
	  options.numTeamFlags[fDesc->flagTeam] += rptCnt;
	} else {
	  options.flagCount[fDesc] += rptCnt;
	}
      }
    } else if (strcmp(argv[i], "-fb") == 0) {
      // flags on buildings
      options.flagsOnBuildings = true;
    } else if (strcmp(argv[i], "-filterCallsigns") == 0) {
      options.filterCallsigns = true;
    } else if (strcmp(argv[i], "-filterChat") == 0) {
      options.filterChat = true;
    } else if (strcmp(argv[i], "-filterSimple") == 0) {
      options.filterSimple = true;
    } else if (strcmp(argv[i], "-g") == 0) {
      options.oneGameOnly = true;
    } else if (strcmp(argv[i], "-groupdb") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      groupsFile = argv[i];
      std::cerr << "using group file \"" << argv[i] << "\"\n";
    } else if (strcmp(argv[i], "-h") == 0) {
      options.randomHeights = true;
    } else if (strcmp(argv[i], "-help") == 0) {
      extraUsage(argv[0]);
    } else if (strcmp(argv[i], "-helpmsg") == 0) {
      if (i + 2 >= argc) {
	std::cerr << "2 arguments expected for -helpmsg\n";
	usage(argv[0]);
      } else {
	i++;
	if (!options.textChunker.parseFile(argv[i], argv[i+1])){
	  std::cerr << "couldn't read helpmsg file \"" << argv[i] << "\"\n";
	  usage(argv[0]);
	}
	i++;
      }
    } else if (strcmp(argv[i], "-i") == 0) {
      // use a different interface
      if (++i == argc) {
	std::cerr << "argument expected for -i\n";
	usage(argv[0]);
      }
      options.pingInterface = argv[i];
    } else if (strcmp(argv[i], "-j") == 0) {
      // allow jumping
      options.gameStyle |= int(JumpingGameStyle);
    } else if (strcmp(argv[i], "-lagdrop") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.maxlagwarn = atoi(argv[i]);
    } else if (strcmp(argv[i], "-lagwarn") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.lagwarnthresh = atoi(argv[i])/1000.0f;
    } else if (strcmp(argv[i], "-maxidle") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.idlekickthresh = (float) atoi(argv[i]);
    } else if (strcmp(argv[i], "-mp") == 0) {
      // set maximum number of players
      if (++i == argc) {
	std::cerr << "argument expected for -mp\n";
	usage(argv[0]);
      }
      if (playerCountArg == 0)
	playerCountArg = i;
      else
	playerCountArg2 = i;
    } else if (strcmp(argv[i], "-mps") == 0) {
      // set maximum player score
      if (++i == argc) {
	std::cerr << "argument expected for -mps\n";
	usage(argv[0]);
      }
      options.maxPlayerScore = atoi(argv[i]);
      if (options.maxPlayerScore < 1) {
	std::cerr << "disabling player score limit\n";
	options.maxPlayerScore = 0;
      }
    } else if (strcmp(argv[i], "-ms") == 0) {
      // set maximum number of shots
      if (++i == argc) {
	std::cerr << "argument expected for -ms\n";
	usage(argv[0]);
      }
      int newMaxShots = atoi(argv[i]);
      if (newMaxShots < 1) {
	std::cerr << "using minimum number of shots of 1\n";
	options.maxShots = 1;
      }
      else if (newMaxShots > MaxShots) {
	std::cerr << "using maximum number of shots of " << MaxShots << std::endl;
	options.maxShots = uint16_t(MaxShots);
      }
      else options.maxShots = uint16_t(newMaxShots);
    } else if (strcmp(argv[i], "-mts") == 0) {
      // set maximum team score
      if (++i == argc) {
	std::cerr << "argument expected for -mts\n";
	usage(argv[0]);
      }
      options.maxTeamScore = atoi(argv[i]);
      if (options.maxTeamScore < 1) {
	std::cerr << "disabling team score limit\n";
	options.maxTeamScore = 0;
      }
    } else if (strcmp(argv[i], "-p") == 0) {
      // use a different port
      if (++i == argc) {
	std::cerr << "argument expected for -p\n";
	usage(argv[0]);
      }
      options.wksPort = atoi(argv[i]);
      if (options.wksPort < 1 || options.wksPort > 65535)
	options.wksPort = ServerPort;
      else
	options.useGivenPort = true;
    } else if (strcmp(argv[i], "-passdb") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      passFile = argv[i];
      std::cerr << "using password file \"" << argv[i] << "\"\n";
    } else if (strcmp(argv[i], "-passwd") == 0 || strcmp(argv[i], "-password") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      // at least put password someplace that ps won't see
      options.password = (char *)malloc(strlen(argv[i]) + 1);
      strcpy(options.password, argv[i]);
      memset(argv[i], ' ', strlen(options.password));
    } else if (strcmp(argv[i], "-pf") == 0) {
      // try wksPort first and if we can't open that port then
      // let system assign a port for us.
      options.useFallbackPort = true;
#ifdef PRINTSCORE
    } else if (strcmp(argv[i], "-printscore") == 0) {
      // dump score whenever it changes
      options.printScore = true;
#endif
    } else if (strcmp(argv[i], "-prohibitBots") == 0) {
      // disallow clients from using autopilot or bots
      options.prohibitBots = true;
    } else if (strcmp(argv[i], "-public") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -public\n";
	usage(argv[0]);
      }
      options.publicizeServer = true;
      options.publicizedTitle = argv[i];
      if (options.publicizedTitle.length() > 127) {
	argv[i][127] = '\0';
	std::cerr << "description too long... truncated\n";
      }
    } else if (strcmp(argv[i], "-publicaddr") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -publicaddr\n";
	usage(argv[0]);
      }
      options.publicizedAddress = argv[i];
      options.publicizeServer = true;
    } else if (strcmp(argv[i], "-publiclist") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -publiclist\n";
	usage(argv[0]);
      }
      options.listServerURL = argv[i];
    } else if (strcmp(argv[i], "-q") == 0) {
      // don't handle pings
      handlePings = false;
    } else if (strcmp(argv[i], "+r") == 0) {
      // all shots ricochet style
      options.gameStyle |= int(RicochetGameStyle);
    } else if (strcmp(argv[i], "-rabbit") == 0) {
      // rabbit chase style
      options.gameStyle |= int(RabbitChaseGameStyle);
      if (options.gameStyle & int(TeamFlagGameStyle)) {
	options.gameStyle &= ~int(TeamFlagGameStyle);
	std::cerr << "Rabbit Chase incompatible with Capture the flag\n";
	std::cerr << "Rabbit Chase assumed\n";
      }
      // default selection style
      options.rabbitSelection = ScoreRabbitSelection; 

      // if there are any arguments following, see if they are a
      // rabbit selection styles.
      if (i+1 != argc) {
	if (strcmp(argv[i+1], "score") == 0) {
	  options.rabbitSelection = ScoreRabbitSelection;
	  i++;
	} else if (strcmp(argv[i+1], "killer") == 0) {
	  options.rabbitSelection = KillerRabbitSelection;
	  i++;
	} else if (strcmp(argv[i+1], "random") == 0) {
	  options.rabbitSelection = RandomRabbitSelection;
	  i++;
	}
      }
    } else if (strcmp(argv[i], "-reportfile") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -reportfile\n";
	usage(argv[0]);
      }
      options.reportFile = argv[i];
    } else if (strcmp(argv[i], "-reportpipe") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -reportpipe\n";
	usage(argv[0]);
      }
      options.reportPipe = argv[i];
    } else if (strcmp(argv[i], "-requireudp") == 0) {
      std::cerr << "require UDP clients!\n";
      options.requireUDP = true;
    } else if (strcmp(argv[i], "+s") == 0) {
      // set required number of random flags
      if (i+1 < argc && isdigit(argv[i+1][0])) {
	++i;
	if ((options.numExtraFlags = atoi(argv[i])) == 0)
	  options.numExtraFlags = 16;
      } else {
	options.numExtraFlags = 16;
      }
      allFlagsOut = true;
    } else if (strcmp(argv[i], "-s") == 0) {
      // allow up to given number of random flags
      if (i+1 < argc && isdigit(argv[i+1][0])) {
	++i;
	if ((options.numExtraFlags = atoi(argv[i])) == 0)
	  options.numExtraFlags = 16;
      }
      else {
	options.numExtraFlags = 16;
      }
      allFlagsOut = false;
    } else if (strcmp(argv[i], "-sa") == 0) {
      // insert antidote flags
      options.gameStyle |= int(AntidoteGameStyle);
    } else if (strcmp(argv[i], "-sb") == 0) {
      // respawns on buildings
      options.respawnOnBuildings = true;
    } else if (strcmp(argv[i], "-sl") == 0) {
      // add required flag
      if (i +2 >= argc) {
	std::cerr << "2 arguments expected for -sl\n";
	usage(argv[0]);
      } else {
	i++;
	FlagType *fDesc = Flag::getDescFromAbbreviation(argv[i]);
	if (fDesc == Flags::Null) {
	  std::cerr << "invalid flag \"" << argv[i] << "\"\n";
	  usage(argv[0]);
	} else {
	  i++;
	  int x = 10;
	  if (isdigit(argv[i][0])){
	    x = atoi(argv[i]);
	    if (x < 1){
	      std::cerr << "can only limit to 1 or more shots\n";
	      usage(argv[0]);
	    }
	  } else {
	    std::cerr << "invalid shot limit \"" << argv[i] << "\"\n";
	    usage(argv[0]);
	  }
	  options.flagLimit[fDesc] = x;
	}
      }
    } else if (strcmp(argv[i], "-speedtol") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      speedTolerance = (float) atof(argv[i]);
      std::cerr << "using speed autokick tolerance of \"" << speedTolerance << "\"\n";
    } else if (strcmp(argv[i], "-srvmsg") == 0) {
       if (++i == argc) {
	 std::cerr << "argument expected for -srvmsg\n";
	 usage(argv[0]);
       }
       options.servermsg = argv[i];
    } else if (strcmp(argv[i], "-st") == 0) {
      // set shake timeout
      if (++i == argc) {
	std::cerr << "argument expected for -st\n";
	usage(argv[0]);
      }
      float timeout = (float)atof(argv[i]);
      if (timeout < 0.1f) {
	options.shakeTimeout = 1;
	std::cerr << "using minimum shake timeout of " << 0.1f * (float)options.shakeTimeout << std::endl;
      } else if (timeout > 300.0f) {
	options.shakeTimeout = 3000;
	std::cerr << "using maximum shake timeout of " << 0.1f * (float)options.shakeTimeout << std::endl;
      } else {
	options.shakeTimeout = uint16_t(timeout * 10.0f + 0.5f);
      }
      options.gameStyle |= int(ShakableGameStyle);
    } else if (strcmp(argv[i], "-sw") == 0) {
      // set shake win count
      if (++i == argc) {
	std::cerr << "argument expected for -sw\n";
	usage(argv[0]);
      }
      int count = atoi(argv[i]);
      if (count < 1) {
	options.shakeWins = 1;
	std::cerr << "using minimum shake win count of " << options.shakeWins << std::endl;
      } else if (count > 20) {
	options.shakeWins = 20;
	std::cerr << "using maximum ttl of " << options.shakeWins << std::endl;
      } else {
	options.shakeWins = uint16_t(count);
      }
      options.gameStyle |= int(ShakableGameStyle);
    } else if (strcmp(argv[i], "-synctime") == 0) {
      // client clocks should be synchronized to server clock
      options.gameStyle |= int(TimeSyncGameStyle);
    } else if (strcmp(argv[i], "-t") == 0) {
      // allow teleporters
      options.useTeleporters = true;
      if (options.worldFile != NULL)
	std::cerr << "-t is meaningless when using a custom world, ignoring\n";
    } else if (strcmp(argv[i], "-tftimeout") == 0) {
      // use team flag timeout
      if (++i == argc) {
	std::cerr << "argument expected for -tftimeout\n";
	usage(argv[0]);
      }
      options.teamFlagTimeout = atoi(argv[i]);
      if (options.teamFlagTimeout < 0)
	options.teamFlagTimeout = 0;
      std::cerr << "using team flag timeout of " << options.teamFlagTimeout << " seconds\n";
#ifdef TIMELIMIT
    } else if (strcmp(argv[i], "-time") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -time\n";
	usage(argv[0]);
      }
      options.timeLimit = (float)atof(argv[i]);
      if (options.timeLimit <= 0.0f) {
	options.timeLimit = 300.0f;
      }
      std::cerr << "using time limit of " << (int)options.timeLimit << " seconds\n";
      options.timeElapsed = options.timeLimit;
    } else if (strcmp(argv[i], "-timemanual") == 0) {
      options.timeManualStart = true;
#endif
    } else if (strcmp(argv[i], "-tk") == 0) {
      // team killer does not die
      options.teamKillerDies = false;
    } else if (strcmp(argv[i], "-tkkr") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for -tkkr";
	usage(argv[0]);
      }
      options.teamKillerKickRatio = atoi(argv[i]);
      if (options.teamKillerKickRatio < 0) {
	 options.teamKillerKickRatio = 0;
	 std::cerr << "disabling team killer kick ratio";
      }
    } else if (strcmp(argv[i], "-userdb") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      userDatabaseFile = argv[i];
      std::cerr << "using userDB file \"" << argv[i] << "\"\n";
    } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0) {
      printVersion();
      exit(0);
    } else if (strcmp(argv[i], "-vars") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.bzdbVars = argv[i];
    } else if (strcmp(argv[i], "-vetoTime") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.vetoTime = (unsigned short int)atoi(argv[i]);
    } else if (strncmp(argv[i], "-votePercentage", 15) == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.votePercentage = (float)atof(argv[i]);
    } else if (strncmp(argv[i], "-voteRepeatTime", 15) == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.voteRepeatTime = (unsigned short int)atoi(argv[i]);
    } else if (strncmp(argv[i], "-votesRequired", 15) == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.votesRequired = (unsigned short int)atoi(argv[i]);
    } else if (strncmp(argv[i], "-voteTime", 9) == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      options.voteTime = (unsigned short int)atoi(argv[i]);
    } else if (strcmp(argv[i], "-world") == 0) {
       if (++i == argc) {
	 std::cerr << "argument expected for -world\n";
	 usage(argv[0]);
       }
       options.worldFile = argv[i];
       if (options.useTeleporters)
	 std::cerr << "-t is meaningless when using a custom world, ignoring\n";
    } else if (strcmp(argv[i], "-worldsize") == 0) {
      if (++i == argc) {
	std::cerr << "argument expected for \"" << argv[i] << "\"\n";
	usage(argv[0]);
      }
      BZDB.set(StateDatabase::BZDB_WORLDSIZE, string_util::format("%d",atoi(argv[i])*2));
      std::cerr << "using world size of \"" << BZDB.eval(StateDatabase::BZDB_WORLDSIZE) << "\"\n";
    } else {
      std::cerr << "bad argument \"" << argv[i] << "\"\n";
      usage(argv[0]);
    }
  }

  if (options.flagsOnBuildings && !(options.gameStyle & JumpingGameStyle)) {
    std::cerr << "flags on boxes requires jumping\n";
    usage(argv[0]);
  }

  // get player counts.  done after other arguments because we need
  // to ignore counts for rogues if rogues aren't allowed.
  if (playerCountArg > 0 && (!parsePlayerCount(argv[playerCountArg], options) ||
      playerCountArg2 > 0 && !parsePlayerCount(argv[playerCountArg2], options)))
    usage(argv[0]);

  // first disallow flags inconsistent with game style
  if (options.gameStyle & InertiaGameStyle) {
    options.flagCount[Flags::Momentum] = 0;
    options.flagDisallowed[Flags::Momentum] = true;
  }
  if (options.gameStyle & JumpingGameStyle) {
    options.flagCount[Flags::Jumping] = 0;
    options.flagDisallowed[Flags::Jumping] = true;
  }
  if (options.gameStyle & RicochetGameStyle) {
    options.flagCount[Flags::Ricochet] = 0;
    options.flagDisallowed[Flags::Ricochet] = true;
  }
  if (!options.useTeleporters && !options.worldFile) {
    options.flagCount[Flags::PhantomZone] = 0;
    options.flagDisallowed[Flags::PhantomZone] = true;
  }
  bool hasTeam = false;
  for (int p = RedTeam; p <= PurpleTeam; p++) {
    if (options.maxTeam[p] > 1) {
	hasTeam = true;
	break;
    }
  }
  if (!hasTeam) {
    options.flagCount[Flags::Genocide] = 0;
    options.flagDisallowed[Flags::Genocide] = true;
    options.flagCount[Flags::Colorblindness] = 0;
    options.flagDisallowed[Flags::Colorblindness] = true;
    options.flagCount[Flags::Masquerade] = 0;
    options.flagDisallowed[Flags::Masquerade] = true;
  }

  if (options.gameStyle & int(RabbitChaseGameStyle)) {
    for (int i = RedTeam; i <= PurpleTeam; i++)
      options.maxTeam[i] = 0;
  }

  // make table of allowed extra flags
  if (options.numExtraFlags > 0) {
    // now count how many aren't disallowed
    for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin();
	it != FlagType::getFlagMap().end(); ++it)
      if (!options.flagDisallowed[it->second])
	options.numAllowedFlags++;

    // if none allowed then no extra flags either
    if (options.numAllowedFlags == 0) {
      options.numExtraFlags = 0;
    }

    // otherwise make table of allowed flags
    else {
      allowedFlags.clear();
      for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin();
	  it != FlagType::getFlagMap().end(); ++it) {
	FlagType *fDesc = it->second;
	if ((fDesc == Flags::Null) || (fDesc->flagTeam != ::NoTeam))
	  continue;
	if (!options.flagDisallowed[it->second])
	  allowedFlags.push_back(it->second);
      }
    }
  }

  // allocate space for flags
  numFlags = options.numExtraFlags;
  if (options.gameStyle & TeamFlagGameStyle) {
    for (int col = RedTeam; col <= PurpleTeam; col++)
      if (options.maxTeam[col] > 0)
        numFlags += options.numTeamFlags[col];
  }
  for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin();
       it != FlagType::getFlagMap().end(); ++it) {
    numFlags += options.flagCount[it->second];
  }

  flag = new FlagInfo[numFlags];

  // prep flags
  for (i = 0; i < numFlags; i++) {
    flag[i].flag.type = Flags::Null;
    flag[i].flag.status = FlagNoExist;
    flag[i].flag.endurance = FlagNormal;
    flag[i].flag.owner = NoPlayer;
    flag[i].flag.position[0] = 0.0f;
    flag[i].flag.position[1] = 0.0f;
    flag[i].flag.position[2] = 0.0f;
    flag[i].flag.launchPosition[0] = 0.0f;
    flag[i].flag.launchPosition[1] = 0.0f;
    flag[i].flag.launchPosition[2] = 0.0f;
    flag[i].flag.landingPosition[0] = 0.0f;
    flag[i].flag.landingPosition[1] = 0.0f;
    flag[i].flag.landingPosition[2] = 0.0f;
    flag[i].flag.flightTime = 0.0f;
    flag[i].flag.flightEnd = 0.0f;
    flag[i].flag.initialVelocity = 0.0f;
    flag[i].player = -1;
    flag[i].grabs = 0;
    flag[i].required = false;
  }
  int f = 0;
  if (options.gameStyle & TeamFlagGameStyle) {
    if (options.maxTeam[RedTeam] > 0) {
      for (int n = 0; n < options.numTeamFlags[RedTeam]; n++) {
        flag[f].required = true;
        flag[f].flag.type = Flags::RedTeam;
        flag[f].flag.endurance = FlagNormal;
        f++;
      }
    }
    if (options.maxTeam[GreenTeam] > 0) {
      for (int n = 0; n < options.numTeamFlags[GreenTeam]; n++) {
        flag[f].required = true;
        flag[f].flag.type = Flags::GreenTeam;
        flag[f].flag.endurance = FlagNormal;
        f++;
      }
    }
    if (options.maxTeam[BlueTeam] > 0) {
      for (int n = 0; n < options.numTeamFlags[BlueTeam]; n++) {
        flag[f].required = true;
        flag[f].flag.type = Flags::BlueTeam;
        flag[f].flag.endurance = FlagNormal;
        f++;
      }
    }
    if (options.maxTeam[PurpleTeam] > 0) {
      for (int n = 0; n < options.numTeamFlags[PurpleTeam]; n++) {
        flag[f].required = true;
        flag[f].flag.type = Flags::PurpleTeam;
        flag[f].flag.endurance = FlagNormal;
        f++;
      }
    }
  }


  for (FlagTypeMap::iterator it2 = FlagType::getFlagMap().begin(); it2 != FlagType::getFlagMap().end(); ++it2) {
    FlagType *fDesc = it2->second;

    if ((fDesc != Flags::Null) && (fDesc->flagTeam == NoTeam)) {
      if (options.flagCount[it2->second] > 0) {
	  for (int j = 0; j < options.flagCount[it2->second]; j++) {
		  if (setRequiredFlag(flag[f], it2->second))
			f++;
	  }
	  options.gameStyle |= int(SuperFlagGameStyle);
      }
    }
  }
  for (; f < numFlags; f++) {
    flag[f].required = allFlagsOut;
    options.gameStyle |= int(SuperFlagGameStyle);
  }

  // debugging
  // print style
  DEBUG1("style: %x\n", options.gameStyle);
  if (options.gameStyle & int(TeamFlagGameStyle))
    DEBUG1("  capture the flag\n");
  if (options.gameStyle & int(RabbitChaseGameStyle))
    DEBUG1("  rabbit chase\n");
  if (options.gameStyle & int(SuperFlagGameStyle))
    DEBUG1("  super flags allowed\n");
  if (options.gameStyle & int(JumpingGameStyle))
    DEBUG1("  jumping allowed\n");
  if (options.gameStyle & int(InertiaGameStyle))
    DEBUG1("  inertia: %f, %f\n", options.linearAcceleration, options.angularAcceleration);
  if (options.gameStyle & int(RicochetGameStyle))
    DEBUG1("  all shots ricochet\n");
  if (options.gameStyle & int(ShakableGameStyle))
    DEBUG1("  shakable bad flags: timeout=%f, wins=%i\n",
	  0.1f * float(options.shakeTimeout), options.shakeWins);
  if (options.gameStyle & int(AntidoteGameStyle))
    DEBUG1("  antidote flags\n");
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
