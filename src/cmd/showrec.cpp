#include "command.h"
#include "../CmdHandler.h"
#include "../option.h"
#include "../stringparse.h"

/* full name of the command */
CMDNAME("showrec");
/* description of the command */
CMDDESCR("show recurring messages");
/* command usage synopsis */
CMDUSAGE("$showrec [ID]");

/* listrec: show recurring messages */
int CmdHandler::showrec(char *out, struct command *c)
{
	int64_t id;
	int opt, status;
	static struct l_option long_opts[] = {
		{ "help", NO_ARG, 'h' },
		{ 0, 0, 0 }
	};

	if (!P_ALMOD(c->privileges)) {
		PERM_DENIED(out, c->nick, c->argv[0]);
		return EXIT_FAILURE;
	}

	opt_init();
	status = EXIT_SUCCESS;
	while ((opt = l_getopt_long(c->argc, c->argv, "", long_opts)) != EOF) {
		switch (opt) {
		case 'h':
			HELPMSG(out, CMDNAME, CMDUSAGE, CMDDESCR);
			return EXIT_SUCCESS;
		case '?':
			_sprintf(out, MAX_MSG, "%s", l_opterr());
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}

	if (l_optind == c->argc) {
		_sprintf(out, MAX_MSG, "%s", m_evtp->msglist().c_str());
		return status;
	}

	if (l_optind != c->argc - 1) {
		USAGEMSG(out, CMDNAME, CMDUSAGE);
		return EXIT_FAILURE;
	}

	/* show a single message */
	if (!parsenum(c->argv[l_optind], &id)) {
		_sprintf(out, MAX_MSG, "%s: invalid number: %s",
				c->argv[0], c->argv[l_optind]);
		status = EXIT_FAILURE;
	} else if (id < 1 || (size_t)id > m_evtp->messages()->size()) {
		_sprintf(out, MAX_MSG, "%s: recurring message %ld "
				"doesn't exist", c->argv[0], id);
		status = EXIT_FAILURE;
	} else {
		_sprintf(out, MAX_MSG, "%s", m_evtp->message(id - 1).c_str());
	}
	return status;
}
