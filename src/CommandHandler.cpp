#include "stdafx.h"

CommandHandler::CommandHandler() :m_customCmds(&m_defaultCmds, &m_timerManager, m_wheel.cmd()) {

	// initializing pointers to all default commands
	m_defaultCmds["ehp"] = &CommandHandler::ehpFunc;
	m_defaultCmds["level"] = &CommandHandler::levelFunc;
	m_defaultCmds["lvl"] = &CommandHandler::levelFunc;
	m_defaultCmds["ge"] = &CommandHandler::geFunc;
	m_defaultCmds["calc"] = &CommandHandler::calcFunc;
	m_defaultCmds["cml"] = &CommandHandler::cmlFunc;
	m_defaultCmds["8ball"] = &CommandHandler::eightballFunc;
	m_defaultCmds["strawpoll"] = &CommandHandler::strawpollFunc;
	m_defaultCmds["sp"] = &CommandHandler::strawpollFunc;
	m_defaultCmds["addcom"] = &CommandHandler::addcomFunc;
	m_defaultCmds["delcom"] = &CommandHandler::delcomFunc;
	m_defaultCmds["editcom"] = &CommandHandler::editcomFunc;

	if (!m_customCmds.isActive()) {
		std::cerr << " Custom commands will be disabled for this session." << std::endl;
		std::cin.get();
	}

	// read all responses from file
	m_responding = utils::readJSON("responses.json", m_responses);
	if (m_responding) {
		// add response cooldowns to TimerManager
		for (auto &val : m_responses["responses"]) {
			m_timerManager.add(val["name"].asString(), val["cooldown"].asInt());
		}
	}
	else {
		std::cerr << "Failed to read responses.json. Responses disabled for this session.";
		std::cin.get();
	}

	// set all command cooldowns
	for (auto &p : m_defaultCmds) {
		m_timerManager.add(p.first);
	}
	m_timerManager.add(m_wheel.name(), 10);

	// read extra 8ball responses
	std::ifstream reader(utils::getApplicationDirectory() + "\\extra8ballresponses.txt");
	if (!reader.is_open()) {
		std::cerr << "Could not read extra8ballresponses.txt" << std::endl;
	}
	std::string line;
	while (std::getline(reader, line)) {
		m_eightballResponses.push_back(line);
	}
	reader.close();

}

CommandHandler::~CommandHandler() {}

std::string CommandHandler::processCommand(const std::string &nick, const std::string &fullCmd, bool privileges) {

	std::string output = "";

	// the command is the first part of the string up to the first space
	std::string cmd = fullCmd.substr(0, fullCmd.find(' '));

	if (m_defaultCmds.find(cmd) != m_defaultCmds.end() && (privileges || m_timerManager.ready(cmd))) {
		output += (this->*m_defaultCmds[cmd])(nick, fullCmd, privileges);
		m_timerManager.setUsed(cmd);
	}
	else if (m_wheel.isActive() && cmd == m_wheel.cmd() && (privileges || m_timerManager.ready(m_wheel.name()))) {
		output += wheelFunc(nick, fullCmd, privileges);
		m_timerManager.setUsed(m_wheel.name());
	}
	else if (m_customCmds.isActive()) {
		Json::Value *customCmd = m_customCmds.getCom(cmd);
		if (!customCmd->empty() && (privileges || m_timerManager.ready((*customCmd)["cmd"].asString()))) {
			output += (*customCmd)["response"].asString();
			m_timerManager.setUsed((*customCmd)["cmd"].asString());
		}
		else {
			std::cerr << "Invalid command or is on cooldown: " << cmd << std::endl << std::endl;
		}
	}

	return output;

}

std::string CommandHandler::processResponse(const std::string &message) {

	if (!m_responding) {
		return "";
	}

	for (auto &val : m_responses["responses"]) {

		std::string name = val["name"].asString();
		std::string regex = val["regex"].asString();

		std::regex responseRegex(regex, std::regex_constants::ECMAScript | std::regex_constants::icase);
		std::smatch match;
		if (std::regex_search(message.begin(), message.end(), match, responseRegex) && m_timerManager.ready(name)) {
			m_timerManager.setUsed(name);
			return val["response"].asString();
		}

	}

	return "";

}

