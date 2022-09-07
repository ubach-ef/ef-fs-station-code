/* /usr2/st/inject_snap/inject_snap.c
 * $Id$
 * This inject_snap was written by Paul Harbison (harbison@cdscc.nasa.gov)
 * and is just the oprin.c code written by:
 * Copyright (C) 1992 NASA GSFC, Ed Himwich. (weh@vega.gsfc.nasa.gov)
 * Copyright (C) 1995 Ari Mujunen. (amn@nfra.nl, Ari.Mujunen@hut.fi)
 *
 * that has been modified to take a command line instead of stdin from the
 * readline routine.

 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License.
 * See the file 'COPYING' for details.

 * $Log$
 */

#include <stdio.h>
#include <sys/types.h>

/* For tolower. */
#include <ctype.h>

/* For assert. */
#include <assert.h>

/* For open, stat, read, close, isatty. */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* For malloc, free. */
#include <stdlib.h>

#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/fscom.h"

/* External FS variables. */
extern struct fscom *shm_addr;

/* External FS functions, perhaps these should eventually go into a '.h'? */
extern void setup_ids(void);
extern void sig_ignore(void);
extern void cls_snd(long *class,
		    char *buffer,
		    int length,
		    int parm3,
		    int parm4);
extern void skd_run(char name[5], char w, long ip[5]);

static long ipr[5] = { 0, 0, 0, 0, 0};

/* The dynamically allocated SNAP command table. */

/* 'inject_snap' main. */

int
main(int argc, char **argv)
{
  char *input;
  char *previous_input;
  int length;

  setup_ids();
  sig_ignore();

  previous_input = NULL;

    input = argv[1];

    if ((length = strlen(input))) {

      /* Execute this SNAP command via "boss". */
      cls_snd( &(shm_addr->iclopr), input, length, 0, 0);
      skd_run("boss ",'n',ipr);

    }  /* if have at least one character */

  return (0); 
}  /* main of inject_snap */
