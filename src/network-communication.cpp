#include <EtherCard.h>

// Network unique mac address
static byte macAddress[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte ipAddress[] = { 192,168,1,110 };
// remote website ip address and port
static byte hisip[] = { 192,168,1,114 };

byte Ethernet::buffer[700];
BufferFiller buffer;
byte value = 0;

const char remote[] PROGMEM = "192.168.1.114";

/**
 * Returns a response buffer.
 * This is the HTTP Response from the
 * Webserver
 * @returns:
 *  HTTP Response Buffer
 */
static word sendResponse() {
  buffer = ether.tcpOffset();
  buffer.emit_p(PSTR("HTTP/1.0 200 OK"));
  return buffer.position();
}

/**
 * Response callback. When a GET Request
 * has completed and the answer is received
 * this callback will be triggered.
 * @param status:
 *  State of the Response
 * @param offset:
 *  Offset
 * @param length:
 *  The package length
 */
static void gotResponse(byte status, word offset, word length) {
  Ethernet::buffer[offset+300] = 0;
  Serial.print((const char*) Ethernet::buffer + offset);
}

/**
 * The setup function gets called
 * at the application start
 */
void setup () {
  // setup the serial connection
  Serial.begin(57600);
  Serial.println("[NETWORK COMMUNICATION]");

  // setup the Ethernet controller
  if (ether.begin(sizeof Ethernet::buffer, macAddress, 53) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
  }

  if (!ether.dhcpSetup()) {
    Serial.println(F("Failed to set the ip address"));
  }

  // set the ip to ping
  ether.copyIp(ether.hisip, hisip);

  ether.printIp("IP:       ", ether.myip);
  ether.printIp("Gateway:  ", ether.gwip);
  ether.printIp("Mask:     ", ether.netmask);
  ether.printIp("SRV:      ", ether.hisip);

  // Attach to interrupt
  DDRD = 0x00; // all bits to input
  // Enable INT0 External Interrupt
  bitWrite(EIMSK, INT0, 1);
  // Rising edge triggered
  bitWrite(EICRA, ISC00, 1);
  bitWrite(EICRA, ISC01, 1);
  sei(); // Enable the interupts
}

/**
 * The loop function get scalled after the
 * setup function has completed.
 * After the loop function finishes, it gets called again
 */
void loop () {
  word packetLength = ether.packetReceive(); // go receive new packets
  word position = ether.packetLoop(packetLength); // respond to incoming pings

  if (position) {
    char* data = (char *) Ethernet::buffer + position;
    Serial.println(data);
    // The value can be found at position 12
    int value = data[12];
    Serial.print("Got: ");
    Serial.println(value);
    ether.httpServerReply(sendResponse()); // send web page data
  }

  if (value) {
    char * querryString = " ";
    querryString[0] = value;
    Serial.print("GET /");
    Serial.println(querryString);
    ether.browseUrl(PSTR("/?value="), querryString, remote, gotResponse);
    value = 0; // Reset the value, can only be changed by the interupt
  }
}

/**
 * Interrupt service routine (int.0)
 * reads the value from PIND
 */
ISR(INT0_vect) {
  value = PIND;
}
