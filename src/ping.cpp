#include <EtherCard.h>

// Network unique mac address
static byte macAddress[] = { 0x69,0x69,0x69,0x69,0x69,0x69 };

byte Ethernet::buffer[700];
static uint32_t timer;

/**
 * Ping callback,
 * this function gets called when a
 * ICMP Request is received
 * @param source:
 *  The source IP where the ping comes from
 */
static void gotPinged(byte* source) {
  ether.printIp("Ping from: ", source);
}


/**
 * The setup function gets called
 * at the application start
 */
void setup () {
  // setup the serial connection
  Serial.begin(57600);
  Serial.println("[PING]");

  // setup the Ethernet controller
  if (ether.begin(sizeof Ethernet::buffer, macAddress, 53) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
  }
  // use DNS to get our IP and the Gateway
  if (!ether.dhcpSetup()) {
    Serial.println(F("DHCP failed"));
  }

  ether.printIp("IP:       ", ether.myip);
  ether.printIp("Gateway:  ", ether.gwip);

  // use DNS to locate the IP address we want to ping
  if (!ether.dnsLookup(PSTR("www.google.com"))) {
    Serial.println("DNS failed");
  }
  // The resolved IP Address
  ether.printIp("SRV: ", ether.hisip);

  // cregister the ping callback
  ether.registerPingCallback(gotPinged);

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

  // report whenever a reply to our outgoing ping comes back
  if (packetLength > 0 && ether.packetLoopIcmpCheckReply(ether.hisip)) {
    Serial.print("  ");
    Serial.print((micros() - timer) * 0.001, 3);
    Serial.println(" ms");
  }

  // ping a remote server once every few seconds
  if ((micros() - timer) >= 5000000) {
    ether.printIp("Pinging: ", ether.hisip);
    timer = micros();
    ether.clientIcmpRequest(ether.hisip);
  }
}
