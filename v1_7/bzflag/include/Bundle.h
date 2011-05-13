#ifndef BZF_BUNDLE_H
#define BZF_BUNDLE_H

#ifdef WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include <vector>
#include <map>
#include "common.h"

typedef std::map<std::string, std::string> BundleStringMap;

class Bundle
{
public:
	std::string getLocalString(const std::string &key);
	std::string formatMessage(const std::string &key, const std::vector<std::string> *parms);

private:
	typedef enum { tERROR, tCOMMENT, tMSGID, tMSGSTR, tAPPEND } TLineType;

	Bundle(const Bundle *pBundle);
	Bundle(const Bundle &xBundle);
	Bundle& operator=(const Bundle &xBundle);
	void load(const std::string &path);
	TLineType parseLine(const std::string &line, std::string &data);
	void ensureNormalText(std::string &msg);
	BundleStringMap mappings;

	friend class BundleMgr;
};

#endif
// ex: shiftwidth=2 tabstop=8