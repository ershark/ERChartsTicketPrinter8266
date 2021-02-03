
#include "SoftwareSerial.h"
#include "thermalprinter.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Time.h>
#include <FS.h>
#include <stdio.h>
#include <FirebaseArduino.h>

#define FIREBASE_HOST "cardioer-cb630.firebaseio.com"
#define FIREBASE_AUTH "MKDVScVnRofBVJeE7iw0bgApOgTJYSnn79FqmZVM"
#define rxPin 14
#define txPin 12

char device[] = "ticketPrintNew/";  //  your network SSID (name)
String location = "2/areas/2/";  // ER
//String location = "2/areas/3/"; // Admisiones
//char ssid[] = "ERCharts-HomeBase";  //  your network SSID (name)
//char pass[] = "33779902";       // your network password
//char ssid[] = "Cardio-Wifi";  //  your network SSID (name)
//char pass[] = "C4rdio##";       // your network password
//char ssid[] = "Galaxy S6 active 1267";  //  your network SSID (name)
//char pass[] = "cquc6054";       // your network password
//char ssid[] = "ERCharts";  //  your network SSID (name)
//char pass[] = "triton4chicken";       // your network password
char ssid[] = "ErCharts";  //  your network SSID (name)
char pass[] = "22Sharks";

int printStatus = 0;
Epson Scangle = Epson(rxPin, txPin); // init the Printer with Output-Pin
int inPin = 15;   // choose the input pin (for a pushbutton)
int val = 0;     // variable for reading the pin status
int ptNumber = 0;    // variable for reading the pin status
int toDay = 0;
int toMonth = 0;
int toYear = 0;
int nowTime;
int aliveCount = 0;
int deadCount = 0;
int streamError = 0;
char ipAdd[15];
String wifiConn = ssid;
unsigned int aliveSyncTime = 60;
unsigned long aliveLastUpdate = 0;

//IPAddress timeServer(132,163,96,3);
IPAddress timeServerIP; // time.nist.gov NTP server address
//const char* ntpServerName = "time-d-g.nist.gov";
//const char* ntpServerName = "utcnist.colorado.edu";
const char* ntpServerName = "us.pool.ntp.org";

const long timeZoneOffset = -14400L;
unsigned int ntpSyncTime = 86400;
unsigned int localPort = 2390;

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP Udp;
unsigned long ntpLastUpdate = 0;
time_t prevDisplay = 0;
// LongPress
long buttonTimer = 0;
long longPressTime = 2000;
boolean buttonActive = false;
boolean longPressActive = false;



void setup() {
  Serial.begin(115200);
  pinMode(inPin, INPUT);    // declare pushbutton as input
  initWifi();

  ///FireBase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  String path  = "/locations/";
  path = path + location;
  path = path +"/queue/lastTicket";  
  String lastNumber = Firebase.getString(path);
  toDay = lastNumber.substring(0, 2).toInt();
  ptNumber = lastNumber.substring(2, 4).toInt();
  Serial.print("\n ToDay<");
  Serial.print(toDay);
  Serial.print(">Pt Number:<");
  Serial.print(ptNumber);
  Serial.println(">");

}

void loop() {
  val = digitalRead(inPin);

  if (val == HIGH) {
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
    }
    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
      longPressActive = true;
    }
  } else {
    if (buttonActive == true) {
      if (longPressActive == true) {
        longPressActive = false;
        printTimeTicket();
      } else {
        ptNumber++;
        printTicket();
        uploadFireBase();
        delay(500);
      }
      buttonActive = false;
    }
  }

  if (now() > 62008) {
    if (toDay != day()) {
      toDay = day();
      toMonth = month();
      toYear = year();
      Serial.println("Restting Pt number to 0 from toDay !=day()");
      Serial.print("toDay: ");
      Serial.println(toDay);
      Serial.print("day() : ");
      Serial.println(day());
      ptNumber = 0;
    }
  }
  if (now() - ntpLastUpdate > ntpSyncTime || now() < 62008) {
    int tries = 0;
    while (!getTimeAndDate() && tries < 10) {
      tries++;
    }
    if (tries < 10) {
      Serial.print("ntp server update success. Server: ");
      Serial.println(timeServerIP);
      printTimeTicket();
      char todayF[11];
      sprintf(todayF, "%02d-%02d-%02d", year(), month(), day());
      if (toDay == 0) {
        toDay = day();
        toMonth = month();
        toYear = year();
        Serial.println("Restting Pt number to 0 from toDay == 0");
        Serial.print("today: ");
        Serial.println(toDay);
        Serial.print(" day(): ");
        Serial.println(day());
        //Edited but not sure
        //ptNumber = 0;
      }
    }
    else {
      Serial.println("ntp server update failed");
    }
  }
  if (now() - aliveLastUpdate > aliveSyncTime || now() < 62008) {
    setAlive();
  }
}

