#include "ISnetwork.h"


//Sensor MAC adress set on 002632f0204b
byte mac[] = {
  0x00, 0x26, 0x32, 0xF0, 0x20, 0x4B
};


//HTTP protocol needs port 80
EthernetServer server(80);

EthernetClient userQuery;

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
      delay(1);
    }
  }
  delay(1000);

  //At this point the IS is connected to the network
  IPAddress gateway(192, 168, 10, 1);
  Ethernet.setGatewayIP(gateway);
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
    String allParams[5], allValues[5];
    int sent = 0;
    int param=0;
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


        while (c == '?') {
          while (c != '=') {
            c = userQuery.read();
            if (c != '=')qParam[i] = c;
            i++;
          }
          Serial.println("Parameter identified!");

          //At this point the Parameter has been acknowledged, now the Value is read



          i = 0;

          while ( c != ' ') {
            c = userQuery.read();
            if (c == '?') {
              param++;
              Serial.println("Nouveau Paramètre");
              i = 0;
              break;
            }
            if (c != ' ')qValue[i] = c;
            i++;

          }
          Serial.println("Value identified!");
          //At this point the Value has been acknowledged

          allParams[sent]=(String)qParam;
           allValues[sent]=(String)qValue;
          Serial.print(" | Parameter: ");
          Serial.println(allParams[sent]);
          Serial.print(" | Value: ");
          Serial.println(allValues[sent]);


          if(((String)qParam).equals("M"))requestedCommand.mode=((String)qValue).toInt();
          if(((String)qParam).equals("C"))requestedCommand.id=((String)qValue).toInt();
          if(((String)qParam).equals("P"))requestedCommand.readingPeriod=((String)qValue).toInt();
          if(((String)qParam).equals("m"))requestedCommand.min=((String)qValue).toInt();
          if(((String)qParam).equals("MM"))requestedCommand.max=((String)qValue).toInt();
          if(((String)qParam).equals("N"))requestedCommand.logicalLevel=((String)qValue).toInt();
          if(((String)qParam).equals("B"))requestedCommand.dataBank=((String)qValue).toInt();
          if(((String)qParam).equals("I"))requestedCommand.interrupt=((String)qValue).toInt();
         
          
          sent++;
   for (int cStr = 0; cStr < 256; cStr++) {
            qParam[cStr] = ' ';
            qValue[cStr] = ' ';
          }
        }

        //c = userQuery.read();

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
  DynamicJsonDocument doc(1024);

  delay(1);
  doc["error"] = "Unknown command";
  serializeJson(doc, userQuery);
  userQuery.stop();
  Serial.println("End of query \n \n");
}
