/*
Sketch for the leve0 of the setup */

#include <Wire.h>
#include <SoftwareSerial.h>

#define DEBUG
#define FALSE      0
#define TRUE       1
#define LOAD0_R   A0
#define LOAD1_R   A1
#define LOAD2_R   A2
#define LOAD3_R   A3

#define LOAD0_W   0x02
#define LOAD1_W   0x03
#define LOAD2_W   0x04
#define LOAD3_W   0x05

#define NODE_ADDRESS   0x02
#define SEND_DATA      72U
#define PARENT_RX      0x06
#define PARENT_TX      0x07
#define PARENT_ADDRESS 0x01
#define BufferSize     0x0A   // child receives only asigned loads
#define NUM_LOADS      0x03
#define TOLERANCE      20U
#define TIMEOUT        10U

SoftwareSerial parent(5,7);
unsigned char  recvBuffer[BufferSize];
unsigned int   aliveLED = 13;
unsigned int   aliveLEDState = LOW;
unsigned int   I2CLED  = 12;
unsigned int   I2CLEDState = LOW;
unsigned int   parentDataReq = LOW;    // Signal to send data
unsigned char  dataSendState = LOW;

unsigned int NodeTotalLoad = 1234;
unsigned int NodeTotalDemand = 1234;
unsigned int NodeAssignedLoad = 1230;
float NodePrio = 12.34; 
float Pstep = 0.1;

struct Load{
  unsigned char readPin;
  unsigned char writePin;
  //unsigned int DL;
  unsigned int DCL;
  unsigned int ASL;
  float fixPrio;
  float dynPrio;
  char state;
};

Load loads[NUM_LOADS];


//Add more if necessary
void LoadInit(){
  loads[0] = (struct Load){LOAD0_R, LOAD0_W, 0, 0,/* 0,*/ 1.0, 1.0, HIGH};
  loads[1] = (struct Load){LOAD1_R, LOAD1_W, 0, 0,/* 0,*/ 2.0, 2.0, HIGH};
  loads[2] = (struct Load){LOAD2_R, LOAD2_W, 0, 0,/* 0,*/ 3.0, 3.0, HIGH};
  
  for(int i =0; i<NUM_LOADS; i++){
    pinMode(loads[i].writePin, OUTPUT);
  }
}


void toggleLED(){
  if(HIGH == aliveLEDState){
    aliveLEDState = LOW;
  }
  else
    aliveLEDState = HIGH;
    digitalWrite(aliveLED, aliveLEDState);
}

void toggleI2CLED(){
  if(HIGH == I2CLEDState){
    I2CLEDState = LOW;
  }
  else
    I2CLEDState = HIGH;
    digitalWrite(I2CLED, I2CLEDState);
}


