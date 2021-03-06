#include <string.h>
#include "command.h"
#include "../CmdHandler.h"
#include "../option.h"
#include "../stringparse.h"

/* full name of the command */
CMDNAME("addrec");
/* description of the command */
CMDDESCR("add a recurring message");
/* command usage synopsis */
CMDUSAGE("$addrec [-c INT] MSG");

/* addrec: add a recurring message */
int CmdHandler::addrec(char *out, struct command *c)
{
	time_t cooldown;
	char msg[MAX_MSG];

	int opt;
	static struct l_option long_opts[] = {
		{ "cooldown", REQ_ARG, 'c' },
		{ "help", NO_ARG, 'h' },
		{ "interval", REQ_ARG, 'c' },
		{ 0, 0, 0 }
	};

	if (!P_ALMOD(c->privileges)) {
		PERM_DENIED(out, c->nick, c->argv[0]);
		return EXIT_FAILURE;
	}

	opt_init();
	cooldown = 600;
	while ((opt = l_getopt_long(c->argc, c->argv, "c:", long_opts)) != EOF) {
		switch (opt) {
		case 'c':
			/* user provided a cooldown */
			if (!parsenum(l_optarg, &cooldown)) {
				snprintf(out, MAX_MSG, "%s: invalid number: %s",
						c->argv[0], l_optarg);
				return EXIT_FAILURE;
			}
			if (cooldown < 0) {
				snprintf(out, MAX_MSG, "%s: interval cannot be "
						"negative", c->argv[0]);
				return EXIT_FAILURE;
			}
			cooldown *= 60;
			break;
		case 'h':
			HELPMSG(out, CMDNAME, CMDUSAGE, CMDDESCR);
			return EXIT_SUCCESS;
		case '?':
			snprintf(out, MAX_MSG, "%s", l_opterr());
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}

	if (cooldown % 300 != 0) {
		snprintf(out, MAX_MSG, "%s: interval must be a multiple "
				"of 5 mins", c->argv[0]);
		return EXIT_FAILURE;
	} else if (cooldown > 3600) {
		snprintf(out, MAX_MSG, "%s: interval cannot be longer "
				"than 60 mins", c->argv[0]);
		return EXIT_FAILURE;
	}

	if (l_optind == c->argc) {
		USAGEMSG(out, CMDNAME, CMDUSAGE);
		return EXIT_FAILURE;
	}

	argvcat(msg, c->argc, c->argv, l_optind, 1);
	if (!evtman->addmsg(msg, cooldown))
		snprintf(out, MAX_MSG, "%s: limit of 5 recurring "
				"messages reached", c->argv[0]);
	else
		snprintf(out, MAX_MSG, "@%s, recurring message \"%s\" has been "
				"added at a %ld min interval.", c->nick, msg,
				cooldown / 60);
	return EXIT_SUCCESS;
}
