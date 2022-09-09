/*  @(#)rad2str.c  version 1.4  created 94/03/09 22:07:56
                fetched from SCCS 94/04/25 10:06:37
%% function to convert angle in radians to arc or time string
LANGUAGE: C
ENVIRONMENT: Any
:: angle convert string arc-meas format
*/
#include <string.h>

#include "ctype.h"
#include "STDDEFS.H"
#include "MATHCNST.H"

char *rad2str ();    /* convert radians to string, output always rounded */
char *rad2strg ();   /* convert radians to string, rounded or truncated */
static int ltostr ();

/*++****************************************************************************
*/
char *rad2str (angle, pformat, pstring)    /* convert radians to string */
double angle;      /* input angle in radians */
char *pformat;     /* input string specifing format of output string */
char *pstring;     /* returned string ('\0' terminated) */
/*
* RETURNS original pointer to given string
* 
* NOTE- The output is ROUNDED to the specified seconds-field precision.  Be 
* careful, this sometimes produces unexpected results.
-*/
{
    return (rad2strg (angle, pformat, pstring, TRUE));
}

/*++****************************************************************************
*/
char *rad2strg (angle, pformat, pstring, round)
double angle;      /* input angle in radians */
char *pformat;     /* input string specifing format of output string */
char *pstring;     /* returned string ('\0' terminated) */
BOOL round;        /* TRUE- output string is rouned, FALSE-truncated */
/*
* RETURNS original pointer to given string
* 
* Converts a floating point number presumed to contain an angle in radians to a 
* string according to instructions in the format string.  The output string 
* format can be either degrees (default form is iiidjj'kk.f") or time (default 
* form is iihjjmkk.ffs) depending on the format string contents.  The jj 
* (minutes) and kk (integer seconds) fields are always two digits.  The widths 
* of the iii (degrees or hours) and ff (fractional seconds) fields are 
* controlled by the format string.
*
* The format string is a null-terminated string of the general form "dm.n" or 
* "hm.n" where m and n are digit strings.  The forms "d", "h" (use default 
* values for m and n), "dm", "hm" (use given m and default n), "d.n",  and 
* "h.n" (use default m and given n) are also permitted.
*
* The digit string m controls the width of the degrees/hours field in the 
* output string.  If m does not specify sufficient digits to represent the 
* degrees/hours, the field will be widened to the exact number of digits 
* needed.  If the first digit of m is zero, zero fill will be used; otherwise 
* blank fill will be used.
*
* The digit string n controls the number of decimal places in the fractional 
* portion of the seconds field.  If n is zero, no seconds fraction and no 
* decimal point will appear in the seconds field.
*
* EXAMPLE FORMAT STRINGS AND THEIR RESULTS --
*   "d", or indecipherable string --  111d22'33.3"
*   "h"            --  11h22m33.33s
*   "d4"           --     1d22'33.3"
*   "h.1"          --  11h22m33.3s
*   "d03.2"        --  001d22'33.33"
-*/
{
    char *poriginal = pstring;  /* hold output string address for return */
    char units_char[4];         /* field separation chars (d'" or hms) */
    int m;                      /* degrees/hours field width */
    int n;                      /* fractional seconds field width */
    BOOL lzero_flag = FALSE;    /* TRUE => use leading zeros fill */
    int sign_flag = 1;          /* 1=positive, -1=negative angle */
    register int i;
    register long long_tmp;

    /* decode measure type from format string */
    if (*pformat == 'h')
        {                   /* hour format */
	angle *= HOUR_RAD;         /* convert angle to hours */
//	strcpy (units_char, "hms");  /* set field separator chars */
	strcpy (units_char, ":::");  /* set field separator chars */
	m = 2;                     /* default hours field width */
	n = 2;                     /* default frac seconds field width */
        }
    else
        {                   /* degrees (or default) format */
	angle *= DEG_RAD;          /* convert angle to degrees */
//	strcpy (units_char, "d'\"");  /* set field separator chars */
	strcpy (units_char, ":::");  /* set field separator chars */
	m = 3;                     /* default degrees field width */
	n = 1;                     /* default frac seconds field width */
        }

    /* advance to next char if not at end */
    if (*pformat != '\0')
        pformat++;

    /* decode given degree/hour field width (if any) */
    if (isdigit (*pformat))
	{
	if (*pformat == '0')
	    {
	    lzero_flag = TRUE;        /* leading zeros fill */
	    pformat++;
	    }
	if (isdigit (*pformat))
	    {
	    m = 0;
	    while (isdigit (*pformat))
		m = m * 10 + *pformat++ - '0';
	    }
	}

    /* decode given fractional seconds field width (if any) */
    if (*pformat == '.')
        {
	pformat++;                     /* skip over . */
	if (isdigit (*pformat))
	    {
	    n = 0;
	    while (isdigit (*pformat))
		n = n * 10 + *pformat++ - '0';
	    }
        }

    /* if value is negative set sign flag and convert to positive */
    if (angle < 0.0)
	{
	sign_flag = -1;
	angle = -angle;
	}

    /* add rounding fraction to angle appropriate for desired output */
    if (round)
	{
        double round = 1.3888889E-4;    /* 1/2 second = 1/7200 */
	for (i = 0; i < n;  i++)
	    round /= 10.0;
	angle += round;
	}

    /* calculate and place degree/hours in output string
     here units of angle are either hours or degrees set according to format */
    long_tmp = angle;          /* note use of truncation */
    if (lzero_flag)            /* neg field width => use leading zeros */
        m = -m;
    pstring += ltostr (long_tmp, pstring, m, sign_flag);
    *pstring++ = units_char[0];

    /* calculate and place minutes in output string */
    angle = (angle - long_tmp) * 60.0;
    long_tmp = angle;
    pstring += ltostr (long_tmp, pstring, -2, 0);
    *pstring++ = units_char[1];

    /* calculate and place integer seconds in output string */
    angle = (angle - long_tmp) * 60.0;
    long_tmp = angle;
    pstring += ltostr (long_tmp, pstring, -2, 0);

    /* calculate and place fractional seconds in output string */
    if (n != 0)
        {
	*pstring++ = '.';
	angle -= long_tmp;     /* angle = fractional part of secs  */
        for (i = 0; i < n; i++)
            angle *= 10.0;
        pstring += ltostr ((long)angle, pstring, -n, 0);
	}
//    *pstring++ = units_char[2];
    *pstring++ = '\0';
 
    return (poriginal);
}