void setup(){
  pinMode(aliveLED,OUTPUT);
  pinMode(I2CLED, OUTPUT);
 // pinMode(dataSend, INPUT);

  LoadInit();
 // Wire.begin(NODE_ADDRESS);
//  Wire.onReceive(I2Cevent);
//  attachInterrupt(0, HWint, RISING);
  Serial.begin(9600);
  parent.begin(9600);
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                      Conversions Task
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

void sendFloat(float data, SoftwareSerial parent){
  unsigned char *c = (unsigned char*)&data; 
  Serial.write("Writing Float : ");
  parent.write(*c);
  parent.write(*(c+1));
  parent.write(*(c+2));
  parent.write(*(c+3));
  
   Serial.print(*c, DEC); Serial.print(" ");
   Serial.print(*(c+1), DEC); Serial.print(" ");
   Serial.print(*(c+2), DEC); Serial.print(" ");
   Serial.print(*(c+3), DEC); Serial.print("\n");
}

void sendWord(unsigned int data, SoftwareSerial parent){
  
  Serial.print("Writing Word : ");
  unsigned char *c = (unsigned char*)&data; 
  parent.write(*c);
  parent.write(*(c+1));
  Serial.print(*c, DEC); Serial.print(" ");
  Serial.print(*(c +1), DEC); Serial.print("\n");
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                      Cyclic Task
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
void cycLoadRead(){
  #ifdef DEBUG
  Serial.print("Inside cycLoadRead\n");
  #endif
  NodeTotalLoad = 0;
  for(int i =0; i< NUM_LOADS; i++){
    loads[i].DCL = analogRead(loads[i].readPin);

    NodeTotalLoad += loads[i].DCL;
  }
 
  #ifdef DEBUG
  Serial.print("Inside cycLoadRead\n");
  #endif
}

void cycLoadWrite(){
  #ifdef DEBUG
  Serial.print("Inside cycLoadWrite\n");
 #endif
  
  for (int i=0; i<NUM_LOADS; i++){

    digitalWrite(loads[i].writePin, loads[i].state);
  }
  
  #ifdef DEBUG
  Serial.print("Exit cycLoadWrite\n");
  #endif
}

void cycLoadCalc(){
  #ifdef DEBUG
  Serial.print("Inside cycLoadCalc\n");
  #endif
  
  NodeTotalLoad = 0;
  for(int i=0; i<NUM_LOADS; i++){
    if(loads[i].state == HIGH){ // only if the load is ON should we check it as a demand
          NodeTotalLoad += loads[i].DCL;
    }
  }

  #ifdef DEBUG
  Serial.print("Exit cycLoadCalc\n");
  #endif
}


void cycComm(){
  
   #ifdef DEBUG
  Serial.print("Inside cycComm\n");
  #endif
  
  toggleI2CLED();
  Serial.print("\n============================================\n");
//  if(LOW == digitalRead(dataSend)){
//     Serial.print("\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
//    Serial.print("\n Setting datasend1 Low\n");
//   I2CSendState = LOW;
//  }
  
   /* Cyclical comm handled during cyclically */
// if((HIGH == parentDataReq)&& (LOW == dataSendState)){  // should send only one time
      //I2CSendState = HIGH;  // we will send data only on next level change on datasend pin
      Serial.print("\n Setting datasend1 HIGH\n");
    
      // we need to send data/     
      Serial.print("Sending the data to Parent\n");
     Serial.print("Parent :");
     Serial.print(PARENT_ADDRESS);
     Serial.print("\n");
  //   Wire.beginTransmission(PARENT_ADDRESS);
     
 //    Wire.write(NODE_ADDRESS);
     Serial.print("ID sent\n");
     sendWord(NodeTotalLoad, parent);
     Serial.print("Total Load written sent\n");
     sendWord(NodeTotalDemand, parent);
     Serial.print("Total Demand Written\n");
     sendFloat(NodePrio, parent);
     Serial.print("Prio Written :\n");
      dataSendState = HIGH; //we should not sent data continuously
   //  Wire.endTransmission();
     Serial.print("Sent the data to Parent\n");
//  }
   
   
   Serial.print("\n\nI2CSendState : "); Serial.print(dataSendState, DEC); Serial.print("\n");
   Serial.print("\n============================================\n");
  #ifdef DEBUG
  Serial.print("Exit cycComm\n");
  #endif
}

void cycPrioCalc(){
  #ifdef DEBUG
  Serial.print("Inside cycPrioCalc\n");
  #endif
  
  float sum = 0.0, product = 1.0;
  
  for(int i=0; i< NUM_LOADS; i++){
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

    product *= loads[i].dynPrio;

  }
   NodePrio = product/sum;
   
  #ifdef DEBUG
  Serial.print("Exit cycPrioCalc\n");
  #endif
}

unsigned char isLowestPrio(int index){
    // check if the mentioned node has the highest PRIORITY
    for(int i =0; i< NUM_LOADS; i++){
      if(loads[i].state == HIGH){
         if(loads[i].dynPrio > loads[index].dynPrio){
          return FALSE;
        }
      }
    }
   
     return TRUE; 
}

void cycLogic(){
  #ifdef DEBUG
  Serial.print("Inside cycLogic\n");
  #endif
  unsigned char timeout = 0;
  unsigned int NodeLoadTotalLocal = 0;
  // TO switch the lods off

  if(NodeTotalLoad > NodeAssignedLoad){
    
      while(NodeTotalLoad > NodeAssignedLoad){
          unsigned char index = 0;
          timeout++;
          for(int i =0; i<NUM_LOADS; i++){
           // is this the lowest priority
          if(loads[i].state == HIGH){          
            if(isLowestPrio(i)){
            loads[i].state = LOW;
            NodeTotalLoad -= loads[i].DCL;
         }
      }
    }
    if(timeout > TIMEOUT) break;
   }
  } 
  
  // To switch the loads ON
    //Assign whatever each one is demanding
  //  Serial.print("demand went down\n");
    for(int i=0; i< NUM_LOADS; i++)
    {
      NodeLoadTotalLocal = NodeTotalLoad;
      if(loads[i].state == LOW){
        //if(NodeTotalLoad + loads[i].DCL  <= NodeAssignedLoad)  // we are in the safe zone
              //loads[i].ASL = loads[i].DCL;    // Keep this in check
              
              if(NodeLoadTotalLocal + loads[i].DCL < NodeAssignedLoad){
                    loads[i].state = HIGH;
              }
              
              for(int j=0; j< NUM_LOADS; j++){
                if((loads[i].dynPrio < loads[j].dynPrio)){
                  if(isLowestPrio(j)){
                    if((NodeTotalLoad + loads[i].DCL) < NodeAssignedLoad){
                          loads[j].state = LOW;
                          NodeTotalLoad -= loads[j].DCL;
                          loads[i].state = HIGH;
                          NodeTotalLoad += loads[i].DCL;
                  }
                }
                }      
      
              }
      }
    }
  #ifdef DEBUG
  Serial.print("Exit cycLogic\n");
  #endif
}

 void cycListen(){
   #ifdef DEBUG
   Serial.print("Inside cyc Listen\n");
   #endif
   
   static int i = 0;
   if(parent.available()){
     Serial.print("\nChildReceived Something\n");
     while(parent.available()){
       recvBuffer[i] = parent.read();
     }
     i =0;
   
   
   if(recvBuffer[0] == SEND_DATA){
       parentDataReq = HIGH;
       dataSendState = LOW;   // this will be changed to HIGH in comm so that data is not sent repeatedly
   }
  }
  
  #ifdef DEBUG
   Serial.print("Exit cyc Listen\n");
  #endif 
 }
 
 void debug(){
     #ifdef DEBUG
      Serial.print(" Inside sebug \n");
     #endif 
     for(int i =0; i< NUM_LOADS; i++){
      Serial.print("Load "); Serial.print(i, DEC);Serial.print(":\n");
      Serial.print("State : "); Serial.print(loads[i].state, DEC);Serial.print("\n");
      Serial.print("DCL : "); Serial.print(loads[i].DCL, DEC);Serial.print("\n");
      Serial.print("PRIO : "); Serial.print(loads[i].dynPrio);Serial.print("\n");
     } 
     
     Serial.print("Node Data: \n");
     Serial.print("NodeTotalLoad : "); Serial.print(NodeTotalLoad); Serial.print("\n");
     Serial.print("NodeTotalDemand : "); Serial.print(NodeTotalDemand); Serial.print("\n");
     Serial.print("NodeAssignedLoad : "); Serial.print(NodeAssignedLoad); Serial.print("\n");
     Serial.print("NodePrio : "); Serial.print(NodePrio); Serial.print("\n");
     Serial.print("=======================================================");
 } 
  
void NodeTask(){
  /*
    Cyclic Task to be executed by the Node
    */
  #ifdef DEBUG
  Serial.print("Inside NodeTask\n");
  #endif
  
  
    cycLoadRead();
    //cycLoadWrite();
    cycLoadCalc();
    cycPrioCalc();
    cycComm();
    cycLogic();
    cycLoadWrite();
    cycListen();   
    toggleLED();
    //debug();
  
       
  #ifdef DEBUG
  Serial.print("Exit NodeTask\n");
  #endif
}



void loop(){
    //digitalWrite(led, HIGH);
    NodeTask();
    //delay(1000);
    //digitalWrite(led, LOW);
    //delay(1000);
   
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
            


