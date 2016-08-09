#include <string.h>
#include "command.h"
#include "../CmdHandler.h"
#include "../option.h"
#include "../sed.h"
#include "../stringparse.h"

/* full name of the command */
CMDNAME("editrec");
/* description of the command */
CMDDESCR("modify a recurring message");
/* command usage synopsis */
CMDUSAGE("editrec [-c INTERVAL] [-s SEDCMD] ID [MSG]");

/* interval of command */
static time_t interval;

static int edit_recurring(char *out, EventManager *evt, int id,
		struct command *c, const char *resp);

/* editrec: modify a recurring message */
int CmdHandler::editrec(char *out, struct command *c)
{
	int app, sedflag;
	int64_t id;
	char sedcmd[MAX_MSG];
	char response[MAX_MSG];

	int opt;
	static struct l_option long_opts[] = {
		{ "append", NO_ARG, 'a'},
		{ "cooldown", REQ_ARG, 'c'},
		{ "interval", REQ_ARG, 'c'},
		{ "help", NO_ARG, 'h' },
		{ "sed", REQ_ARG, 's' },
		{ 0, 0, 0 }
	};

	if (!P_ALMOD(c->privileges)) {
		PERM_DENIED(out, c->nick, c->argv[0]);
		return EXIT_FAILURE;
	}

	opt_init();
	app = sedflag = 0;
	interval = -1;
	response[0] = '\0';
	while ((opt = l_getopt_long(c->argc, c->argv, "ac:s:", long_opts))
			!= EOF) {
		switch (opt) {
		case 'a':
			app = 1;
			break;
		case 'c':
			/* user provided a cooldown */
			if (!parsenum(l_optarg, &interval)) {
				_sprintf(out, MAX_MSG, "%s: invalid number: %s",
						c->argv[0], l_optarg);
				return EXIT_FAILURE;
			}
			if (interval < 0) {
				_sprintf(out, MAX_MSG, "%s: interval cannot be "
						"negative", c->argv[0]);
				return EXIT_FAILURE;
			}
			interval *= 60;
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

	if (l_optind == c->argc) {
		USAGEMSG(out, CMDNAME, CMDUSAGE);
		return EXIT_FAILURE;
	}
	if (app && sedflag) {
		_sprintf(out, MAX_MSG, "%s: cannot use -a with -s", c->argv[0]);
		return EXIT_FAILURE;
	}
	if (!parsenum(c->argv[l_optind], &id)) {
		_sprintf(out, MAX_MSG, "%s: invalid number: %s",
				c->argv[0], c->argv[l_optind]);
		return EXIT_FAILURE;
	}

	if (id < 1 || (size_t)id > m_evtp->messages()->size()) {
		_sprintf(out, MAX_MSG, "%s: invalid ID: %ld", c->argv[0], id);
		return EXIT_FAILURE;
	}

	if (interval != -1) {
		if (interval % 300) {
			_sprintf(out, MAX_MSG, "%s: interval must be a multiple "
					"of 5 mins", c->argv[0]);
			return EXIT_FAILURE;
		} else if (interval > 3600) {
			_sprintf(out, MAX_MSG, "%s: interval cannot be longer "
					"than 60 mins", c->argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (app || sedflag)
		strcpy(response, m_evtp->messages()->at(id - 1).first.c_str());

	if (sedflag) {
		if (l_optind != c->argc - 1) {
			_sprintf(out, MAX_MSG, "%s: cannot provide message "
					"with -s", c->argv[0]);
			return EXIT_FAILURE;
		}
		if (!sed(response, MAX_MSG, response, sedcmd)) {
			_sprintf(out, MAX_MSG, "%s: %s", c->argv[0], response);
			return EXIT_FAILURE;
		}
	}

	if (app) {
		if (l_optind == c->argc - 1) {
			_sprintf(out, MAX_MSG, "%s: nothing to append",
					c->argv[0]);
			return EXIT_FAILURE;
		}
		strcat(response, " ");
	}
	argvcat(response + strlen(response), c->argc, c->argv, l_optind + 1, 1);

	return edit_recurring(out, m_evtp, id, c, response);
}

/* edit_recurring: edit a recurring message */
static int edit_recurring(char *out, EventManager *evt, int id,
		struct command *c, const char *resp)
{
	evt->editmsg(id, *resp ? resp : NULL, interval);
	_sprintf(out, MAX_MSG, "@%s, recurring message %d", c->nick, id);
	if (!*resp && interval == -1) {
		strcat(out, " was unchanged.");
		return EXIT_SUCCESS;
	}
	strcat(out, " has been changed to");
	out = strchr(out, '\0');
	if (*resp) {
		_sprintf(out, MAX_MSG, " \"%s\"%s", resp,
				interval == -1 ? "" : ", with ");
		out = strchr(out, '\0');
	}
	if (interval != -1)
		_sprintf(out, MAX_MSG, " a %ld min interval", interval / 60);
	strcat(out, ".");
	return EXIT_SUCCESS;
}
