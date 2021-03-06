#include <stdlib.h>
#include <string.h>
#include "cmdparse.h"
#include "lynxbot.h"
#include "stringparse.h"

#define ERR_LEN 256

static char err[ERR_LEN];

static int escseq(char *s);
static char *readquotes(char *s);

/* parse_cmd: split cmdstr into argv of c */
int parse_cmd(char *cmdstr, struct command *c)
{
	int i;
	char *s;

	c->argc = 0;
	for (s = cmdstr; *s; ++s) {
		switch (*s) {
		case '\\':
			if (!escseq(s))
				return 0;
			break;
		case '\'':
		case '"':
			if (!(s = readquotes(s)))
				return 0;
			break;
		case ' ':
		case '\t':
			c->argc++;
			*s = '\0';
			break;
		default:
			break;
		}
	}
	c->argc++;

	c->argv = malloc((c->argc + 1) * sizeof(*(c->argv)));

	s = cmdstr;
	for (i = 0; i < c->argc; ++i) {
		c->argv[i] = s;
		s = strchr(s, '\0') + 1;
	}
	c->argv[i] = NULL;
	err[0] = '\0';
	return 1;
}

/* free_cmd: free allocated memory in c */
void free_cmd(struct command *c)
{
	free(c->argv);
}

/* cmderr: return parsing error */
char *cmderr()
{
	return err;
}

/* escseq: process an escape sequence in s */
static int escseq(char *s)
{
	static const char *esc = " \\'\"";

	if (!s[1]) {
		snprintf(err, ERR_LEN, "error: unexpected end of line");
		return 0;
	}
	if (!strchr(esc, s[1])) {
		snprintf(err, ERR_LEN, "error: unrecognized "
				"escape sequence \\%c", s[1]);
		return 0;
	}
	shift_l(s);
	return 1;
}

/* readquotes: process a quoted sequence in s */
static char *readquotes(char *s)
{
	int quot;

	quot = *s;
	shift_l(s);
	for (++s; *s && *s != quot; ++s) {
		if (*s == '\\' && s[1] == quot)
			shift_l(s);
	}
	if (!*s) {
		snprintf(err, ERR_LEN, "error: unterminated quote");
		return NULL;
	}
	shift_l(s);
	return s - 1;
}
