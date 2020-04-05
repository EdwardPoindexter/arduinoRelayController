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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int voltageOne = 2;
const int voltageTwo = 3;
const int voltageThree = 4;
const int switchOne = 5;
const int switchTwo = 6;
const int switchThree = 7;
const int MAX_CMD_LENGTH = 10;

char cmd[10];
int cmdIndex;
char incomingByte;
int statusFlag;
int switchStatusFlag;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
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
  Serial.begin(9600);
  
  while (!Serial) {
    ; //wait for serial port to connect. Needed for native usb only
  }
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.print("*******     RELAY (POWER) CONTROLLER *********\n");
  Serial.print("*** COMMANDS ARE:| PWRON | PWROFF | STATUS |**\n");
  Serial.print("***************** PUTTY SETUP ****************\n");
  Serial.print("Terminal; Implicit CR in every LF\n");
  Serial.print("Terminal; Local echo Force on\n");
  Serial.print("Use CTRL+J to enter command in putty terminal.\n");
  Serial.print("**********************************************\n");

  //add oled display
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println(F("DEV UNIT"));
  display.setCursor(0, 8);
  display.println(F("192.168.200.100"));
  display.display();
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
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("DEV UNIT"));
  display.setCursor(0, 8);
  display.println(F("192.168.200.100"));
  if (statusFlag == 1){
    display.setCursor(0, 24);
    display.println(F("EXTERNAL CONTROL"));
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.println(F("POWER ON"));
  }else if (switchStatusFlag == 1) {
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.println(F("POWER ON"));    
  }else{
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.println(F("POWER OFF"));
  }
  display.display();
  display.startscrollright(0x02,0x04); //lines where each line 8 pixels 
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

int checkComm(int switchStatusFlag) {
   //check the com port 
   if (incomingByte=Serial.available()>0) {
      
      char byteIn = Serial.read();
      cmd[cmdIndex] = byteIn; 
      
      if(byteIn=='\n'){
        //command finished
        cmd[cmdIndex] = '\0';
        Serial.println(cmd);
        cmdIndex = 0;
        
        if(strcmp(cmd, "PWRON")  == 0){
          Serial.println(F("Command received: PWRON"));
          digitalWrite(LED_BUILTIN, HIGH);
          powerOnSeq();
          statusFlag = 1;
        }else if (strcmp(cmd, "PWROFF")  == 0) {
          Serial.println(F("Command received: PWROFF"));
          digitalWrite(LED_BUILTIN, LOW);
          powerOffSeq();
          statusFlag = 0;
        }else if (strcmp(cmd, "STATUS")  == 0) {
          Serial.println(F("Command received: STATUS"));
          if((statusFlag == 1) && (switchStatusFlag == 1)) {
            Serial.println(F("Comm AND Manual Set Power is ON"));
          }else if ((statusFlag == 1) && (switchStatusFlag == 0)) {
            Serial.println(F("Comm Set Power is ON"));
          }else if ((statusFlag == 0) && (switchStatusFlag == 1)) {
            Serial.println(F("Manually Set Power is ON"));           
          }else{
            Serial.println(F("Power is OFF"));   
          }
        }else{
          Serial.println(F("Command received: unknown!"));
        }
        
      }else{
        if(cmdIndex++ >= MAX_CMD_LENGTH){
          cmdIndex = 0;
        }
      }
    }
    return statusFlag;  
}

int prevStatusFlag = 0;
int prevSwitchStatusFlag = 0;

void loop() {
  // put your main code here, to run repeatedly:
  statusFlag = checkComm(switchStatusFlag);
  switchStatusFlag = checkSwitch(statusFlag);
  if((switchStatusFlag != prevSwitchStatusFlag) || (statusFlag != prevStatusFlag)) {
    displayUpdate(statusFlag,switchStatusFlag);
  }
  prevStatusFlag = statusFlag;
  prevSwitchStatusFlag = switchStatusFlag;  
}
