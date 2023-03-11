#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <Servo.h>

const char ssid[] = "k20";       //  your network SSID (name)
const char pass[] = "00000000";  // your network password
#define host "iottt-f5e25-default-rtdb.firebaseio.com"
#define auth "SP5M1xmdjb09Cjhq1MEXCkFsso9DeW4DvER3RI3Z"

// NTP Servers:
static const char ntpServerName[] = "bd.pool.ntp.org";
// Servo motor pins
#define IR_SENSOR_1_PIN 2
#define IR_SENSOR_2_PIN 14
#define IR_SENSOR_3_PIN 4

// Servo motor pins
#define SERVO_1_PIN 5
#define SERVO_2_PIN 13
#define SERVO_3_PIN 15
Servo servo1;
Servo servo2;
Servo servo3;

int objectCount1 = 0;
int objectCount2 = 0;
int objectCount3 = 0;
bool b = true;



const int timeZone = 6;  // Central European Time



WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void setup() {

  Serial.begin(9600);
  while (!Serial)
    ;  // Needed for Leonardo only
  delay(250);
  servo1.attach(SERVO_1_PIN);
  servo2.attach(SERVO_2_PIN);
  servo3.attach(SERVO_3_PIN);
  // Initialize IR sensors
  pinMode(IR_SENSOR_1_PIN, INPUT);
  pinMode(IR_SENSOR_2_PIN, INPUT);
  pinMode(IR_SENSOR_3_PIN, INPUT);
  //Initialize Burzzer pin
  pinMode(D6, OUTPUT);

  Serial.println("TimeNTP Example");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  Firebase.begin(host, auth);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

time_t prevDisplay = 0;  // when the digital clock was displayed

void loop() {
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {  //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }

  if (b == true) {
    objectCount1 = Firebase.getInt("/Counter/Morning");
    objectCount2 = Firebase.getInt("/Counter/Noon");
    objectCount3 = Firebase.getInt("/Counter/Night");
    b = false;
  }

  // Detect objects using IR sensors
  int object1 = digitalRead(IR_SENSOR_1_PIN);
  int object2 = digitalRead(IR_SENSOR_2_PIN);
  int object3 = digitalRead(IR_SENSOR_3_PIN);
  // Update object counts and Firebase
  if (object1 == LOW) {
    objectCount1++;
    updateFirebase();
  }
  if (object2 == LOW) {
    objectCount2++;
    updateFirebase();
  }
  if (object3 == LOW) {
    objectCount3++;
    updateFirebase();
  }
  int h = hour();
  int m = minute();
  String minuteStr = String(m);
  String hourStr = String(h);
  String timeStr = hourStr + ":" + minuteStr;
  String str_man1 = Firebase.getString("/Manual/Morning");
  String str_man2 = Firebase.getString("/Manual/Noon");
  String str_man3 = Firebase.getString("/Manual/Night");
  String str1 = Firebase.getString("/storetime/time1");
  String str2 = Firebase.getString("/storetime/time2");
  String str3 = Firebase.getString("/storetime/time3");
  //////////////////// For Beeping

  ///////////////////Servo Morning
  if (str_man1 == "true") {

    rotate(servo1);
    objectCount1--;
    updateFirebase();
    Firebase.setString("/Manual/Morning", "false");
  } else if (timeStr == str1) {
    beep(1000, 500, 20);
    rotate(servo1);
    objectCount1--;
    updateFirebase();
    delay(59000);
  }
  /////////////////// Servo Noon
  if (str_man2 == "true") {
    rotate(servo2);
    objectCount2--;
    updateFirebase();
    Firebase.setString("/Manual/Noon", "false");
  } else if (timeStr == str3) {
    beep(1000, 500, 20);
    rotate(servo2);
    objectCount2--;
    updateFirebase();
    delay(59000);
  }
  /////////////////Servo Night
  if (str_man3 == "true") {
    rotate(servo3);
    objectCount3--;
    updateFirebase();
    Firebase.setString("/Manual/Night", "false");
  } else if (timeStr == str3) {
    beep(1000, 500, 20);
    rotate(servo3);
    objectCount3--;
    updateFirebase();
    delay(59000);
  }
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {  //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
  Serial.println();
}
// Delay between object detection and count updates
void updateFirebase() {
  // Set the values of the object counts in Firebase
  Firebase.setInt("/Counter/Morning", objectCount1);
  Firebase.setInt("/Counter/Noon", objectCount2);
  Firebase.setInt("/Counter/Night", objectCount3);
}

void digitalClockDisplay() {
  // digital clock display of the time
  int h = hour();
  int m = minute();
  String minuteStr = String(m);
  String hourStr = String(h);

  String timeStr = hourStr + ":" + minuteStr;
  String str1 = Firebase.getString("/storetime/time1");
  String str2 = Firebase.getString("/storetime/time2");
  String str3 = Firebase.getString("/storetime/time2");
  delay(500);
  Serial.print("Real time: " + timeStr + "\nSchedule time(morning): " + str1 + "\nSchedule time(Noon): " + str2 + "\nSchedule time(Night): " + str3);
  Serial.println();
  // Print debug information to Serial monitor
  Serial.print("----------------------------\n");
  Serial.print("Remaining Medicines: \nMorning: ");
  Serial.print(objectCount1);
  Serial.print("\nNoon: ");
  Serial.print(objectCount2);
  Serial.print("\nNight: ");
  Serial.println(objectCount3);
}
////////////Rotate Servo////////////
void rotate(Servo k) {
  for (int pos = 0; pos <= 180; pos += 1) {
    k.write(pos);
    delay(15);
  }
  for (int pos = 180; pos >= 0; pos -= 1) {
    k.write(pos);
    delay(15);
  }
}
//////////////Bepp the burzzer///////////
void beep(int frequency, int duration, int repetitions) {
  for (int i = 0; i < repetitions; i++) {
    tone(D6, frequency, duration);
    delay(duration);
  }
  noTone(D6);
}
//////////////Print Digits////////////////
void printDigits(int digits) {
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48;      // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];  //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP;  // NTP server's ip address

  while (Udp.parsePacket() > 0)
    ;  // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0;  // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123);  //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
//IOT
