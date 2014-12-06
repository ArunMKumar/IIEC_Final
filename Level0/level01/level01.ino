/*
Sketch for the leve0 of the setup */

#include <Wire.h>


#define Load0_R   A0
#define Load1_R   A1
#define Load2_R   A3
#define Load3_R   A5

#define Load0_W   0x02
#define Load1_W   0x03
#define Load2_W   0x04
#define Load3_W   0x05

#define NODE_ADDRESS   0x01
#define PARENT_ADDRESS 0x00
#define BufferSize     0x04

unsigned int led = 13;
unsigned int ledState = LOW;
unsigned int dataSend = 5;    // Signal to send data
unsigned int recvData = 6;
char recvBuffer[BufferSize];
unsigned int NodeTotalLoad = 1234;
unsigned int NodeTotalDemand = 1234;
unsigned int NodeAssignedLoad = 1234;
float NodePrio = 12.34; 
float Pstep = 0.1;

struct Load{
  unsigned char readPin;
  unsigned char writePin;
  unsigned int DL;
  unsigned int DCL;
  unsigned int ASL;
  float fixPrio;
  float dynPrio;
  char state;
};

Load loads[4];

void toggleLED(){
  if(HIGH == ledState){
    ledState = LOW;
  }
  else
    ledState = HIGH;
    digitalWrite(led, ledState);
}


void setup(){
  pinMode(led,OUTPUT);
  pinMode(dataSend, INPUT);
  pinMode(recvData, INPUT);
  Wire.begin(NODE_ADDRESS);
  Wire.onReceive(I2Cevent);
  Serial.begin(9600);
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                      Conversions Task
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

void sendFloat(float data){
  char *c = (char*)&data; 
  Wire.write(*c);
  Wire.write(*(c+1));
  Wire.write(*(c+2));
  Wire.write(*(c+3));
}

void sendWord(unsigned int data){
  char *c = (char*)&data; 
  Wire.write(*c);
  Wire.write(*(c+1));
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                      Cyclic Task
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
void cycLoadRead(){
  loads[0].DCL = analogRead(loads[0].readPin);
  loads[1].DCL = analogRead(loads[1].readPin);
  loads[2].DCL = analogRead(loads[2].readPin);
  loads[3].DCL = analogRead(loads[3].readPin);
}

void cycLoadCalc(){
  NodeTotalLoad = 0;
  for(int i=0; i<4; i++){
    NodeTotalLoad += loads[i].DCL;
  }
}


void cycComm(){
   /* Cyclical comm handled during cyclically */
   if(HIGH == digitalRead(dataSend)){
     // we need to send data
     Serial.print("Sending the data to Parent\n");
     Wire.beginTransmission(PARENT_ADDRESS);
  
     sendWord(NodeTotalLoad);
     sendWord(NodeTotalDemand);
     sendFloat(NodePrio);
     Serial.print(NodePrio);
     Serial.print("\n");
 
     Wire.endTransmission();
         Serial.print("Sent the data to Parent\n");
   }
}

void cycPrioCalc(){
  float sum = 0.0, product = 1.0;
  
  for(int i=0; i< 4; i++){
    if(loads[i].state == LOW){
      if(loads[i].dynPrio > 1.0){
        loads[i].dynPrio -= Pstep;
      }
      if(loads[i].dynPrio < 1.0){    // stop overshoot
        loads[i].dynPrio = 1.0;
      }
    }
    
    else if(loads[i].state == HIGH){
      if(loads[i].dynPrio < loads[i].fixPrio){
        loads[i].dynPrio += Pstep;
      }
      if(loads[i].dynPrio > loads[i].fixPrio){    // stop overshoot
        loads[i].dynPrio = loads[i].fixPrio;
      }
    }
    // CAlculate the priority of the node
    sum += loads[i].dynPrio;
     // Serial.print(sum);
    // Serial.print("\n");
    product *= loads[i].dynPrio;
   // Serial.print(product);
   //  Serial.print("\n");
  }
   NodePrio = product/sum;
}
  
  
void NodeTask(){
  /*
    Cyclic Task to be executed by the Node
    */
    cycLoadRead();
    cycLoadCalc();
    cycPrioCalc();
    cycComm();
}



void loop(){
    digitalWrite(led, HIGH);
    NodeTask();
    digitalWrite(led, LOW);
   
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                     ISR
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
void I2Cevent(int count){

  while(0 <= Wire.available()){
    for(int i =0; i< BufferSize; i++){
        recvBuffer[i] = Wire.read();
      }
  }
  NodeAssignedLoad = *((unsigned int*)recvBuffer);
}
            


