#include <EtherCard.h>

// Network unique mac address
static byte macAddress[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte ipAddress[] = { 192,168,1,110 };
// remote website ip address and port
static byte hisip[] = { 192,168,0,111 };

byte Ethernet::buffer[700];
BufferFiller buffer;

static uint32_t timer;

const char remote[] PROGMEM = "192.168.0.111";
const char okHeader[] PROGMEM =
  "HTTP/1.0 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Pragma: no-cache\r\n";

/**
 * Returns a response buffer.
 * This is the HTTP Response from the
 * Webserver
 * @returns:
 *  HTTP Response Buffer
 */
static word sendResponse() {
  buffer = ether.tcpOffset();
  buffer.emit_p(PSTR("$F\r\n20"), okHeader);
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
  Serial.print("STATUS: ");
  Serial.println(status);
  Serial.println();

  Ethernet::buffer[offset+300] = 0;
  Serial.print((const char*) Ethernet::buffer + offset);
  Serial.println("...");
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

  timer = -9999999; // start timing out right away
  Serial.println();
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
    ether.httpServerReply(sendResponse()); // send web page data
  }

  if (micros() - timer >= 5000000) {
    ehter.printip("GET / ", ether.hisip);
    timer = micros();
    ether.browseUrl(PSTR("192.168.0.111"), "", remote, gotResponse);
  }
}
