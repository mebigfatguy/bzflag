// statTemplates.cpp : Defines the entry point for the DLL application.
//

#include "reportTemplates.h"
#include "bzfsAPI.h"
#include "plugin_utils.h"

double start;
std::string getFileHeader ( void )
{
  start = bz_getCurrentTime();

  std::string serverName = bz_getPublicAddr().c_str();
  if (serverName.size() == 0) {
	  serverName = format("localhost:%d", bz_getPublicPort());
  }

  // HTML 4.01 Strict doctype
  std::string page = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n";
  page += "<html>\n";
  page += "<head>\n";
  page += "<title>Webstats for " + serverName + "</title>\n";
  page += "<style type=\"text/css\">\n";
  page += "th { border: 1px solid black; }\n";
  page += ".odd td { border: 1px solid gray; }\n";
  page += ".even td { border: 1px solid black; }\n";

  page += ".unknown td { background-color: white; }\n";
  page += ".red td { background-color: #FF6347; }\n";
  page += ".blue td { background-color: #6495ED; }\n";
  page += ".green td { background-color: #7FFF00; }\n";
  page += ".purple td { background-color: #EE82EE; }\n";
  page += ".rogue td { background-color: #FFFF00; }\n";
  page += ".observer td { background-color: white; color: #666666; }\n";
  page += ".rabbit td { background-color: #999999; color: white; }\n";
  page += ".hunter td { background-color: #FFA500; }\n";
  page += "</style>\n";
  page += "</head>\n";
  page += "<body>\n";
  page += "<p>Reports for " + serverName + "</p>\n";

  return page;
}

std::string getFileFooter ( void )
{
  return format("<hr>\n<p>Page generated by webReport in %f seconds</p>\n</body>\n</html>",bz_getCurrentTime()-start);
}

std::string getPlayersHeader ( void )
{
  return std::string ("<hr><h2>Players</h2>\n<table>\n\t<tr><th>Callsign</th><th>Wins</th><th>Losses</th><th>TKs</th><th>Status</th></tr>\n");
}

std::string getPlayersLineItem ( bz_BasePlayerRecord *rec, bool evenLine )
{
  std::string code = "";

  code += "\t<tr class=\"";
  if (evenLine) code += "even ";
  else code += "odd ";

  code += tolower(getTeamTextName(rec->team)) + "\">\n";
  code += "\t\t<td>";
  code += rec->callsign.c_str();
  code += "</td>\n";

  if ( rec->team != eObservers )
  {
    code += format("\t\t<td>%d</td>\n\t\t<td>%d</td>\n\t\t<td>%d</td>\n", rec->wins, rec->losses, rec->teamKills);

	code += "\t\t<td>";
    if ( rec->admin )
      code += "Admin";

	if ( rec->spawned )
	{
	  if (rec->admin) code += "/";
      code += "Spawned";
	}

	if ( rec->verified )
	{
	  if (rec->admin || rec->spawned) code += "/";
      code += "Verified";
	}

	if (!rec->admin && !rec->spawned && !rec->verified)
	  code += "&nbsp;";

	code += "</td>\n";
  }
  else
  {
	code += "\t\t<td colspan=\"4\">&nbsp;</td>";
  }

  code += "\n\t</tr>\n";

  return code;
}

std::string getPlayersNoPlayers ( void )
{
  return std::string("\t<tr class=\"odd unknown\"><td colspan=\"6\">There are currently no players online.</td></tr>\n");
}

std::string getPlayersFooter ( void )
{
  return std::string ("</table>\n");
}

std::string getTeamHeader ( bz_eTeamType team )
{
  std::string code = "<b><font";
  code += getTeamFontCode(team);
  code += ">" + getTeamTextName(team) = "</font></b><br>";

  return code;
}

std::string getTeamFooter ( bz_eTeamType team )
{
  return std::string();
}

std::string getTeamFontCode ( bz_eTeamType team )
{
  std::string code = "";
  switch (team)
  {
  case eRedTeam:
    code = "color=#800000";
    break;

  case eGreenTeam:
    code = "color=#008000";
    break;

  case eBlueTeam:
    code = "color=#000080";
    break;

  case ePurpleTeam:
    code = "color=#800080";
    break;

  case eRogueTeam:
    code = "color=#808000";
    break;

  case eObservers:
    code = "color=#808080";
    break;

  case eHunterTeam:
    code = "color=#C35617";
    break;
  
  case eRabbitTeam:
    code = "color=#C0C0C0";
     break;
  }

  return code;
}

std::string getTeamTextName ( bz_eTeamType team )
{
  std::string name = "unknown";

  switch (team)
  {
  case eRedTeam:
    name = "Red";
    break;

  case eGreenTeam:
    name = "Green";
    break;

  case eBlueTeam:
    name = "Blue";
    break;

  case ePurpleTeam:
    name = "Purple";
    break;

  case eRogueTeam:
    name = "Rogue";
    break;

  case eObservers:
    name = "Observer";
    break;

  case eRabbitTeam:
    name = "Rabbit";
    break;
  case eHunterTeam:
    name = "Hunter";
    break;

  }
  return name;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
