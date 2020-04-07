/*
Edward Poindexter

Relay controller and power sequencer
coded for arduino nano with SainSmart 4Relay board
added code for physical switches
code monitors usb com port and checks for these commands
PWRON   = turn unit power on 
PWROFF  = turn unit power off
STATUS  = return the unit power on/off status
*/
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <SPI.h>
#include <Ethernet.h>

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiWire oled;

//pins
const int voltageOne = 2;
const int voltageTwo = 3;
const int voltageThree = 4;
const int switchOne = 5;
const int switchTwo = 6;
const int switchThree = 7;
//const int MSS  = 10;
//const int MOSI = 11;
//const int MISO = 12;
//const int SCLK = 13;

const int MAX_CMD_LENGTH = 10;

char cmd[10];
int cmdIndex;
char incomingByte;
int statusFlag;
int switchStatusFlag;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(169, 254, 226, 177);
IPAddress myDns(169, 254, 1, 1);
IPAddress gateway(169, 254, 1, 1);
IPAddress subnet(255, 255, 0, 0);


// telnet defaults to port 23
EthernetServer server(23);
boolean alreadyConnected = false; // whether or not the client was connected previously

void setup() {
  Wire.begin();
  Wire.setClock(400000L);
  
  // put your setup code here, to run once:
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(voltageOne, OUTPUT);
  pinMode(voltageTwo, OUTPUT);
  pinMode(voltageThree, OUTPUT);
  pinMode(switchOne, INPUT_PULLUP);   //temp pullup for board test
  pinMode(switchTwo, INPUT_PULLUP);   //temp pullup for board test
  pinMode(switchThree, INPUT_PULLUP); //temp pullup for board test 
  cmdIndex = 0;
  statusFlag = 0;
  switchStatusFlag = 0;
  //switch voltages off SainSmart Relay is active low
  digitalWrite(voltageOne, HIGH);
  digitalWrite(voltageTwo, HIGH);
  digitalWrite(voltageThree, HIGH);
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // initialize the ethernet device
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  Serial.begin(9600);
  
  while (!Serial) {
    ; //wait for serial port to connect. Needed for native usb only
  }

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);

//  Serial.print("*******     RELAY (POWER) CONTROLLER *********\n");
//  Serial.print("*** COMMANDS ARE:| PWRON | PWROFF | STATUS |**\n");
//  Serial.print("***************** PUTTY SETUP ****************\n");
//  Serial.print("Terminal; Implicit CR in every LF\n");
//  Serial.print("Terminal; Local echo Force on\n");
//  Serial.print("Use CTRL+J to enter command in putty terminal.\n");
//  Serial.print("**********************************************\n");

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start listening for clients
  server.begin();

  Serial.print("Chat server address:");
  Serial.println(Ethernet.localIP());


  //add oled display
  delay(2000);
  oled.clear();
  oled.println("DEV UNIT");
  oled.println(Ethernet.localIP());
}

void powerOnSeq() {
  // power on sequence
  // turn on first supply
  digitalWrite(voltageOne, LOW); //SainSmart Relay, low for on
  // turn on second supply
  delay(1500);
  digitalWrite(voltageTwo, LOW);
  // turn on third supply
  delay(1000);
  digitalWrite(voltageThree, LOW);
}

void powerOffSeq() {
  // power off sequence
  // turn off third supply
  digitalWrite(voltageThree, HIGH);  //SainSmart Relay, high for off
  // turn off second supply
  delay(200);
  digitalWrite(voltageTwo, HIGH);
  // turn off first supply
  delay(500);
  digitalWrite(voltageOne, HIGH);
}

void displayUpdate(int statusFlag, int switchStatusFlag){
  oled.clear();
  oled.set1X();
  oled.println("DEV UNIT");
  oled.println(Ethernet.localIP());
  if (statusFlag == 1){
    oled.set1X();
    oled.println("\nEXTERNAL CONTROL");
    oled.set2X();
    oled.println("\nPOWER ON");
  }else if (switchStatusFlag == 1) {
    oled.set2X();
    oled.println("\nPOWER ON");    
  }else{
    oled.set2X();
    oled.println("\nPOWER OFF");
  }
}

