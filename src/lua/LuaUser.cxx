
#include "common.h"

// interface header
#include "LuaUser.h"

// system headers
#include <cctype>
#include <string>
#include <vector>
#include <set>
using std::vector;
using std::string;
using std::set;

// common headers
#include "BzVFS.h"
#include "EventHandler.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "bzfio.h"

// bzflag headers
#include "../bzflag/Downloads.h"

// local headers
#include "LuaClientOrder.h"
#include "LuaInclude.h"
#include "LuaUtils.h"

#include "LuaCallInCheck.h"
#include "LuaCallInDB.h"
#include "LuaCallOuts.h"
#include "LuaUtils.h"
#include "LuaBitOps.h"
#include "LuaDouble.h"
#include "LuaOpenGL.h"
#include "LuaConstGL.h"
#include "LuaConstGame.h"
#include "LuaKeySyms.h"
#include "LuaSpatial.h"
#include "LuaObstacle.h"
#include "LuaScream.h"
#include "LuaURL.h"
#include "LuaVFS.h"
#include "LuaBZDB.h"
#include "LuaPack.h"
#include "LuaExtras.h"
#include "LuaVector.h"
#include "LuaBzMaterial.h"
#include "LuaDynCol.h"
#include "LuaTexMat.h"
#include "LuaPhyDrv.h"


LuaUser* luaUser = NULL;

static const char* sourceFile = "bzUser.lua";


/******************************************************************************/
/******************************************************************************/

void LuaUser::LoadHandler()
{
	if (luaUser) {
		return;
	}

	const string& dir = BZDB.get("luaUserDir");
	if (dir.empty() || (dir[0] == '!')) {
		return;
	}

	new LuaUser();

	if (luaUser->L == NULL) {
		delete luaUser;
	}
}


void LuaUser::FreeHandler()
{
	delete luaUser;
}


/******************************************************************************/
/******************************************************************************/

LuaUser::LuaUser()
: LuaHandle("LuaUser", ORDER_LUA_USER, devMode, true)
{
	luaUser = this;

	if (L == NULL) {
		return;
	}

	// setup the handle pointer
	L2HH(L)->handlePtr = (LuaHandle**)&luaUser;

	if (!SetupEnvironment()) {
		KillLua();
		return;
	}

	const string sourceCode = LoadSourceCode(sourceFile, BZVFS_LUA_USER);
	if (sourceCode.empty()) {
		KillLua();
		return;
	}

	fsRead = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	         BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	         BZVFS_BASIC;
	fsReadAll = BZVFS_LUA_USER  BZVFS_LUA_USER_WRITE
	            BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE
	            BZVFS_BASIC;
	fsWrite    = BZVFS_LUA_USER_WRITE;
	fsWriteAll = BZVFS_LUA_USER_WRITE;

	if (!ExecSourceCode(sourceCode)) {
		KillLua();
		return;
	}

	// register for call-ins
	eventHandler.AddClient(this);
}


LuaUser::~LuaUser()
{
	if (L != NULL) {
		Shutdown();
		KillLua();
	}
	luaUser = NULL;
}


/******************************************************************************/
/******************************************************************************/

void LuaUser::ForbidCallIns()
{
	const string forbidden = BZDB.get("_forbidLuaUser");
	const vector<string> callIns = TextUtils::tokenize(forbidden, ", ");
	for (size_t i = 0; i < callIns.size(); i++) {
		const string& ciName = callIns[i];
		const int ciCode = luaCallInDB.GetCode(ciName);
		if (validCallIns.find(ciCode) != validCallIns.end()) {
			validCallIns.erase(ciCode);
			string realName = ciName;
			if (ciName == "GLReload") {
				realName = "GLContextInit";
			}
			eventHandler.RemoveEvent(this, realName);
			logDebugMessage(0, "LuaUser: %s is forbidden\n", ciName.c_str());
		}
	}
}


/******************************************************************************/
/******************************************************************************/
