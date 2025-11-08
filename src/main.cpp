/*
 * This is an object oriented approach to communication with a
 * mini-box DC-DC converter, based on the code written by
 * Nicu Pavel (https://github.com/mini-box/dcdc-usb)
 */

#include <iostream>
#include <sys/stat.h>
#include <getopt.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

#include "dcdc_usb.h"

using namespace std;


int main(int argc, char *argv[])
{

  bool enable = false, disable = false, onoff = false, ramp = false;
  double vout=0, vstart=0, vfinish=0, interval=0;

  int c;
  while(true){

    // command line options
    static struct option long_options[] =
      {
	{"help",    no_argument,       0, 'h'},
	{"enable",  no_argument,       0, 'e'},
	{"disable", no_argument,       0, 'd'},
	{"vout",    required_argument, 0, 'v'},
	{"vstart",  required_argument, 0, 's'},
	{"vfinish", required_argument, 0, 'f'},
	{"interval",required_argument, 0, 'i'},
	{"onoff",   no_argument,       0, 'o'},
	{"ramp",    no_argument,       0, 'r'},
	//{"",  , 0, ''},
	{0,0,0,0}
      };

    int option_index = 0;

    // leave case out of the list if no short option is to be allowed
    c = getopt_long (argc, argv, "hedv:s:f:i:or", long_options, &option_index);

    if (c == -1)
      break;

    // parse the command line options
    switch(c){
    case 0:
      if (long_options[option_index].flag != 0){
	break;
      }
    case 'h':
      cout<<"help"<<endl;
      break;
    case 'e':
      enable = true;
      break;
    case 'd':
      disable = true;
      break;
    case 'v':
      vout = strtod(optarg, NULL);
      break;
    case 's':
      vstart = strtod(optarg, NULL);
      break;
    case 'f':
      vfinish = strtod(optarg, NULL);
      break;
    case 'i':
      interval = strtod(optarg, NULL);
      break;
    case 'o':
      onoff = true;
      break;
    case 'r':
      ramp=true;
      break;
    }
  }


  dcdc_usb dcdc = dcdc_usb();


  if(ramp){ // ramp the voltage from vstart to vfinish
    if(dcdc.ramp(vstart, vfinish, interval, onoff))
      return 0;
    else
      return -1;
  }else if(vout!=0){ // Set the output voltage
    dcdc.set_vout(vout);
    cout<<"Current voltage: "<<dcdc.get_vout()<<endl;
  }else if(enable) // Turn on the output voltage
    dcdc.on();
  else if(disable) // Turn off the output voltage
    dcdc.off();
  else // Print current voltage
    cout<<"Current voltage: "<<dcdc.get_vout()<<endl;

}
