/*
 * This is an object oriented approach to communication with a
 * mini-box DC-DC converter, based on the code written by
 * Nicu Pavel (https://github.com/mini-box/dcdc-usb)
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <time.h>

#include <usb.h>

#include "dcdc_usb.h"

// Default constructor: Connect the the dcdc converter
dcdc_usb::dcdc_usb()
{
    struct usb_bus *b;
    struct usb_device *d;
    struct usb_dev_handle *h_temp = NULL;

    usb_init();
    usb_set_debug(0);
    usb_find_busses();
    usb_find_devices();

    for (b = usb_get_busses(); b != NULL; b = b->next)
    {
	for (d = b->devices; d != NULL; d = d->next)
	{
	    if ((d->descriptor.idVendor == dcdc_vid) &&
	       (d->descriptor.idProduct == dcdc_pid))
	    {
		h_temp = usb_open(d);
		break;
	    }
	}
    }

    h = h_temp;

    setup();

}

// Setup the device
void dcdc_usb::setup()
{
    char buf[65535];

    if (h == NULL){
      status = -1;
      return;
    }

    if (usb_get_driver_np(h, 0, buf, sizeof(buf)) == 0)
    {
	if (usb_detach_kernel_driver_np(h, 0) < 0)
	{
	    fprintf(stderr, "Cannot detach from kernel driver\n");
	    status = -2;
	    return;
	}
    }

    if (usb_set_configuration(h, 1) < 0)
    {
	fprintf(stderr, "Cannot set configuration 1 for the device\n");
	status = -3;
	return;
    }

    usleep(1000);

    if (usb_claim_interface(h, 0) < 0)
    {
	fprintf(stderr, "Cannot claim interface 0\n");
	status = -4;
	return;
    }

    if (usb_set_altinterface(h, 0) < 0)
    {
	fprintf(stderr, "Cannot set alternate configuration\n");
	status = -5;
	return;
    }

    if (usb_control_msg(h, USB_TYPE_CLASS + USB_RECIP_INTERFACE,
		0x000000a, 0x0000000, 0x0000000, buf, 0x0000000, 1000)
	< 0)
    {
	fprintf(stderr, "Cannot send control message\n");
	status = -6;
	return;
    }

    return;


}

// Send data to dcdc
void dcdc_usb::send(uint8_t *data, int size)
{
  if (data == NULL){
    status = -1;
    return;
  }

  status = usb_interrupt_write(h, USB_ENDPOINT_OUT + 1, (char *) data, size, 1000);
}

// Receive data from dcdc
void dcdc_usb::recv(uint8_t *data, int size, int timeout)
{

  if (data == NULL){
    status = -1;
    return;
  }

  status = usb_interrupt_read(h, USB_ENDPOINT_IN + 1, (char *) data, size, timeout);
}


// transforms voltage value to a byte
uint8_t dcdc_usb::vout2dev(double vout)
{
    double rpot = (double)0.8 * CT_R1 / (vout - (double)0.8) - CT_R2;

    double result = (257 * (rpot-CT_RW) / CT_RP);

    if (result<0) result = 0;
    if (result>255) result = 255;

    //result = round(result);
    return (uint8_t)result;
}

// Send a command to the dcdc converter
void dcdc_usb::send_command(uint8_t cmd, uint8_t val)
{
    uint8_t c[3];
    c[0] = DCDCUSB_CMD_OUT;
    c[1] = cmd;
    c[2] = val;

    send(c, 3);
}

// Turn off vout
void dcdc_usb::off()
{
  send_command(CMD_SET_OUTPUT, 0);
}

// Turn on vout
void dcdc_usb::on()
{
  send_command(CMD_SET_OUTPUT, 1);
}

// Set the max vout for the ramping
bool dcdc_usb::set_max_vout(double max){
  // Check that requested max is not greater than the maximum safe value
  if(max <= MAXV){
    max_vout = max;
    return true;
  } else{
    cout<<"Requested vmax outside safe range ("<<MINV<<"V-"<<MAXV<<"V)"<<endl;
    return false;
  }
}

// Set the minimum vout for ramping
bool dcdc_usb::set_min_vout(double min){
  // Check that requested min is not less than minumum safe value
  if(min >= MINV){
    min_vout = min;
    return true;
  }else{
    cout<<"Requested vmin ("<<min<<") outside safe range ("<<MINV<<"V-"<<MAXV<<"V)"<<endl;
    return false;
  }
}

// Ramp the voltage from a start value to an end value
bool dcdc_usb::ramp(double start, double finish, double interval, bool onoff)
{
  if(interval==0){
      interval=1;
      cout<<"No interval specified, using 1s"<<endl;
    }

    if(start==finish){
      cout<<"Start and finish are the same"<<endl;
      return false;
    }

    // If no starting voltage is specified, use the current voltage
    if(start == 0){
      double temp = get_vout();
      if(temp>MAXV)
	start=MAXV;
      else if(temp<MINV)
	start=MINV;
      else
	start=temp;
    }

    time_t currentTime;
    time(&currentTime);

    cout<<"Voltage ramping started at "<<ctime(&currentTime);
    cout<<"Starting voltage is "<<start<<endl;

    if(start>finish){ //ramp down

      // Set min and max voltage, making sure they're in the safe range
      if(set_min_vout(finish) && set_max_vout(start)){
	bool can_continue = true;
	while(can_continue){
	  // decrement the voltage
	  can_continue = decrement();
	  // then wait out the interval
	  sleep(interval);
	}
	// turn off vout if requested
	if(onoff)
	  off();
      }else
	return false-2;
    }else{ //ramp up
      // Set min and max voltage, making sure they're in the safe range
      if(set_max_vout(finish) && set_min_vout(start)){
	// turn on vout if requested
	if(onoff)
	  on();
	bool can_continue = true;
	while(can_continue){
	  // increment the voltage
	  can_continue = increment();
	  // then wait out the interval
	  sleep(interval);
	}
      }else
	return false;

    }

    time(&currentTime);
    cout<<"Voltage ramping ended at "<<ctime(&currentTime);
    cout<<"Ending voltage is "<<get_vout()<<endl;

    return true;
}

// Get the device status
void dcdc_usb::get_status()
{

  uint8_t buf[MAX_TRANSFER_SIZE];

    uint8_t c[2];


    c[0] = DCDCUSB_GET_ALL_VALUES;
    c[1] = 0;

    send(c, 2);
    if (status < 0)
    {
	fprintf(stderr, "Cannot send command to device\n");
	return;
    }

    recv(buf, MAX_TRANSFER_SIZE, 1000);
    if (status < 0)
    {
      	fprintf(stderr, "Cannot get device status\n");
	return;
    }

    parse_values(buf);

}

// Set the output voltage
void dcdc_usb::set_vout(double vout)
{
  if(vout > MAXV || vout < MINV){
    cout<<"Requested vout outside safe range ("<<MINV<<"V-"<<MAXV<<"V)"<<endl;
    return;
  }

    if (vout < 5) vout = 5;
    if (vout > 24) vout = 24;

    send_command(CMD_WRITE_VOUT, vout2dev(vout));
}

// Get the current output voltage
double dcdc_usb::get_vout()
{

    uint8_t buf[MAX_TRANSFER_SIZE];

    send_command(CMD_READ_VOUT, 0);

    recv(buf, MAX_TRANSFER_SIZE, 1000);

    return byte2vout(buf[3]);
}

// increment the output voltage by one step
bool dcdc_usb::increment()
{
  double vout = get_vout();
  double nextvout = byte2vout(vout2dev(vout)-1);

  // Make sure the next output voltage value is below the maximum
  if(nextvout > (double)max_vout)
    return false;

  send_command(CMD_INC_VOUT,0);


  return true;
}


// decrement the output voltage by one step
bool dcdc_usb::decrement()
{

  double vout = get_vout();
  double nextvout = byte2vout(vout2dev(vout)+1);

  // Make sure the next output voltage value is above the minumum
  if(nextvout < min_vout)
    return false;
  send_command(CMD_DEC_VOUT,0);

  return true;
}


// Parse the data from the dcdc converter
void dcdc_usb::parse_data(uint8_t *data, int size)
{
  if (data == NULL){
    status = -1;
    return;
  }

#ifdef DEBUG
    int i;
    for (i = 0; i < size; i++)
    {
	if (i % 8 == 0) fprintf(stderr, "\n");
	fprintf(stderr, "[%02d] = 0x%02x ", i, data[i]);
    }
    fprintf(stderr, "\n");
#endif

    if(data[0] == DCDCUSB_RECV_ALL_VALUES)
      parse_values(data);
    else if(data[0] == DCDCUSB_CMD_IN)
      parse_cmd(data);
    else if(data[0] == INTERNAL_MESG)
      parse_internal_msg(data);
    else if(data[0] == DCDCUSB_MEM_READ_IN)
      parse_mem(data);
    else
      fprintf(stderr, "Unknown message\n");

}

#define P(t, v...) fprintf(stderr, t "\n", ##v)

// Concatenate two bytes into an integer
int dcdc_usb::bytes2int(uint8_t c1, uint8_t c2)
{
    int i = c1;
    i = i << 8;
    i = i | c2;

    return i;
}

// Get the bits of a byte
int dcdc_usb::byte2bits(uint8_t c)
{
    int i,n = 0;
    for (i = 0; i < 8; i++)
    {
	n = n * 10;
	if ((c >> i) & 1)
	    n = n + 1;
    }
    return n;
}

// Convert the byte value returned by the dcdc converter into a voltage
double dcdc_usb::byte2vout(uint8_t c)
{
  double rpot = (((double) c) * CT_RP / (double)257) + CT_RW;
  double voltage = (double)80 * ((double) 1 + CT_R1/(rpot + CT_R2));
  voltage = floor(voltage);
  return voltage/100;
}


// Print out the current settings
void dcdc_usb::parse_values(uint8_t *data)
{
    int mode, state, status;
    float ignition_voltage, input_voltage, output_voltage;

    mode = data[1];
    state = data[2];
    input_voltage = (float) data[3] * 0.1558f;
    ignition_voltage = (float) data[4] * 0.1558f;
    output_voltage = (float) data[5] * 0.1170f;
    status = data[6];
    switch(mode & 0x03)
    {
	case 0: P("mode: 0 (dumb)"); break;
	case 1: P("mode: 1 (automotive)"); break;
	case 2: P("mode: 2 (script)"); break;
	case 3: P("mode: 3 (ups)");break;
    }
    P("time config: %d", (mode >> 5) & 0x07);
    P("voltage config: %d", (mode >> 2) & 0x07);
    P("state: %d", state);
    P("input voltage: %.2f", input_voltage);
    P("ignition voltage: %.2f", ignition_voltage);
    P("output voltage: %2f", output_voltage);
    P("power switch: %s", ((status & 0x04) ? "On":"Off"));
    P("output enable: %s", ((status & 0x08) ? "On":"Off"));
    P("aux vin enable %s", ((status & 0x10) ? "On":"Off"));
    P("status flags 1: %d", byte2bits(data[6]));
    P("status flags 2: %d", byte2bits(data[7]));
    P("voltage flags: %d", byte2bits(data[8]));
    P("timer flags: %d", byte2bits(data[9]));
    P("flash pointer: %d", data[10]);
    P("timer wait: %d", bytes2int(data[11], data[12]));
    P("timer vout: %d", bytes2int(data[13], data[14]));
    P("timer vaux: %d", bytes2int(data[15], data[16]));
    P("timer pw switch: %d", bytes2int(data[17], data[18]));
    P("timer off delay: %d", bytes2int(data[19], data[20]));
    P("timer hard off: %d", bytes2int(data[21], data[22]));
    P("version: %d.%d", ((data[23] >> 5) & 0x07), (data[23] & 0x1F));
}


void dcdc_usb::parse_ignition(uint8_t *data)
{
	P("%.2f", (float) data[4] * 0.1558f);
}

void dcdc_usb::parse_cmd(uint8_t *data)
{
    if (data[1] == 0)
    {
	if (data[2] == CMD_READ_REGULATOR_STEP)
	{
	    P("regulator step: %d", data[3]);
	}
	else
	{
	    P("output voltage: %.2f", byte2vout(data[3]));
	}
    }
    else
    {
	if (data[2] == CMD_READ_REGULATOR_STEP)
	{
	    P("regulator step not defined");
	}
    }
}

void dcdc_usb::parse_internal_msg(uint8_t *data)
{
    P("Parsing INTERNAL MESSAGE: Not implemented");
}

void dcdc_usb::parse_mem(uint8_t *data)
{
    P("Parsing MEM READ IN: Not implemented");
}
