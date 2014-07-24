/***************************************************
  LaundryNow Arduino Code

  A 2-axis accelerometer attached to Arduino with Adafruit cc3000 Wifi shield.
  Monitors the accelerometer data and sends the data to the server every 500 miliseconds
  for near real time updates.

 ****************************************************/

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ,
                                          ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);

#define WLAN_SSID "puvisitor" // cannot be longer than 32 characters!
#define WLAN_PASS  ""
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_UNSEC

/**
 * Amount of time to wait (in milliseconds) with no data
 * received before closing the connection.  If you know the server
 * you're accessing is quick to respond, you can reduce this value.
 */

#define IDLE_TIMEOUT_MS  3000

// API url
#define WEBSITE "cyanogen32-laundry.jit.su"


// Sets up the HW and the CC3000 module (called automatically on startup)
uint32_t ip;
const int xPin = 2;		// X output of the accelerometer
const int yPin = 3;		// Y output of the accelerometer
int Xsum, Ysum, InstAccelX, InstAccelY, AvgAccelX, AvgAccelY;
char url [100];


// SETUP
void setup(void)
{

  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n"));

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  // Initialise the module
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }

  // SSID scan
  // listSSIDResults();

  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }

  Serial.println(F("Connected!"));

  // Wait for DHCP to complete
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }

  // Display the IP address DNS, Gateway, etc.
  while (! displayConnectionDetails()) {
    delay(1000);
  }

  // Send Request
  ip = 0;

  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }

  cc3000.printIPdotsRev(ip);

  /**
   * Try connecting to the website.
   * Note: HTTP/1.1 protocol is used to keep the server from closing the connection
   * before all data is read.
   **/

  int infinite =0;
  while(infinite == 0){
  int pulse;

  getCoordinates(&pulse);

  String pulseString = String(pulse);
  char pulsechar [10];
  pulseString.toCharArray(pulsechar, 10);

  Serial.print(pulseString);


  char* url1 = "/recieve_pulse?model=81920&pulse=";
  char url [100];
  strcpy(url, url1);
  strcat(url, pulsechar);


  Serial.print(url);

  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);

  if (www.connected()) {
    www.fastrprint(F("GET "));
    www.fastrprint(url);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
  } else {
    Serial.println(F("Connection failed"));
    return;
  }

  Serial.println(F("-------------------------------------"));

  Serial.println("");
  /* Read data until either the connection is closed, or the idle timeout is reached. */
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  www.close();

  }

  Serial.println(F("-------------------------------------"));

  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
  /* the next time your try to connect ... */
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();

}

void loop(void)
{
 delay(5000);
}


void getCoordinates(int* xcord){
  // variables to read the pulse widths:
  int pulseX, pulseY;
  pulseX = pulseIn(xPin, HIGH);
  InstAccelX = ((pulseX / 10) - 500) * 8;
  InstAccelX = InstAccelX*InstAccelX;
  InstAccelX = sqrt(InstAccelX);
  *xcord = pulseX/100;
}

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
