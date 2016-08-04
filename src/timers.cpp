#include <cpr/cpr.h>
#include <json/json.h>
#include <iomanip>
#include <sstream>
#include "lynxbot.h"
#include "timers.h"

#define MAX_URL 128

static const char *UPTIME_API = "https://api.twitch.tv/kraken/streams/";

static time_t bot_time;
static time_t chan_time;

/* init_timers: initialize bot and channel start timers */
void init_timers(const char *channel)
{
	bot_time = time(NULL);
	check_channel(channel);
}

/* check_channel: update stream start timer */
void check_channel(const char *channel)
{
	cpr::Response resp;
	Json::Reader reader;
	Json::Value val;
	char url[MAX_URL];
	struct tm start;
	std::istringstream ss;

	_sprintf(url, MAX_URL, "%s%s", UPTIME_API, channel);
	resp = cpr::Get(cpr::Url(url), cpr::Header{{ "Connection", "close" }});

	printf("%s\n", resp.text.c_str());
	if (!reader.parse(resp.text, val)) {
		fprintf(stderr, "could not parse uptime response\n");
		return;
	}

	if (val["stream"].isNull()) {
		chan_time = 0;
		return;
	}

	ss = std::istringstream(val["stream"]["created_at"].asString());
#ifdef __linux__
	ss.imbue(std::locale("en_US.utf-8"));
#endif
#ifdef _WIN32
	ss.imbue(std::locale("en-US"));
#endif
	ss >> std::get_time(&start, "%Y-%m-%dT%H:%M:%S");
	start.tm_isdst = 0;
	chan_time = std::mktime(&start);
}

/* bot_uptime: return how long bot has been running */
time_t bot_uptime()
{
	return time(NULL) - bot_time;
}

/* channel_uptime: return how long stream has been live */
time_t channel_uptime()
{
	if (chan_time == 0)
		return 0;

	time_t now;
	struct tm curr;

	now = time(NULL);
#ifdef __linux__
	curr = *std::gmtime(&now);
#endif
#ifdef _WIN32
	gmtime_s(&curr, &now);
#endif
	curr.tm_isdst = 0;
	now = std::mktime(&curr);

	return now - chan_time;
}