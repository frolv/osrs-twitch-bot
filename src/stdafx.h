#pragma once

// default libraries
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sdkddkver.h>
#include <string>
#include <iostream>
#include <regex>
#include <vector>
#include <sstream>
#include <stack>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <time.h>
#include <random>
#include <thread>

// project libraries
#include <json\json.h>
#include <json\json-forwards.h>
#include <ExpressionParser.h>

// project headers
#include "TwitchBot.h"
#include "CommandHandler.h"
#include "CustomCommandHandler.h"
#include "Moderator.h"
#include "Utils.h"
#include "GEReader.h"
#include "TimerManager.h"
#include "cmdmodules\HTTPTools.h"
#include "cmdmodules\SkillMap.h"
#include "cmdmodules\SelectionWheel.h"

// library for winsock2
#pragma comment(lib, "ws2_32.lib")