#pragma once

#include "CustomCommandHandler.h"
#include "GEReader.h"
#include "TimerManager.h"
#include "cmdmodules\SelectionWheel.h"

class CustomCommandHandler;
class GEReader;
class TimerManager;
class SelectionWheel;

typedef std::map<std::string, std::string(CommandHandler::*)(const std::string &, const std::string &)> commandMap;

class CommandHandler {

	public:
		CommandHandler();
		~CommandHandler();
		std::string processCommand(const std::string &nick, const std::string &fullCmd, bool privileges);
		std::string processResponse(const std::string &message);
	private:
		commandMap m_defaultCmds;
		CustomCommandHandler m_customCmds;
		GEReader m_GEReader;
		TimerManager m_timerManager;
		SelectionWheel m_wheel;
		Json::Value m_responses;
		bool m_responding;
		const std::string CML_HOST = "crystalmathlabs.com";
		const std::string CML_EHP_AHI = "/tracker/api.php?type=virtualhiscoresatplayer&page=timeplayed&player=";
		const std::string RS_HOST = "services.runescape.com";
		const std::string RS_HS_API = "/m=hiscore_oldschool/index_lite.ws?player=";
		const std::string EXCHANGE_HOST = "api.rsbuddy.com";
		const std::string EXCHANGE_API = "/grandExchange?a=guidePrice&i=";
		const std::string m_eightballResponses[21] = { "It is certain", "It is decidedly so", "Without a doubt", "Yes, definitely",
			"You may rely on it", "As I see it, yes", "Most likely", "Outlook good", "Yes", "Signs point to yes",
			"Reply hazy try again", "Ask again later", "Better not tell you now", "Cannot predict now", "Concentrate and ask again",
			"Don't count on it", "My reply is no", "My sources say no", "Outlook not so good", "Very doubtful",
			"About the same chance as Yekouri getting 200m mining" };
		std::string ehpFunc(const std::string &nick, const std::string &fullCmd);
		std::string levelFunc(const std::string &nick, const std::string &fullCmd);
		std::string geFunc(const std::string &nick, const std::string &fullCmd);
		std::string calcFunc(const std::string &nick, const std::string &fullCmd);
		std::string cmlFunc(const std::string &nick, const std::string &fullCmd);
		std::string wheelFunc(const std::string &nick, const std::string &fullCmd);
		std::string eightballFunc(const std::string &nick, const std::string &fullCmd);
		std::string addcomFunc(const std::string &nick, const std::string &fullCmd);
		std::string delcomFunc(const std::string &nick, const std::string &fullCmd);
		std::string extractCMLData(const std::string &httpResp, const std::string &rsn);
		std::string extractHSData(const std::string &httpResp, uint8_t skillID);
		std::string extractGEData(const std::string &httpResp);

};