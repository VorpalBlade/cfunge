/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: gperf safe_env.gperf  */
/* Computed positions: -k'4,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 7 "safe_env.gperf"

#include <string.h>
#include <stdbool.h>
#include "safe_env.h"

#define TOTAL_KEYWORDS 32
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 17
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 54
/* maximum key range = 53, duplicates = 0 */

FUNGE_ATTR_FAST static inline unsigned int
safe_env_hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55,  5,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 15, 55,  0,  0,  5,
      55, 25, 20,  0, 55, 55, 15, 15, 15, 15,
       0, 55, 10, 15,  0,  0, 55, 55, 55,  0,
       0, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
      case 2:
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

FUNGE_ATTR_FAST static inline const char *
safe_in_word_set (register const char *str, register unsigned int len)
{
  static const unsigned char lengthtable[] =
    {
       0,  0,  2,  3,  4,  5,  0,  7,  3,  0,  0,  6,  7,  8,
       4, 10,  6, 12,  8,  0,  5,  6,  7,  0,  4, 10, 11,  7,
       0, 14,  0,  0, 17,  8,  4,  5,  6,  0,  0,  9, 10, 11,
       0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4
    };
  static const char * const wordlist[] =
    {
      "", "",
      "TZ",
      "PWD",
      "HOST",
      "GROUP",
      "",
      "DISPLAY",
      "PS1",
      "", "",
      "OSTYPE",
      "LC_TIME",
      "LC_CTYPE",
      "HOME",
      "LC_COLLATE",
      "EDITOR",
      "LC_TELEPHONE",
      "LC_PAPER",
      "",
      "PAGER",
      "VISUAL",
      "COLUMNS",
      "",
      "USER",
      "LC_NUMERIC",
      "LC_MONETARY",
      "LC_NAME",
      "",
      "LC_MEASUREMENT",
      "", "",
      "LC_IDENTIFICATION",
      "MACHTYPE",
      "TERM",
      "SHELL",
      "LC_ALL",
      "", "",
      "COLORTERM",
      "LC_ADDRESS",
      "LC_MESSAGES",
      "", "",
      "PATH",
      "", "", "", "", "", "", "", "", "",
      "LANG"
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = safe_env_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        if (len == lengthtable[key])
          {
            register const char *s = wordlist[key];

            if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
              return s;
          }
    }
  return 0;
}
#line 45 "safe_env.gperf"


FUNGE_ATTR_FAST bool check_env_is_safe(const char *envvar) {
	const char * p;

	p = strchr(envvar, '=');
	if (!p)
		return false;
	else
		return (safe_in_word_set(envvar, p - envvar) != NULL);
}
