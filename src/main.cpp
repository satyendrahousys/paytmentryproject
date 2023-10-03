#include <Arduino.h>
#include <SPI.h>
#include <EthernetLarge.h>

// Paytm entry project
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:

#define EXIT_PIN  9
#define TOTAL_CLIENT 1
#define MAX_TIME 2

unsigned long currentMillis2, previousMillis2, MilliSeconds = 0;
byte Seconds;

boolean IsClientConnected = false;
boolean IsClientAlive = false;

byte mac[] = {0xDE, 0xAD, 0xBA, 0xEF, 0xFE, 0xEC};
IPAddress ip(192, 168, 1, 151);

EthernetServer server(7000);
EthernetClient clients[TOTAL_CLIENT];

String clientData = "";

void setup() {
  Ethernet.begin(mac, ip);   // initializing ethernet
  server.begin();
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print(F("Machine Gate IP: "));
  Serial.println(Ethernet.localIP());

  pinMode(EXIT_PIN, OUTPUT);
  digitalWrite(EXIT_PIN, HIGH);

  IsClientConnected = false;
  IsClientAlive = false;
  currentMillis2 = 0;
  previousMillis2 = 0;
  MilliSeconds = 0;
  Seconds = 0;
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    boolean newClient = true;
    for (byte i = 0; i < TOTAL_CLIENT; i++) {
      if (clients[i] == client) {
        newClient = false;
        break;
      }
    }

    if (newClient) {
      IsClientConnected = true;
      IsClientAlive = true;

      for (byte i = 0; i < TOTAL_CLIENT; i++) {
        if (!clients[i] && clients[i] != client) {
          clients[i] = client;
          client.flush();
          Serial.println(F("Client Connected"));
          client.print("Connected, client number: ");
          client.print(i);
          client.println();
          break;
        }
      }
    }

    if (client.available() > 0) {
      char thisChar = client.read();
      if (thisChar == '|') {
        clientData = "";
      }
      else if (thisChar == '%') {
        Serial.println(clientData);
        IsClientAlive = true;
        if (clientData.equals("OPENEN")) {
          Serial.println("Barrier is opening");
          digitalWrite(EXIT_PIN, LOW);
          delay(500);
          digitalWrite(EXIT_PIN, HIGH);
          delay(500);
        }
      }
      else {
        clientData += thisChar;
      }
    }
  }

  if (IsClientConnected == true) {
    delay(5);
    MilliSeconds++;
    
    if (MilliSeconds >= 200) {
      MilliSeconds = 0;
      Seconds++;
      
      if (Seconds >= MAX_TIME) {
        Seconds = 0;
        if (IsClientAlive == false) {
          IsClientConnected = false;
        }
        else {
          Serial.println("Client Active");
          IsClientAlive = false;
          MilliSeconds = 0;
          Seconds = 0;
          clients[0].println("|HLT%");
        }
      }
    }
  }

  for (byte i = 0; i < TOTAL_CLIENT; i++) {
    if (!(clients[i].connected())) {
      clients[i].stop();
    }
  }
}
