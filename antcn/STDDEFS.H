/*  @(#)STDDEFS.H  version 1.12  created 94/06/21 13:27:55
                fetched from SCCS 94/07/25 10:24:17
%% general-purpose, environment-independent definitions and macros
LANGUAGE: C
ENVIRONMENT: Any standard C with contiguous alphabetic codes (not EBCDIC)
:: headerinclude macros true toupper tolower isalpha
*/
	
/***************************************************/
/*            various symbolic constants           */
/***************************************************/

#if	!defined (EOF) || (EOF != (-1))
#define EOF  (-1)
#endif

#if	!defined (TRUE) || (TRUE != 1)
#define TRUE  1
#endif

#if	!defined (FALSE) || (FALSE != 0)
#define FALSE  0
#endif

#if	!defined (STD_IN) || (STD_IN != 0)
#define STD_IN  0
#endif

#if	!defined (STD_OUT) || (STD_OUT != 1)
#define STD_OUT  1
#endif

#if	!defined (STD_ERR) || (STD_ERR != 2)
#define STD_ERR  2
#endif

#if	!defined (FOREVER)
#define FOREVER  for(;;)
#endif

#if     !defined (NONE) || (NONE != (-1))
#define NONE  (-1)	/* for times when NULL won't do */
#define EOS   '\0'	/* C string terminator */
#define NUL   '\0'      /*  nul character, do not confuse with NULL pointer  */
#endif

#if     !defined (ERROR) || (ERROR != (-1))
#define ERROR  (-1)
#endif

#if     !defined (OK)
#define OK     0
#endif

#define NELEMENTS(array)    /* number of elements in an array */ \
              (sizeof (array) / sizeof ((array) [0]))

/***************************************************/
/*              max and min macros                 */
/***************************************************/

#if     !defined(max)
#define max(x, y)	(((x) < (y)) ? (y) : (x))
#define min(x, y)	(((x) < (y)) ? (x) : (y))
#endif


/***************************************************/
/*                  type definitions               */
/***************************************************/

#if     !defined(LOCAL)
#define FAST	register
#define IMPORT	extern
#define LOCAL	static
typedef int (*FUNCPTR) ();		/* pointer to a function */
typedef char * (*CFUNCPTR) ();    /* ptr to a function returning ptr to char */
typedef char TINY;
typedef char TBOOL;
typedef unsigned char UTINY;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef int ARGINT, BOOL, VOID, STATUS;
typedef unsigned short INSTR;		/* word-alligned instructions */
#endif


/***************************************************/ 
/*  macros for character testing and manipulation  */
/***************************************************/ 

#ifndef isalpha
#define isalpha(c) (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
#define isdigit(c) ( '0' <= c  &&  c <= '9' )
#define islower(c) ( 'a' <= c  &&  c <= 'z' )
#define isupper(c) ( 'A' <= c  &&  c <= 'Z' )
#define iswhite(c) (c == ' ' || c == '\t' || c == '\n')
#define isalnum(c) (isalpha(c) || isdigit(c))
#endif
#ifndef tolower
#define tolower(c) (isupper(c) ? (c - 'A' + 'a') : c)
#define toupper(c) (islower(c) ? (c - 'a' + 'A') : c)
#endif


/***************************************************/
/*         macros for string manipulation          */
/***************************************************/

#define SKIPWHITE(sptr)  { while (iswhite(*sptr)) sptr++; }
 

/***************************************************/
/* allow gcc to use __attribute__; acc & cc ignore*/
/***************************************************/
#ifndef __GNUC__
#define __attribute__(x)
#endif