std::string CommandHandler::ehpFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {
	
	std::vector<std::string> tokens;
	utils::split(fullCmd, ' ', tokens);

	if (tokens.size() == 2) {
		// a username was provided
		std::string rsn = tokens[1];
		std::replace(rsn.begin(), rsn.end(), '-', '_');
		const std::string httpResp = HTTPGet(CML_HOST, CML_EHP_AHI + rsn);
		std::clog << httpResp << std::endl << std::endl;
		return "[EHP] " + extractCMLData(httpResp, rsn);

	}
	else if (tokens.size() == 1) {
		return "EHP stands for efficient hours played. You earn 1 EHP whenever you gain a certain amount of experience in a skill, \
			depending on your level. You can find XP rates here: http://crystalmathlabs.com/tracker/suppliescalc.php";
	}
	else {
		return "Invalid syntax. Use \"$ehp [RSN]\".";
	}

}

std::string CommandHandler::levelFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {

	std::vector<std::string> tokens;
	utils::split(fullCmd, ' ', tokens);

	if (tokens.size() == 3) {

		if (skillMap.find(tokens[1]) == skillMap.end() && skillNickMap.find(tokens[1]) == skillNickMap.end()) {
			return "Invalid skill name.";
		}
		uint8_t skillID = skillMap.find(tokens[1]) == skillMap.end() ? skillNickMap.find(tokens[1])->second : skillMap.find(tokens[1])->second;

		std::string rsn = tokens[2];
		std::replace(rsn.begin(), rsn.end(), '-', '_');

		const std::string httpResp = HTTPGet(RS_HOST, RS_HS_API + rsn);
		std::clog << httpResp << std::endl;
		if (httpResp.find("404 - Page not found") != std::string::npos) {
			return "Player not found on hiscores.";
		}

		std::string nick = getSkillNick(skillID);
		std::transform(nick.begin(), nick.end(), nick.begin(), ::toupper);

		return "[" + nick + "] Name: " + rsn + ", " + extractHSData(httpResp, skillID);
	}
	else {
		return "Invalid syntax. Use \"$lvl SKILL RSN\".";
	}

}

std::string CommandHandler::geFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {

	if (!m_GEReader.active()) {
		return "";
	}

	if (fullCmd.length() < 4) {
		return "No item name provided.";
	}

	std::string itemName = fullCmd.substr(3);
	std::replace(itemName.begin(), itemName.end(), '_', ' ');

	Json::Value item = m_GEReader.getItem(itemName);

	if (item.empty()) {
		return "Item not found: " + itemName + ".";
	}

	const std::string httpResp = HTTPGet(EXCHANGE_HOST, EXCHANGE_API + std::to_string(item["id"].asInt()));
	std::clog << httpResp;

	return "[GE] " + item["name"].asString() + ": " + extractGEData(httpResp) + " gp.";

}

std::string CommandHandler::calcFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {

	if (fullCmd.length() < 6) {
		return "Invalid mathematical expression.";
	}

	std::string expr = fullCmd.substr(5);
	// remove all whitespace
	expr.erase(std::remove_if(expr.begin(), expr.end(), isspace), expr.end());
	
	std::string result;

	try {
		ExpressionParser exprP(expr);
		exprP.tokenizeExpr();
		double res = exprP.eval();
		result += std::to_string(res);
	}
	catch (std::runtime_error &e) {
		result = e.what();
	}

	if (result == "inf" || result == "-nan(ind)") {
		result = "Error: division by 0.";
	}

	return "[CALC] " + result;

}

std::string CommandHandler::cmlFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {
	return "[CML] http://" + CML_HOST;
}

std::string CommandHandler::wheelFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {

	std::vector<std::string> tokens;
	utils::split(fullCmd, ' ', tokens);

	if (tokens.size() == 1) {
		return m_wheel.name() + ": " + m_wheel.desc() + " " + m_wheel.usage();
	}
	if (tokens.size() > 2 || (!m_wheel.valid(tokens[1]) && tokens[1] != "check")) {
		return "Invalid syntax. " + m_wheel.usage();
	}
	std::string output = "@" + nick + ", ";

	if (tokens[1] == "check") {
		std::string selection = m_wheel.selection(nick);
		output += selection.empty() ? "you have not been assigned anything." : "you are currently assigned " + selection + ".";
	}
	else if (!m_wheel.ready(nick)) {
		output += "you have already been assigned something!";
	}
	else {
		output += "your entertainment for tonight is " + m_wheel.choose(nick, tokens[1]) + ".";
	}

	return output;

}

