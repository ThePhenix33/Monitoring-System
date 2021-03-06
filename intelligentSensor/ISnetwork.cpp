/*
   ISnetwork

   Yann BLANC

   Handle the Intelligent Sensor network aspect,
   from connecting to the global network to handle
   HTTP requests.

*/




#include "ISnetwork.h"


//Sensor MAC adress set on 002632f0204b


//00-22-8F-10-A0-03
byte mac[] = {
  0x00, 0x22, 0x8F, 0x10, 0xA0, 0x04
};


//HTTP protocol needs port 80
EthernetServer server(80);

EthernetClient userQuery, client;

int ISnetwork::networkCheck() {


  /*This part permits the user to have a visual indicator
     for the network status.
  */




  //In case of internal shield issue

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.");
    digitalWrite(15, HIGH);
    return -1;
  }


  //In case of network shutdown
  else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    digitalWrite(15, HIGH);
    return -2;
  } else {
    digitalWrite(15, LOW);
    return 0;

  }
}

void ISnetwork::networkSetup() {

  //The CS pin of the Ethernet module is GPIO17
  Ethernet.init(17);
  delay(1000);



  Serial.println("Trying to get an IP address using DHCP");

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");

    while (networkCheck() < 0) {
      Serial.println("Attempting to connect to network");
      delay(1);
    }

  } else {

  }
  delay(1000);

  //At this point the IS is connected to the network

  // IPAddress gateway(192, 168, 10, 1); //Only for local switch network
  // Ethernet.setGatewayIP(gateway);
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("My gateway address: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("My DNS address: ");
  Serial.println(Ethernet.dnsServerIP());

  server.begin();

}

struct Command ISnetwork::queryAK() {
  struct Command requestedCommand;
  userQuery = server.available();
  if (userQuery) {

    Serial.println("Command incoming...");
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    char qParam[256] = " ";
    char qValue[256] = " ";
    int param = 0;

    while (userQuery.connected()) {
      // Serial.println("Command connection ON");
      if (userQuery.available()) {
        // Serial.println("Command reading");
        char c = userQuery.read();
        int i = 0;




        /*A command is queried in the URL, in the format ?sensor=Value
           The following lines read these informations and transmit them
           to the card which will then process them in JSON
        */


        while (c == '?' || c == '&') {
          while (c != '=') {
            c = userQuery.read();
            if (c != '=')qParam[i] = c;
            i++;
          }
          Serial.println(" *Parameter identified!");

          //At this point the Parameter has been acknowledged, now the Value is read



          i = 0;

          while ( c != ' ' && c != '&') {
            c = userQuery.read();
            if (c == '&') {
              Serial.println(" *Another parameter next");
              i = 0;
              break;
            }
            if (c != ' ')qValue[i] = c;
            i++;

          }
          Serial.println(" *Value identified!");
          //At this point the Value has been acknowledged

          Serial.print("  | Parameter: ");
          Serial.println((String)qParam);
          Serial.print("  | Value: ");
          Serial.println((String)qValue);

          //For each parameter, the value is set to the associated variable of the struct Command type return

          if (((String)qParam).equals("M")) {
            requestedCommand.mode = ((String)qValue).toInt();
            Serial.println("   \\MODE value set");
          }
          if (((String)qParam).equals("C")) {
            requestedCommand.id = ((String)qValue).toInt();
            Serial.println("   \\ID value set");
          }
          if (((String)qParam).equals("P")) {
            requestedCommand.readingPeriod = ((String)qValue).toInt();
            Serial.println("   \\PERIOD value set");
          }
          if (((String)qParam).equals("m")) {
            requestedCommand.min = ((String)qValue).toFloat();
            Serial.println("   \\MIN value set");
          }
          if (((String)qParam).equals("MM")) {
            requestedCommand.max = ((String)qValue).toFloat();
            Serial.println("   \\MAX value set");
          }
          if (((String)qParam).equals("N")) {
            requestedCommand.logicalLevel = ((String)qValue).toInt();
            Serial.println("   \\LOGICAL LEVEL value set");
          }
          if (((String)qParam).equals("B")) {
            requestedCommand.databank = ((String)qValue).toInt();
            Serial.println("   \\DATA BANK value set");
          }
          if (((String)qParam).equals("I")) {
            requestedCommand.interrupt = ((String)qValue).toInt();
            Serial.println("   \\INTERRUPT value set");
          }
          if (((String)qParam).equals("D")) {
            requestedCommand.flagReset = ((String)qValue).toInt();
            Serial.println("   \\FLAG RESET value set");
          }


          //Parameter and value buffers resetting
          for (int i = 0; i < ((String)qParam).length(); i++) {
            qParam[i] = ' ';

          }
          for (int i = 0; i < ((String)qValue).length(); i++) {
            qValue[i] = ' ';

          }

        }

        /*The code waits for the headers to be read, and then process
           the command.
        */

        if (c == '\n' && currentLineIsBlank) {
          Serial.println("Command acknowledging! \n");

          return requestedCommand;



          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }

    }

  }

  return requestedCommand;
}
void ISnetwork::endQuery() {

  userQuery.stop();
  Serial.println("End of query \n \n");
}