int checkSwitch(int statusFlag) {
  //check the state of external switches
  if (statusFlag == 0){
    if (digitalRead(switchOne) == HIGH){
      digitalWrite(voltageOne, LOW);
    }else{
      digitalWrite(voltageOne, HIGH);
    }
    delay(100);
    if (digitalRead(switchTwo) == HIGH){
      digitalWrite(voltageTwo, LOW);
    }else{
      digitalWrite(voltageTwo, HIGH);
    }
    delay(100);
    if (digitalRead(switchThree) == HIGH){
      digitalWrite(voltageThree,LOW);
    }else{
      digitalWrite(voltageThree,HIGH);
    }
   }else{
     //unit is powered remotely
     //TODO In future display remote controlled on oled    
   }
   if (digitalRead(switchOne) == HIGH && digitalRead(switchTwo) == HIGH && digitalRead(switchThree) == HIGH){
    //manual power is on
    switchStatusFlag = 1;
   }
   if (digitalRead(switchOne) == LOW && digitalRead(switchTwo) == LOW && digitalRead(switchThree) == LOW){
    //manual power is off
    switchStatusFlag = 0;
   }
   return switchStatusFlag;   
}

//int checkComm(int switchStatusFlag) {
//   //check the com port 
//   if (incomingByte=Serial.available()>0) {
//      
//      char byteIn = Serial.read();
//      cmd[cmdIndex] = byteIn; 
//      
//      if(byteIn=='\n'){
//        //command finished
//        cmd[cmdIndex] = '\0';
//        Serial.println(cmd);
//        cmdIndex = 0;
//        
//        if(strcmp(cmd, "PWRON")  == 0){
//          Serial.println(F("Command received: PWRON"));
//          #digitalWrite(LED_BUILTIN, HIGH);
//          powerOnSeq();
//          statusFlag = 1;
//        }else if (strcmp(cmd, "PWROFF")  == 0) {
//          Serial.println(F("Command received: PWROFF"));
//          #digitalWrite(LED_BUILTIN, LOW);
//          powerOffSeq();
//          statusFlag = 0;
//        }else if (strcmp(cmd, "STATUS")  == 0) {
//          Serial.println(F("Command received: STATUS"));
//          if((statusFlag == 1) && (switchStatusFlag == 1)) {
//            Serial.println(F("Comm AND Manual Set Power is ON"));
//          }else if ((statusFlag == 1) && (switchStatusFlag == 0)) {
//            Serial.println(F("Comm Set Power is ON"));
//          }else if ((statusFlag == 0) && (switchStatusFlag == 1)) {
//            Serial.println(F("Manually Set Power is ON"));           
//          }else{
//            Serial.println(F("Power is OFF"));   
//          }
//        }else{
//          Serial.println(F("Command received: unknown!"));
//        }
//        
//      }else{
//        if(cmdIndex++ >= MAX_CMD_LENGTH){
//          cmdIndex = 0;
//        }
//      }
//    }
//    return statusFlag;  
//}

int checkEth(int switchStatusFlag){
  // wait for a new client:
  EthernetClient client = server.available();

  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer:
      client.flush();
      Serial.println("We have a new client");
      client.println("Hello, valid commmands are STATUS PWRON PWROFF");
      alreadyConnected = true;
    }

    if (client.available() > 0) {
      // read the bytes incoming from the client:
      char byteIn = client.read();
      cmd[cmdIndex] = byteIn;
      if(byteIn=='\n'){
        //command finished
        cmd[cmdIndex] = '\0';
        // echo the bytes back to the client:
        //server.println(cmd);
        // echo the bytes to the server as well:
        Serial.println(cmd);
        cmdIndex = 0;

        if(strcmp(cmd, "PWRON")  == 0){
          server.println("Command received: PWRON");
          powerOnSeq();
          statusFlag = 1;
        }else if (strcmp(cmd, "PWROFF")  == 0) {
          server.println("Command received: PWROFF");
          powerOffSeq();
          statusFlag = 0;
        }else if (strcmp(cmd, "STATUS")  == 0) {
          server.println("Command received: STATUS");
          if((statusFlag == 1) && (switchStatusFlag == 1)) {
            server.println("Comm AND Manual Set Power is ON");
          }else if ((statusFlag == 1) && (switchStatusFlag == 0)) {
            server.println("Comm Set Power is ON");
          }else if ((statusFlag == 0) && (switchStatusFlag == 1)) {
            server.println("Manually Set Power is ON");           
          }else{
            server.println("Power is OFF");   
          }
        }else{
          //server.println("Command received: unknown!");
        }
       
      }else{
         if(cmdIndex++ >= MAX_CMD_LENGTH){
          cmdIndex = 0;              
         }  
      }
    }
  }
  return statusFlag;
}

int prevStatusFlag = 0;
int prevSwitchStatusFlag = 0;

void loop() {
  // put your main code here, to run repeatedly:
  statusFlag = checkEth(switchStatusFlag);
  switchStatusFlag = checkSwitch(statusFlag);
  if((switchStatusFlag != prevSwitchStatusFlag) || (statusFlag != prevStatusFlag)) {
    displayUpdate(statusFlag,switchStatusFlag);
  }
  prevStatusFlag = statusFlag;
  prevSwitchStatusFlag = switchStatusFlag;  
}