std::string CommandHandler::eightballFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {
	if (fullCmd.length() < 6) {
		return "[8 BALL] Ask me a question.";
	}
	if (fullCmd.find("?") == std::string::npos) {
		return "";
	}
	std::srand(static_cast<uint32_t>(std::time(nullptr)));
	size_t ind = std::rand() % m_eightballResponses.size();
	return "[8 BALL] @" + nick + ", " + m_eightballResponses[ind] + ".";
}

std::string CommandHandler::strawpollFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {

	if (!privileges) return "";

	Json::Value poll, options(Json::arrayValue), response;
	std::string output = "[SPOLL] ";

	std::vector<std::string> tokens;
	utils::split(fullCmd, ' ', tokens);
	if (tokens.size() < 2) {
		return output + "Not enough arguments given.";
	}
	std::string::size_type reqStart = tokens[0].length() + 1;
	bool binary = false, multi = false, captcha = false;

	size_t i = 1;
	while (i < tokens.size() && utils::startsWith(tokens[i], "-") && tokens[i].length() > 1) {
		std::regex flagRegex("^-[bcm]+$");
		std::smatch match;
		const std::string flag = tokens[i];
		if (!std::regex_match(flag.begin(), flag.end(), match, flagRegex)) {
			return output + "Invalid flag provided: " + flag.substr(1);
		}
		if (flag.find("b") != std::string::npos) binary = true;
		if (flag.find("c") != std::string::npos) captcha = true;
		if (flag.find("m") != std::string::npos) multi = true;
		reqStart += flag.length() + 1;
		++i;
	}

	if (reqStart >= fullCmd.size()) {
		return output + "Not enough arguments given.";
	}
	const std::string req = fullCmd.substr(reqStart);
	tokens.clear();
	utils::split(req, '|', tokens);

	if (binary && tokens.size() != 1) {
		return output + "Must provide question only for binary poll.";
	}
	if (!binary && tokens.size() < 3) {
		return output + "Poll must have a question and at least two answers.";
	}
	
	if (binary) {
		options.append("yes");
		options.append("no");
	}
	else {
		for (size_t i = 1; i < tokens.size(); ++i) {
			options.append(tokens[i]);
		}
	}

	poll["title"] = tokens[0];
	poll["options"] = options;
	poll["captcha"] = captcha;
	poll["multi"] = multi;

	Json::FastWriter fw;
	const std::string httpResp = HTTPPost(STRAWPOLL_HOST, STRAWPOLL_API, "application/json", fw.write(poll));

	std::regex jsonRegex("(\\{.+\\})");
	std::smatch match;

	if (std::regex_search(httpResp.begin(), httpResp.end(), match, jsonRegex)) {
		const std::string json = match[1];
		Json::Reader reader;
		reader.parse(json, response);
		if (!response.isMember("id")) {
			output += "Poll could not be created.";
		}
		else {
			output += "Poll created : http://" + STRAWPOLL_HOST + "/" + response["id"].asString();
		}
	}
	else {
		output += "Error connecting to server.";
	}

	return output;

}

std::string CommandHandler::addcomFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {
	
	if (!privileges || fullCmd.size() < 8) return "";
	if (!m_customCmds.isActive()) {
		return "Custom commands are currently disabled.";
	}

	command c;
	try {
		// first 7 characters are "addcom "
		c = buildCom(fullCmd.substr(7));
	}
	catch (std::invalid_argument &e) {
		std::cerr << e.what();
		return "@" + nick + ", invalid number provided.";
	}
	catch (std::runtime_error &e) {
		return "@" + nick + ", " + e.what();
	}

	if (!m_customCmds.validName(c.cmd)) {
		return "@" + nick + ", invalid command name: " + c.cmd;
	}
	if (c.response.empty()) {
		return "@" + nick + ", no response provided for command $" + c.cmd + ".";
	}
	if (c.cooldown == -1) {
		c.cooldown = 15;
	}

	m_customCmds.addCom(c.cmd, c.response, c.cooldown);
	return "@" + nick + ", command $" + c.cmd + " has been added with a " + std::to_string(c.cooldown) + "s cooldown.";

}