/*******************************************************************************
*/
static int ltostr (number, pstring, width, sign)    /* convert long to string */
long number;    /* number to be converted */
char *pstring;  /* receiving string */
int width;      /* minimum width of numeric field, positive width => use 
		   leading blank fill, negative => leading zero fill */
int sign;       /* only the sign of this int is important, negative => 
		   places '-' in output */
/*
* RETURNS number of characters that have been appended to string
*
* Output digits are always right-justified in the field.  BE CAREFUL - If 
* 'width' is less than the number of digits needed to express the number, the 
* field will be expanded to the exact number of characters required.
*
* A minus sign is prefixed if either 'number' or 'sign' is negative.  This 
* permits a 'signed zero' to be generated.  The minus sign is the first 
* character in the string if leading zeros are requested, else it is the 
* character immediately preceeding the most significant digit.
*
* The output string DOES NOT have a terminating null.
-*/
{
    register char *preverse;
    char fillchar;
    char tempch;
    int nochar = 0;
 
    /* if input number is negative ... */
    if (number < 0)
	{
        sign = -1;
        number = -number;
	}
 
    /* set fillchar = proper fill character */
    if (width < 0)
	{
        width = -width;
        fillchar = '0';
	}
    else
        fillchar = ' ';

    /* if input number is negative with leading zeroes fill,
       insert minus sign in first character of string */
    if (sign < 0 && fillchar == '0')
	{
        *pstring++ = '-';
        nochar++;
        sign = 0;
	}
 
    /* record pointer where string reversal will begin */
    preverse = pstring;

    /* convert input number to ascii in reverse order */
    do 
	{
        *pstring++ = '0' + number % 10;
        nochar++;
	} while ((number /= 10) > 0);

    /* if input number is negative with leading blank fill,
       insert minus sign in front of most significant digit */
    if (sign < 0 && fillchar == ' ')
	{
        *pstring++ = '-';
        nochar++;
	}

    /* insert enough fill characters to fill minimum field width */
    while (nochar < width)
	{
        *pstring++ = fillchar;
        nochar++;
	}

    /* point to last inserted character */
    pstring--;

    /* digits were generated in reverse order, so re-reverse them */
    while (preverse < pstring)
	{
        tempch = *preverse;
        *preverse++ = *pstring;
        *pstring-- = tempch;
	}

    /* return number of characters inserted */
    return (nochar);
}

