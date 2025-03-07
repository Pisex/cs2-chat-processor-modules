#include <stdio.h>
#include "cp_personal.h"
#include "metamod_oslink.h"
#include "schemasystem/schemasystem.h"

cp_personal g_cp_personal;
PLUGIN_EXPOSE(cp_personal, g_cp_personal);
IVEngineServer2* engine = nullptr;
CGameEntitySystem* g_pGameEntitySystem = nullptr;
CEntitySystem* g_pEntitySystem = nullptr;
CGlobalVars *gpGlobals = nullptr;

IUtilsApi* g_pUtils;
IPlayersApi* g_pPlayers;
IChatProcessorApi* g_pCP;

struct Setting
{
	std::string szPrefix;
	std::string szPrefixColor;
	std::string szNameColor;
	std::string szMessageColor;
};


std::map<std::string, Setting> g_mapPrefixesFlags;
std::map<uint64, Setting> g_mapPrefixes;

CGameEntitySystem* GameEntitySystem()
{
	return g_pUtils->GetCGameEntitySystem();
}

void StartupServer()
{
	g_pGameEntitySystem = GameEntitySystem();
	g_pEntitySystem = g_pUtils->GetCEntitySystem();
	gpGlobals = g_pUtils->GetCGlobalVars();
}

bool cp_personal::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	g_SMAPI->AddListener( this, this );

	return true;
}

void LoadConfig()
{
	KeyValues* hKv = new KeyValues("Personal");
	const char *pszPath = "addons/configs/chat_processor/personal.ini";

	if (!hKv->LoadFromFile(g_pFullFileSystem, pszPath))
	{
		g_pUtils->ErrorLog("[%s] Failed to load %s", g_PLAPI->GetLogTag(), pszPath);
		return;
	}
	FOR_EACH_TRUE_SUBKEY(hKv, pValue)
	{
		const char* szIdentifier = pValue->GetName();
		g_mapPrefixes[std::stoull(szIdentifier)] = {
			pValue->GetString("tag", ""),
			pValue->GetString("tag_color", ""),
			pValue->GetString("name_color", ""),
			pValue->GetString("chat_color", "")
		};
	}
}

bool cp_personal::Unload(char *error, size_t maxlen)
{
	ConVar_Unregister();
	
	return true;
}

bool OnChatMesssage(int iSlot, std::string &szName, std::string &szMessage)
{
	for (auto it = g_mapPrefixes.begin(); it != g_mapPrefixes.end(); ++it)
	{
		if (it->first == g_pPlayers->GetSteamID64(iSlot))
		{
			szName = it->second.szPrefixColor + it->second.szPrefix + it->second.szNameColor + szName;
			szMessage = it->second.szMessageColor + szMessage;
			return true;
		}
	}
	return true;
}

void cp_personal::AllPluginsLoaded()
{
	char error[64];
	int ret;
	g_pUtils = (IUtilsApi *)g_SMAPI->MetaFactory(Utils_INTERFACE, &ret, NULL);
	if (ret == META_IFACE_FAILED)
	{
		g_SMAPI->Format(error, sizeof(error), "Missing Utils system plugin");
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}
	g_pPlayers = (IPlayersApi *)g_SMAPI->MetaFactory(PLAYERS_INTERFACE, &ret, NULL);
	g_pCP = (IChatProcessorApi *)g_SMAPI->MetaFactory(CP_INTERFACE, &ret, NULL);
	if (ret == META_IFACE_FAILED)
	{
		g_SMAPI->Format(error, sizeof(error), "Missing Chat Processor plugin");
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}
	LoadConfig();
	g_pCP->HookOnChatMessage(g_PLID, OnChatMesssage);
	g_pUtils->StartupServer(g_PLID, StartupServer);
}

///////////////////////////////////////
const char* cp_personal::GetLicense()
{
	return "GPL";
}

const char* cp_personal::GetVersion()
{
	return "1.0";
}

const char* cp_personal::GetDate()
{
	return __DATE__;
}

const char *cp_personal::GetLogTag()
{
	return "cp_personal";
}

const char* cp_personal::GetAuthor()
{
	return "Pisex";
}

const char* cp_personal::GetDescription()
{
	return "cp_personal";
}

const char* cp_personal::GetName()
{
	return "[Chat Processor] Personal";
}

const char* cp_personal::GetURL()
{
	return "https://discord.gg/g798xERK5Y";
}