std::string CommandHandler::delcomFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {

	if (!privileges) return "";
	if (!m_customCmds.isActive()) {
		return "Custom commands are currently disabled.";
	}

	std::vector<std::string> tokens;
	utils::split(fullCmd, ' ', tokens);

	if (tokens.size() == 2) {
		return "@" + nick + ", command $" + tokens[1] + (m_customCmds.delCom(tokens[1]) ? " has been deleted." : " not found.");
	}
	else {
		return "Invalid syntax. Use \"$delcom COMMAND\".";
	}

}

std::string CommandHandler::editcomFunc(const std::string &nick, const std::string &fullCmd, bool privileges) {

	if (!privileges) return "";
	if (!m_customCmds.isActive()) {
		return "Custom commands are currently disabled.";
	}

	command c;
	try {
		// first 7 characters are "editcom "
		c = buildCom(fullCmd.substr(8));
	}
	catch (std::invalid_argument &e) {
		std::cerr << e.what();
		return "@" + nick + ", invalid number provided.";
	}
	catch (std::runtime_error &e) {
		return "@" + nick + ", " + e.what();
	}

	bool changedResp = !c.response.empty(), changedCd = c.cooldown != -1;

	if (m_customCmds.editCom(c.cmd, c.response, c.cooldown)) {
		std::string message = "@" + nick + ", command $" + c.cmd + " has been changed to ";
		if (changedResp) {
			message += "\"" + c.response + "\"" + (changedCd ? ", with " : ".");
		}
		if (c.cooldown != -1) {
			message += "a " + std::to_string(c.cooldown) + "s cooldown.";
		}
		return message;
	}
	else {
		return "@" + nick + ", command $" + c.cmd + " does not exist.";
	}

}

std::string CommandHandler::extractCMLData(const std::string &httpResp, const std::string &rsn) {

	std::regex dataRegex("(\\d+," + rsn + ",[\\d\\.]+,[\\d\\.]+)", std::regex_constants::ECMAScript | std::regex_constants::icase);
	std::smatch match;
	// find the required data
	if (std::regex_search(httpResp.begin(), httpResp.end(), match, dataRegex)) {

		std::string data = match[1];
		std::vector<std::string> elems;
		// split data into tokens
		utils::split(data, ',', elems);
		std::string ehp = elems[2];

		if (ehp.find(".") != std::string::npos) {
			// truncate to one decimal place
			ehp = ehp.substr(0, ehp.find(".") + 2);
		}
		return "Name: " + elems[1] + ", Rank: " + elems[0] + ", EHP: " + ehp + " (+" + elems[3] + " this week).";

	}
	else {
		return "Player either does not exist or has not been tracked on CML.";
	}

}

std::string CommandHandler::extractHSData(const std::string &httpResp, uint8_t skillID) {

	std::vector<std::string> skills;
	utils::split(httpResp, '\n', skills);
	
	std::vector<std::string> tokens;
	utils::split(skills[skillID], ',', tokens);

	return "Level: " + tokens[1] + ", Exp: " + utils::formatInteger(tokens[2]) + ", Rank: " + utils::formatInteger(tokens[0]) + ".";

}

std::string CommandHandler::extractGEData(const std::string &httpResp) {

	std::regex jsonRegex("(\\{.+\\})");
	std::smatch match;

	if (std::regex_search(httpResp.begin(), httpResp.end(), match, jsonRegex)) {
		const std::string json = match[1];
		return m_GEReader.extractItemPrice(json);
	}
	else {
		return "An error occured. Please try again.";
	}

}

CommandHandler::command CommandHandler::buildCom(const std::string &s) {

	std::vector<std::string> tokens;
	utils::split(s, ' ', tokens);

	if (tokens.size() < 1) {
		throw std::runtime_error("Not enough arguments given.");
	}

	std::string cmd, response;
	std::string::size_type respStart = 0;
	std::time_t cooldown = -1;

	if (tokens[0] == "-c") {

		if (tokens.size() < 3) {
			throw std::runtime_error("Not enough arguments given.");
		}
		// add the -c, a space then the cooldown and a space
		respStart += 3 + tokens[1].length() + 1;
		cooldown = std::stoi(tokens[1]);
		if (cooldown < 0) {
			throw std::runtime_error("Cooldown cannot be negative!");
		}
		cmd = tokens[2];
	}
	else {
		cmd = tokens[0];
	}

	// add the command and a space
	respStart += cmd.length() + 1;
	response = respStart > s.length() ? "" : s.substr(respStart);

	command c;
	c.cmd = cmd;
	c.response = response;
	c.cooldown = cooldown;

	return c;

}