#ifndef DCDCUSBOOP_H
#define DCDCUSBOOP_H

using namespace std;

class dcdc_usb
{

 public:

  dcdc_usb();

  void get_status();
  void set_vout(double vout);
  double get_vout();
  void parse_data(unsigned char *data, int size);
  void off();
  void on();

  bool ramp(double start, double finish, double interval, bool onoff);

  bool increment();
  bool decrement();

  int get_comm_status(){return status;}


  double get_max_vout(){return max_vout;}
  bool set_max_vout(double max);
  double get_min_vout(){return min_vout;}
  bool set_min_vout(double min);

 private:

  int status = 0;

  struct usb_dev_handle *h;

  double max_vout = MAXV;
  double min_vout = MINV;


  void send(uint8_t *data, int size);
  void recv(uint8_t *data, int size, int timeout);
  void setup();

  uint8_t vout2dev(double vout);

  void send_command(uint8_t cmd, uint8_t val);

  /* DCDC USB data parsing */
  void parse_values(uint8_t *data);
  void parse_cmd(uint8_t *data);
  void parse_internal_msg(uint8_t *data);
  void parse_mem(uint8_t *data);
  void parse_ignition(uint8_t *data);

  int bytes2int(uint8_t c1, uint8_t c2);
  int byte2bits(uint8_t c);
  double byte2vout(uint8_t c);




  // various constants
  const uint16_t dcdc_vid = 0x04d8;
  const uint16_t dcdc_pid = 0xd003;

  const double CT_RW = 75;
  const double CT_R1 = 49900;
  const double CT_R2 = 1500;
  const double CT_RP = 10000;

  const uint8_t MAX_TRANSFER_SIZE = 24;

  const uint8_t STATUS_OK	= 0x00;
  const uint8_t STATUS_ERASE	= 0x01;
  const uint8_t STATUS_WRITE	= 0x02;
  const uint8_t STATUS_READ	= 0x03;
  const uint8_t STATUS_ERROR	= 0xff;

  const uint8_t DCDCUSB_GET_ALL_VALUES	= 0x81;
  const uint8_t DCDCUSB_RECV_ALL_VALUES	= 0x82;
  const uint8_t DCDCUSB_CMD_OUT		= 0xB1;
  const uint8_t DCDCUSB_CMD_IN		= 0xB2;
  const uint8_t DCDCUSB_MEM_READ_OUT	= 0xA1;
  const uint8_t DCDCUSB_MEM_READ_IN	= 0xA2;
  const uint8_t DCDCUSB_MEM_WRITE_OUT	= 0xA3;
  const uint8_t DCDCUSB_MEM_WRITE_IN	= 0xA4;
  const uint8_t DCDCUSB_MEM_ERASE	= 0xA5;

  const uint8_t INTERNAL_MESG		   = 0xFF;
  const uint8_t INTERNAL_MESG_DISCONNECTED = 0x01;

  const uint8_t CMD_SET_AUX_WIN		= 0x01;
  const uint8_t CMD_SET_PW_SWITCH	= 0x02;
  const uint8_t CMD_SET_OUTPUT		= 0x03;
  const uint8_t CMD_WRITE_VOUT		= 0x06;
  const uint8_t CMD_READ_VOUT		= 0x07;
  const uint8_t CMD_INC_VOUT		= 0x0C;
  const uint8_t CMD_DEC_VOUT		= 0x0D;
  const uint8_t CMD_LOAD_DEFAULTS	= 0x0E;
  const uint8_t CMD_SCRIPT_START	= 0x10;
  const uint8_t CMD_SCRIPT_STOP		= 0x11;
  const uint8_t CMD_SLEEP		= 0x12;
  const uint8_t CMD_READ_REGULATOR_STEP	= 0x13;

  /* For reading out memory */
  const uint8_t TYPE_CODE_MEMORY	= 0x00;
  const uint8_t TYPE_EPROM_EXTERNAL	= 0x01;
  const uint8_t TYPE_EPROM_INTERNAL	= 0x02;
  const uint8_t TYPE_CODE_SPLASH	= 0x03;

  /* AddressLo : AddressHi : AddressUp (anywhere inside the 64 byte-block to be erased) */
  const uint8_t FLASH_REPORT_ERASE_MEMORY = 0xF2;
  /* AddressLo : AddressHi : AddressUp : Data Length (1...32) */
  const uint8_t FLASH_REPORT_READ_MEMORY  = 0xF3;
  /* AddressLo : AddressHi : AddressUp : Data Length (1...32) : Data.... */
  const uint8_t FLASH_REPORT_WRITE_MEMORY = 0xF4;
  /* same as F2 but in keyboard mode */
  const uint8_t KEYBD_REPORT_ERASE_MEMORY = 0xB2;
  /* same as F3 but in keyboard mode */
  const uint8_t KEYBD_REPORT_READ_MEMORY  = 0xB3;
  /* same as F4 but in keyboard mode */
  const uint8_t KEYBD_REPORT_WRITE_MEMORY = 0xB4;
  /* response to b3,b4 */
  const uint8_t KEYBD_REPORT_MEMORY	= 0x41;

  const uint8_t IN_REPORT_EXT_EE_DATA	= 0x31;
  const uint8_t OUT_REPORT_EXT_EE_READ	= 0xA1;
  const uint8_t OUT_REPORT_EXT_EE_WRITE	= 0xA2;

  const uint8_t IN_REPORT_INT_EE_DATA	= 0x32;
  const uint8_t OUT_REPORT_INT_EE_READ	= 0xA3;
  const uint8_t OUT_REPORT_INT_EE_WRITE	= 0xA4;


};




#endif
