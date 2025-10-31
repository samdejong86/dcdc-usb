/*
 * Written by Sam de Jong, based on dcdc-usb written by
 * Nicu Pavel <npavel@mini-box.com>
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
    printf (" -i \t interval in seconds between steps\n");
    printf (" -s \t start value (255-0: high value is low voltage)\n");
    printf (" -e \t end value (255-0: high value is low voltage)\n");
    printf (" -t \t turn power on/off\n");

}

int main(int argc, char **argv)
{
    struct usb_dev_handle *h;
    char *s;
    int arg = 0, up = 0, down = 0, interval = 0, start=0, end=0, onoff=0;

    while ( ++arg < argc )
    {
	s = argv[arg];
	if (strncmp(s, "-h", 2) == 0)
	{
	    showhelp(argv[0]);
	    return 0;
	}
	if (strncmp(s, "-i", 2) == 0)
	    if (arg + 1 < argc)
	    {
		arg++;
		interval = strtod(argv[arg], NULL);
	    }
	if (strncmp(s, "-s", 2) == 0)
	    if (arg + 1 < argc)
	    {
		arg++;
		start = strtod(argv[arg], NULL);
	    }
	if (strncmp(s, "-e", 2) == 0)
	    if (arg + 1 < argc)
	    {
		arg++;
		end = strtod(argv[arg], NULL);
	    }

	if (strncmp(s, "-t", 2) == 0)
	{
	  onoff = 1;
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

    if(start == end){
      fprintf(stderr, "Start value and end value must be different\n");
      return 2;
    }
    else if (interval == 0){
      fprintf(stderr, "No interval specified\n");
      return 2;
    }
    else if(start > end)
      up = 1;
    else if(start < end)
      down = 1;


    if(up)
    {

      if (start <= end){
	fprintf(stderr, "Start value must be greater than end value\n");
	return 3;
      }

      unsigned char val = (unsigned char)start;

      time_t currentTime;
      time(&currentTime);

      printf("ramp up started at %s", ctime(&currentTime));
      printf("  starting at value: %d\n", val);

      dcdc_set_raw(h, val);


      if(onoff)
	dcdc_on(h);

      while(val>=end)
	{
	  val--;
	  dcdc_set_raw(h, val);
	  sleep(interval);
	}


      time(&currentTime);
      printf("ramp up finished at %s", ctime(&currentTime));
      printf("  ending at value: %d\n", val);



    } else if(down)
    {

      if (start >= end){
	fprintf(stderr, "Start value must be greater than end value\n");
	return 3;
      }

      unsigned char val = (unsigned char)start;

      time_t currentTime;
      time(&currentTime);

      printf("ramp down started at %s", ctime(&currentTime));
      printf("  starting at value: %d\n", val);


      while(val<=end)
	{
	  val++;
	  dcdc_set_raw(h, val);
	  sleep(interval);
	}

      if(onoff)
	dcdc_off(h);

      time(&currentTime);
      printf("ramp down finished at %s\n", ctime(&currentTime));

    }

    return 0;
}