void uploadFireBase() {
  int arrivalEpoch = now();
  char numberToDisplay[2];
  sprintf(numberToDisplay, "%02d%02d", day(), ptNumber);
  String base =  "/locations/";
  base = base + location;
  base = base + "/tickets/";
  base = base + arrivalEpoch;
  base = base + "/ticketNumber/";
  Firebase.setString(base, numberToDisplay);
  Serial.println("FireBase Arduino Push");
  delay(1000);
  if (Firebase.failed()) {
    Firebase.setString(base, numberToDisplay);
    delay(1000);
    if (Firebase.failed()) {
      Firebase.setString(base, numberToDisplay);
      delay(1000);
    }
  }
  String baseTotal =  "/locations/";
  baseTotal = baseTotal + location;
  baseTotal = baseTotal + "/queue/lastTicket/";
  
  Firebase.setString(baseTotal, numberToDisplay);
  Serial.println("FireBase Arduino Push Totals");
  delay(1000);
  if (Firebase.failed()) {
    Firebase.setString(baseTotal, numberToDisplay);
    delay(1000);
    if (Firebase.failed()) {
      Firebase.setString(baseTotal, numberToDisplay);
      delay(1000);
    }
  }
}
String firebaseTimeDate() {
  char timeToDisplay[20];
  sprintf(timeToDisplay, "%02d:%02d:%02d-%02d-%02d-%02d", hour(), minute(), second(), year(), month(), day());
  return timeToDisplay;
}

void setAlive() {
  aliveLastUpdate = now();
  String alive = firebaseTimeDate();
  String baseAlive =  "/locations/";
  baseAlive = baseAlive + location;
  baseAlive = baseAlive + "/areas/";
  baseAlive = baseAlive + "/Stream/";
  baseAlive = baseAlive + device;

  if (deadCount > 0) {
    baseAlive = baseAlive + "dead/";
    deadCount = 0;
  } else {
    baseAlive = baseAlive + "alive/";
  }
  baseAlive = baseAlive + wifiConn;
  baseAlive = baseAlive + "/";
  Serial.print("Setting alive status<");
  Serial.print(baseAlive);
  Serial.print(">Value:");
  Serial.println(alive);
  Firebase.setString(baseAlive, alive);
  delay(500);
  if (Firebase.failed()) {
    Serial.print("Error: ");
    Serial.println(Firebase.error());
    Serial.print("Setting /alive status 2<");
    Serial.print(baseAlive);
    Serial.print(">Value:");
    Serial.println(alive);
    Firebase.setString(baseAlive, alive);
    delay(1000);
    if (Firebase.failed()) {
      Serial.print("Error: ");
      Serial.println(Firebase.error());
      Serial.print("Setting /alive status 2<");
      Serial.print(baseAlive);
      Serial.print(">Value:");
      Serial.println(alive);
      Firebase.setString(baseAlive, alive);
      delay(1000);
    }
  }

}

