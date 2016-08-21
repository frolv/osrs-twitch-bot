/*
 * lynxbot.h: bot information and general purpose definitions
 */

#ifndef LYNXBOT_H
#define LYNXBOT_H

#include <stdio.h>

/* general program information */
#define BOT_NAME	"LynxBot"
#define BOT_WEBSITE	"https://frolv.github.io/lynxbot"

/* version information */
#define _BOT_VER_MAJOR	"1"
#define _BOT_VER_MINOR	"4"
#define _BOT_VER_PATCH	"6"
#define _BOT_VER_SUF	""

/* full version string */
#define BOT_VERSION "v" _BOT_VER_MAJOR "." _BOT_VER_MINOR "."\
	_BOT_VER_PATCH _BOT_VER_SUF

#ifndef MAX_MSG
/* maximum length of chat message */
# define MAX_MSG 1024
#endif

/* maximum path length */
#define MAX_PATH 4096

/* buffer size for RSN fetching */
#define RSN_BUF 64

#ifdef __linux__
# define _sprintf snprintf
#endif
#ifdef _WIN32
# define _sprintf sprintf_s
#endif

/* wait for user to press return */
#define WAIT_INPUT() while (getchar() != '\n');

#endif /* LYNXBOT_H */
