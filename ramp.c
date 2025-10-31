/*
 * Copyright (c) 2011 by Mini-Box.com, iTuner Networks Inc.
 * Written by Nicu Pavel <npavel@mini-box.com>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#include <usb.h>

#include "dcdc-usb.h"

void showhelp(char *prgname)
{

    printf ("Usage: %s [OPTION]\n", prgname);
    printf ("Options:.\n");
    printf (" -u \t ramp up\n");
    printf (" -d \t ramp down\n");
}

int main(int argc, char **argv)
{
    struct usb_dev_handle *h;
    char *s;
    int arg = 0, up = 0, down = 0;

    while ( ++arg < argc )
    {
	s = argv[arg];
	if (strncmp(s, "-u", 2) == 0)
	    up = 1;
	if (strncmp(s, "-h", 2) == 0)
	{
	    showhelp(argv[0]);
	    return 0;
	}
	if (strncmp(s, "-d", 2) == 0)
	{
	  down = 1;
	}
    }
    h = dcdc_connect();

    if (h == NULL)
    {
	fprintf(stderr, "Cannot connect to DCDC-USB\n");
	return 1;
    }

    if (dcdc_setup(h) < 0)
    {
	fprintf(stderr, "Cannot setup device\n");
	return 2;
    }

    if(up)
    {

      time_t currentTime;
      time(&currentTime);

      printf("ramp up started at %s\n", ctime(&currentTime));

      unsigned char val = 33;
      dcdc_set_raw(h, val);
      dcdc_on(h);

      for(int i=0; i<30; i++)
	{
	  val--;
	  dcdc_set_raw(h, val);
	  sleep(60);
	}


      time(&currentTime);
      printf("ramp up finished at %s\n", ctime(&currentTime));



    } else if(down)
    {

      time_t currentTime;
      time(&currentTime);

      printf("ramp down started at %s\n", ctime(&currentTime));

      unsigned char val = 3;
      for(int i=0; i<30; i++)
	{
	  dcdc_set_raw(h, val);
	  val++;
	  sleep(60);
	}


      dcdc_off(h);


      time(&currentTime);
      printf("ramp down finished at %s\n", ctime(&currentTime));

    }

    return 0;
}
