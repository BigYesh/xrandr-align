/*
 * Original xinput:
 * Copyright 1996 by Frederic Lepied, France. <Frederic.Lepied@sugix.frmug.org>
 *
 * Original xrandr:
 * Copyright © 2001 Keith Packard, member of The XFree86 Project, Inc.
 * Copyright © 2002 Hewlett Packard Company, Inc.
 * Copyright © 2006 Intel Corporation
 *
 * xrandr-monitor:
 *
 * Copyright © 2012 Paul Wolneykien <manowar@altlinux.org>, ALT Linux Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  the authors  not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     The authors  make  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL THE AUTHORS  BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "string.h"
#include "common.h"

const char
*get_argval (int argc,
	     const char *argv[],
	     const char *argname,
	     const char *funcname,
	     const char *usage,
	     const char *defval)
{
  int i;

  for (i = 0; i < argc; i++) {
    if (strlen (argv[i]) > 2 &&
	strncmp (argv[i], "--", 2) == 0 &&
	strncmp (argv[i] + 2, argname, strlen(argname)) == 0) {
      char *eq;
      if (!(eq = strchr (argv[i], '=')) ||
	  strlen (eq) < 2)
	{
	  fprintf (stderr, "Usage: %s %s\n", funcname, usage);
	  return NULL;
	}
      else
	{
	  return eq + 1;
	}
    }
  }

  return defval;
}

int
get_screen (Display *display,
	    int	argc,
	    const char *argv[],
	    const char *funcname,
	    const char *usage,
	    int *retscreen)
{
  long screen;
  const char *screenarg;

  screenarg = get_argval (argc, argv, "screen", funcname, usage, "-1");
  if (screenarg == NULL) {
    return EXIT_FAILURE;
  } else {
    char *endptr;
    screen = strtol (screenarg, &endptr, 0);
    if (endptr == screenarg) {
      fprintf (stderr, "Invalid number: %s\n", screenarg);
      return EXIT_FAILURE;
    }
  }

  if (screen < 0) {
    screen = DefaultScreen (display);
  } else if (screen >= ScreenCount (display)) {
    fprintf (stderr, "Invalid screen number %ld (display has %d)\n",
	     screen, ScreenCount (display));
    return EXIT_FAILURE;
  }

  *retscreen = (int)screen;
  return EXIT_SUCCESS;
}
