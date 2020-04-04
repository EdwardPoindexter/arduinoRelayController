/*
Edward Poindexter

Relay controller and power sequencer
coded for arduino nano with SainSmart 4Relay board

*/


const int voltageOne = 2;
const int voltageTwo = 3;
const int voltageThree = 4;
//const int switchOne = 5;
//const int switchTwo = 6;
//const int switchThree = 7;
const int MAX_CMD_LENGTH = 10;

char cmd[10];
int cmdIndex;
char incomingByte;
int statusFlag;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(voltageOne, OUTPUT);
  pinMode(voltageTwo, OUTPUT);
  pinMode(voltageThree, OUTPUT);
  //pinMode(switchOne, INPUT);
  //pinMode(switchTwo, INPUT);
  //pinMode(switchThree, INPUT);  
  cmdIndex = 0;
  statusFlag = 0;
  //switch voltages off SainSmart Relay is active low
  digitalWrite(voltageOne, HIGH);
  digitalWrite(voltageTwo, HIGH);
  digitalWrite(voltageThree, HIGH);
  Serial.begin(9600);
  while (!Serial) {
    ; //wait for serial port to connect. Needed for native usb only
  }

  Serial.print("Welcome to my test!\n");
  Serial.print("***************** PUTTY SETUP ****************\n");
  Serial.print("Terminal; Implicit CR in every LF\n");
  Serial.print("Terminal; Local echo Force on\n");
  Serial.print("Use CTRL+J to enter command in putty terminal.\n");
  Serial.print("**********************************************\n");
}

void powerOnSeq() {
  // power on sequence
  // turn on first supply
  digitalWrite(voltageOne, LOW); //SainSamrt Relay, low for on
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

void loop() {
  // put your main code here, to run repeatedly:
  //if (Serial.available())
  //  Serial.write(Serial.read());

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
          if(statusFlag == 1) {
            Serial.println("Power is ON");
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
}
