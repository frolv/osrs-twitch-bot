#include <algorithm>
#include <cpr/cpr.h>
#include <tw/oauth.h>
#include <utils.h>
#include "command.h"
#include "../CmdHandler.h"
#include "../option.h"
#include "../sed.h"

#define MAX_URL 128

/* full name of the command */
CMDNAME("status");
/* description of the command */
CMDDESCR("set channel status");
/* command usage synopsis */
CMDUSAGE("status [-a] [-s SEDCMD] [STATUS]");

static const char *TWITCH_API = "https://api.twitch.tv/kraken/channels/";

static int curr_status(char *buf, const char *channel, const cpr::Header *head);
static int set_status(char *out, const char *channel, const char *status,
		const cpr::Header *head);

/* status: set channel status */
int CmdHandler::status(char *out, struct command *c)
{
	const cpr::Header head{{ "Accept", "application/vnd.twitchtv.v3+json" },
		{ "Authorization", "OAuth " + std::string(m_token) }};

	char buf[MAX_MSG];
	char sedcmd[MAX_MSG];
	int append, sedflag;

	int opt;
	static struct l_option long_opts[] = {
		{ "append", NO_ARG, 'a' },
		{ "help", NO_ARG, 'h' },
		{ "sed", REQ_ARG, 's' },
		{ 0, 0, 0 }
	};

	if (!P_ALMOD(c->privileges)) {
		PERM_DENIED(out, c->nick, c->argv[0]);
		return EXIT_FAILURE;
	}

	opt_init();
	append = sedflag = 0;
	buf[0] = '\0';
	while ((opt = l_getopt_long(c->argc, c->argv, "as:", long_opts)) != EOF) {
		switch (opt) {
		case 'a':
			append = 1;
			break;
		case 'h':
			HELPMSG(out, CMDNAME, CMDUSAGE, CMDDESCR);
			return EXIT_SUCCESS;
		case 's':
			sedflag = 1;
			strcpy(sedcmd, l_optarg);
			break;
		case '?':
			_sprintf(out, MAX_MSG, "%s", l_opterr());
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}

	if (append && sedflag) {
		_sprintf(out, MAX_MSG, "%s: cannot combine -s and -a",
				c->argv[0]);
		return EXIT_FAILURE;
	}

	/* get the current status if no arg provided, appending or sed */
	if (l_optind == c->argc || append || sedflag) {
		if (!curr_status(buf, m_channel, &head)) {
			_sprintf(out, MAX_MSG, "%s: %s", c->argv[0], buf);
			return EXIT_FAILURE;
		}
	}

	if (l_optind == c->argc) {
		if (append) {
			_sprintf(out, MAX_MSG, "%s: no text to append",
					c->argv[0]);
			return EXIT_FAILURE;
		} else if (sedflag) {
			if (!sed(buf, MAX_MSG, buf, sedcmd)) {
				_sprintf(out, MAX_MSG, "%s: %s",
						c->argv[0], buf);
				return EXIT_FAILURE;
			}
		} else {
			_sprintf(out, MAX_MSG, "[STATUS] Current status for %s "
					"is \"%s\".", m_channel, buf);
			return EXIT_SUCCESS;
		}
	} else if (sedflag) {
		_sprintf(out, MAX_MSG, "%s: cannot provide status with -s",
				c->argv[0]);
		return EXIT_FAILURE;
	}

	if (append)
		strcat(buf, " ");
	argvcat(buf + strlen(buf), c->argc, c->argv, l_optind, 1);

	return set_status(out, m_channel, buf, &head);
}

/* curr_status: get current status of channel */
static int curr_status(char *buf, const char *channel, const cpr::Header *head)
{
	cpr::Response resp;
	Json::Reader reader;
	Json::Value response;
	char url[MAX_URL];
	char *s;

	_sprintf(url, MAX_URL, "%s%s", TWITCH_API, channel);
	resp = cpr::Get(cpr::Url(url), *head);

	if (reader.parse(resp.text, response)) {
		if (response.isMember("error")) {
			_sprintf(buf, MAX_MSG, "%s",
					response["error"].asCString());
			for (s = buf; *s; ++s)
				*s = tolower(*s);
			return 0;
		}
		_sprintf(buf, MAX_MSG, "%s", response["status"].asCString());
		return 1;
	}
	_sprintf(buf, MAX_MSG, "could not parse channel data");
	return 0;
}

/* curr_status: set new status of channel */
static int set_status(char *out, const char *channel, const char *status,
		const cpr::Header *head)
{
	cpr::Response resp;
	Json::Reader reader;
	Json::Value response;
	char body[MAX_MSG];
	char url[MAX_URL];
	char *s;

	_sprintf(body, MAX_MSG, "channel[status]=%s",
			tw::pencode(status, " ").c_str());
	for (s = body; *s; ++s) {
		if (*s == ' ')
			*s = '+';
	}

	_sprintf(url, MAX_URL, "%s%s", TWITCH_API, channel);
	resp = cpr::Put(cpr::Url(url), cpr::Body(body), *head);

	if (reader.parse(resp.text, response)) {
		if (response.isMember("error")) {
			_sprintf(out, MAX_MSG, "%s: %s", CMDNAME,
					response["error"].asCString());
			for (s = out; *s; ++s)
				*s = tolower(*s);
			return EXIT_FAILURE;
		}
		_sprintf(out, MAX_MSG, "[STATUS] Channel status changed to "
				"\"%s\".", response["status"].asCString());
		return EXIT_SUCCESS;
	}
	_sprintf(out, MAX_MSG, "could not parse channel data");
	return EXIT_FAILURE;
}