void updatePtNumber() {
  File log = SPIFFS.open("/nextPt.txt", "w");
  if (!log)
  {
    Serial.println("file open failed.");
  } else {
    char ptArrival[14];
    sprintf(ptArrival, "%02d:%02d:%02d-%02d", year(), month(), day(), ptNumber);
    log.println(ptArrival);
    Serial.print("ptArrival: ");
    Serial.println(ptArrival);
  }
  log.close();
}
void initWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String timeToDisplayText() {
  char timeToDisplay[20];
  sprintf(timeToDisplay, "%02d:%02d:%02d %02d-%02d-%02d", hour(), minute(), second(), year(), month(), day());
  return timeToDisplay;
}
void printTicket() {
  Scangle.start();
  Scangle.justifyCenter();
  Scangle.boldOn();
  Scangle.doubleSize();
  Scangle.println("Centro Cardiovascular");
  Scangle.normalSize();
  Scangle.doubleHeightOn();
  //Scangle.doubleHeightOff();
  Scangle.println("Sala de Emergencia Especializada");
  Scangle.println("Cardiovascular Adultos");
  Scangle.doubleHeightOff();
  Scangle.doubleSize();
  char numberToDisplay[2];
  sprintf(numberToDisplay, "%02d%02d", day(), ptNumber);
  Scangle.println("Enfermeria");
  Scangle.magText();
  Scangle.println(numberToDisplay);
  Scangle.boldOn();
  Scangle.normalSize();

  Scangle.qrCodeStart(7, 7);
  Scangle.print(numberToDisplay);
  Scangle.qrCodeEnd();

  Scangle.feed(1);
  Scangle.doubleWidth();
  //Scangle.reverseOn();
  Scangle.print(" ");
  Scangle.print(timeToDisplayText());
  Scangle.println(" ");
  Scangle.normalSize();
  Scangle.println("Paciente entrege este boleto.");

  Scangle.partialCut();
//Second ticket
  Scangle.doubleHeightOn();
  Scangle.println("Sala de Emergencia Especializada");
  Scangle.println("Cardiovascular Adultos");

  Scangle.doubleSize();
  Scangle.println("Paciente");
  Scangle.magText();
  Scangle.println(numberToDisplay);
  Scangle.boldOn();
  Scangle.normalSize();

  Scangle.qrCodeStart(7, 7);
  Scangle.print(numberToDisplay);
  Scangle.qrCodeEnd();
  Scangle.feed(1);
  Scangle.doubleWidth();
  Scangle.print(" ");
  Scangle.print(timeToDisplayText());
  Scangle.println(" ");
  Scangle.normalSize();
  Scangle.println("Paciente retenga este boleto.");
  Scangle.println("Sera llamado por este numero en el sistema.");

  Scangle.cut();
  
  
  
  updatePtNumber();

  Serial.println(timeToDisplayText());
  Serial.print("today: ");
  Serial.print(toYear);
  Serial.print("-");
  Serial.print(toMonth);
  Serial.print("-");
  Serial.println(toDay);
  Serial.print("ptNumber: ");
  Serial.println(ptNumber);
  Serial.print("now: ");
  Serial.println(now());



}
void printTimeTicket() {
  Scangle.start();
  Scangle.justifyCenter();
  Scangle.boldOn();
  Scangle.doubleSize();
  Scangle.println("Hospital Cardiovascular");
  Scangle.normalSize();
  //  Scangle.doubleWidth();
  Scangle.doubleHeightOn();
  Scangle.println("Sala de Emergencia Especializada");
  Scangle.println("Cardiovascular Adultos");
  Scangle.doubleHeightOff();
  Scangle.doubleSize();
  Scangle.println("NTP Update");
  Scangle.normalSize();
  char numberToDisplay[2];
  sprintf(numberToDisplay, "%02d%02d", day(), ptNumber);
  Scangle.print("Last Patient: ");
  Scangle.println(numberToDisplay);
  //  Scangle.qrCodeStart(7,7);
  //  Scangle.print(numberToDisplay);
  //  Scangle.qrCodeEnd();

  //  Scangle.partialCut();
  //  Scangle.print(numberToDisplay);
  //  Scangle.qrCodeEnd();
  Scangle.feed(1);
  Scangle.characterSet(0);
  Scangle.print("Time updated to NTP Server: ");
  Scangle.println(ntpServerName);
  Scangle.doubleWidth();
  //  Scangle.reverseOn();
  Scangle.print(" ");
  Scangle.print(timeToDisplayText());
  Scangle.println(" ");
  //  Scangle.reverseOff();
  Scangle.cut();
  delay(2000);
}
int getTimeAndDate() {
  int flag = 0;
  Udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServerIP);
  sendNTPpacket(timeServerIP);
  delay(1000);
  if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord, lowWord, epoch;
    highWord = word(packetBuffer[40], packetBuffer[41]);
    lowWord = word(packetBuffer[42], packetBuffer[43]);
    epoch = highWord << 16 | lowWord;
    epoch = epoch - 2208988800 + timeZoneOffset;
    flag = 1;
    setTime(epoch);
    ntpLastUpdate = now();
  }
  return flag;
}

// Do not alter this function, it is used by the system
unsigned long sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
