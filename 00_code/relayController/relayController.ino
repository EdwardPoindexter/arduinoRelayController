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
  pinMode(switchThree, INPUT_PULLUP); //temp pullup for bard test 
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

  Serial.print("*******     RELAY (POWER) CONTROLLER *********\n");
  Serial.print("*** COMMANDS ARE:| PWRON | PWROFF | STATUS |**\n");
  Serial.print("***************** PUTTY SETUP ****************\n");
  Serial.print("Terminal; Implicit CR in every LF\n");
  Serial.print("Terminal; Local echo Force on\n");
  Serial.print("Use CTRL+J to enter command in putty terminal.\n");
  Serial.print("**********************************************\n");
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
          Serial.println("Command received: PWRON");
          digitalWrite(LED_BUILTIN, HIGH);
          powerOnSeq();
          statusFlag = 1;
        }else if (strcmp(cmd, "PWROFF")  == 0) {
          Serial.println("Command received: PWROFF");
          digitalWrite(LED_BUILTIN, LOW);
          powerOffSeq();
          statusFlag = 0;
        }else if (strcmp(cmd, "STATUS")  == 0) {
          Serial.println("Command received: STATUS");
          if((statusFlag == 1) && (switchStatusFlag == 1)) {
            Serial.println("Comm AND Manual Set Power is ON");
          }else if ((statusFlag == 1) && (switchStatusFlag == 0)) {
            Serial.println("Comm Set Power is ON");
          }else if ((statusFlag == 0) && (switchStatusFlag == 1)) {
            Serial.println("Manually Set Power is ON");           
          }else{
            Serial.println("Power is OFF");   
          }
        }else{
          Serial.println("Command received: unknown!");
        }
        
      }else{
        if(cmdIndex++ >= MAX_CMD_LENGTH){
          cmdIndex = 0;
        }
      }
    }
    return statusFlag;  
}

void loop() {
  // put your main code here, to run repeatedly:
  statusFlag = checkComm(switchStatusFlag);
  switchStatusFlag = checkSwitch(statusFlag);
}
