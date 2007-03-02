/* $Id$	*/

#include <stdlib.h>
#include <string.h>
#include "progmem.h"

#ifndef __AVR__
# include <stdio.h>
# define PRINTFLN(line, fmt, ...)	\
    printf("\nLine %d: " fmt "\n", line, ##__VA_ARGS__)
# define EXIT(code)	exit ((code) < 255 ? (code) : 255)
# define memcmp_P	memcmp
# define strsep_P	strsep
#else
# define PRINTFLN(args...)
# define EXIT	exit
#endif

void Check (int line, const char *s1, const char *s2, int clr, int pnt)
{
    char t1[300];
    char *sp;
    char *rp;

    if (strlen_P(s1) > sizeof(t1) - 1)
	exit (1);
    strcpy_P (t1, s1);
    sp = t1;
    rp = strsep_P (&sp, s2);

    if (rp != t1) {
	PRINTFLN (line, "false return value");
	EXIT (5000 + line);
    }
    if (clr < 0) {
	if (strcmp_P (t1, s1)) {
	    PRINTFLN (line, "string is changed");
	    EXIT (line);
	}
    } else {
	if (strlen (t1) != (size_t)clr) {
	    PRINTFLN (line, "strlen: expect= %d  result= %d",
			 clr, strlen (t1));
	    EXIT (1000 + line);
	}
	if (memcmp_P (t1, s1, clr)
	    || t1[clr]
	    || strcmp_P (t1 + clr + 1, s1 + clr + 1))
	{
	    PRINTFLN (line, "string mismatch");
	    EXIT (2000 + line);
	}
    }
    if (pnt < 0) {
	if (sp) {
	    PRINTFLN (line, "sp is not a NULL");
	    EXIT (3000 + line);
	}
    } else {
	if (sp != t1 + pnt) {
	    PRINTFLN (line, "sp: expect= %d  result= %d",
			 pnt, sp - t1);
	    EXIT (4000 + line);
	}
    }
}

/* Args:
     s - string to parse
     delim - delimeter list
     clr   - if (clr >= 0) s[cln] must be cleared
     pnt   - if (pnt >= 0) s[pnt] must be pointed, else NULL
 */
#define CHECK(s, delim, clr, pnt)	do {		\
    Check (__LINE__, PSTR(s), PSTR(delim), clr, pnt);	\
} while (0)

int main ()
{
    char *p;
    
    /* NULL at first call	*/
    p = 0;
    if (strsep_P (&p, "") || p) exit (__LINE__);
    if (strsep_P (&p, "abc") || p) exit (__LINE__);

    /* Empty string	*/
    CHECK ("", "", -1, -1);
    CHECK ("", "abc", -1, -1);
    
    /* Empty delimeter list	*/
    CHECK ("a", "", -1, -1);
    CHECK ("12345678", "", -1, -1);
    
    /* No delimeter symbols are founded	*/
    CHECK ("\tabc", " ", -1, -1);
    CHECK ("THE QUICK BROWN FOX", "thequickbrownfox", -1, -1);
    
    /* delim is 1 byte long	*/
    CHECK (".", ".", 0, 1);
    CHECK ("abc", "a", 0, 1);
    CHECK ("abc", "b", 1, 2);
    CHECK ("abc", "c", 2, 3);

    /* delim is 2 bytes long	*/
    CHECK ("0", "01", 0, 1);
    CHECK ("1", "01", 0, 1);
    CHECK ("A.", "AB", 0, 1);
    CHECK ("B.", "AB", 0, 1);
    CHECK ("CAD", "AB", 1, 2);
    CHECK ("CDB", "AB", 2, 3);
    
    /* delim length > 2 bytes	*/
    CHECK ("the quick", "0123456789 ", 3, 4);

    /* Very long string	*/
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "...............................................................*",
	   "*", 255, 256);
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "................................................................"
	   "*", "*", 256, 257);

    /* Non ASCII bytes	*/
    CHECK ("\001\002\377", "\001", 0, 1);
    CHECK ("\001\002\377", "\377", 2, 3);
}
