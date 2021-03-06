#include <string.h>
#include <utils.h>
#include "command.h"
#include "../CmdHandler.h"
#include "../option.h"
#include "../strfmt.h"
#include "../stringparse.h"

/* full name of the command */
CMDNAME("xp");
/* description of the command */
CMDDESCR("query experience information");
/* command usage synopsis */
CMDUSAGE("$xp [-i|-r] NUM");
/* -r flag usage */
static const char *RUSAGE = "$xp -r START STOP";

#define MAX_XP 0xBEBC200

static int xptolvl(int x);
static int lvltoxp(int x);
static int xprange(char *out, struct command *c);

/* xp: query experience information */
int CmdHandler::xp(char *out, struct command *c)
{
	int inv, range, status;
	int64_t x;
	char num[RSN_BUF];

	int opt;
	static struct l_option long_opts[] = {
		{ "help", NO_ARG, 'h' },
		{ "inverse", NO_ARG, 'i' },
		{ "range", NO_ARG, 'r' },
		{ 0, 0, 0 }
	};

	inv = range = 0;
	status = EXIT_SUCCESS;
	opt_init();
	while ((opt = l_getopt_long(c->argc, c->argv, "ir", long_opts)) != EOF) {
		switch (opt) {
		case 'h':
			HELPMSG(out, CMDNAME, CMDUSAGE, CMDDESCR);
			return EXIT_SUCCESS;
		case 'i':
			inv = 1;
			break;
		case 'r':
			range = 1;
			break;
		case '?':
			snprintf(out, MAX_MSG, "%s", l_opterr());
			return EXIT_FAILURE;
		default:
			return EXIT_FAILURE;
		}
	}

	if (range) {
		if (inv) {
			snprintf(out, MAX_MSG, "%s: cannot use -i with -r",
					c->argv[0]);
			status = EXIT_FAILURE;
		} else {
			status = xprange(out, c);
		}
		return status;
	}

	if (l_optind != c->argc - 1) {
		USAGEMSG(out, CMDNAME, CMDUSAGE);
		return EXIT_FAILURE;
	}

	if (!parsenum_mult(c->argv[l_optind], &x)) {
		snprintf(out, MAX_MSG, "%s: invalid number: %s",
				c->argv[0], c->argv[l_optind]);
		return EXIT_FAILURE;
	}
	if (x < 0) {
		snprintf(out, MAX_MSG, "%s: number cannot be "
				"negative", c->argv[0]);
		return EXIT_FAILURE;
	}

	if (inv) {
		if (x > MAX_XP) {
			snprintf(out, MAX_MSG, "%s: xp cannot exceed 200m",
					c->argv[0]);
			status = EXIT_FAILURE;
		} else {
			snprintf(out, MAX_MSG, "%ld", x);
			fmtnum(num, RSN_BUF, out);
			snprintf(out, MAX_MSG, "[XP] %s xp: level %d",
					num, xptolvl(x));
		}
		return status;
	}

	if (x < 1 || x > 126) {
		snprintf(out, MAX_MSG, "%s: level must be between 1-126",
				c->argv[0]);
		status = EXIT_FAILURE;
	} else {
		snprintf(out, MAX_MSG, "%d", lvltoxp(x));
		fmtnum(num, RSN_BUF, out);
		snprintf(out, MAX_MSG, "[XP] level %ld: ", x);
		strcat(out, num);
		strcat(out, " xp");
	}
	return status;
}

/* xptolvl: calculate the level at x xp */
static int xptolvl(int x)
{
	int n;

	n = 1;
	x *= 4;
	x += 1;
	while (x >= 0) {
		x -= floor(n + 300 * pow(2, n / 7.0));
		++n;
	}

	return n - 1;
}

/* lvltoxp: return xp required for level x */
static int lvltoxp(int x)
{
	int n, res;

	res = 0;
	for (n = 1; n < x; ++n)
		res += floor(n + 300 * pow(2, n / 7.0));
	res = floor(0.25 * res);

	return res;
}

/* xprange: calcuate the amount of xp between the levels in args */
static int xprange(char *out, struct command *c)
{
	int64_t x, y;
	char *a, *b;
	char num[RSN_BUF];

	if (l_optind == c->argc || l_optind < c->argc - 2) {
		USAGEMSG(out, CMDNAME, RUSAGE);
		return EXIT_FAILURE;
	}

	a = c->argv[l_optind];
	if (l_optind == c->argc - 1) {
		if (!(b = strchr(a, '-')) || !b[1]) {
			snprintf(out, MAX_MSG, "%s: must provide two levels",
					c->argv[0]);
			return EXIT_FAILURE;
		}
		*b++ = '\0';
	} else {
		b = c->argv[l_optind + 1];
	}

	if (!parsenum_mult(a, &x)) {
		snprintf(out, MAX_MSG, "%s: invalid number: %s", c->argv[0], a);
		return EXIT_FAILURE;
	}
	if (x < 1 || x > 126) {
		snprintf(out, MAX_MSG, "%s: level must be between 1-126",
				c->argv[0]);
		return EXIT_FAILURE;
	}
	if (!parsenum_mult(b, &y)) {
		snprintf(out, MAX_MSG, "%s: invalid number: %s", c->argv[0], b);
		return EXIT_FAILURE;
	}
	if (y < 1 || y > 126) {
		snprintf(out, MAX_MSG, "%s: level must be between 1-126",
				c->argv[0]);
		return EXIT_FAILURE;
	}

	if (x > y) {
		snprintf(out, MAX_MSG, "%s: invalid range", c->argv[0]);
		return EXIT_FAILURE;
	}

	x = lvltoxp(x);
	y = lvltoxp(y);

	snprintf(out, MAX_MSG, "%ld", y - x);
	fmtnum(num, RSN_BUF, out);
	snprintf(out, MAX_MSG, "[XP] level %s-%s: %s xp", a, b, num);
	return EXIT_SUCCESS;
}
