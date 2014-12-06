/*
Sketch for the leve0 of the setup */

#include <Wire.h>

#define DEBUG
#define LOAD0_R   A0
#define LOAD1_R   A1
#define LOAD2_R   A3
#define LOAD3_R   A5

#define LOAD0_W   0x02
#define LOAD1_W   0x03
#define LOAD2_W   0x04
#define LOAD3_W   0x05

#define NODE_ADDRESS   0x02
#define PARENT_ADDRESS 0x01
#define BufferSize     0x04
#define NUM_LOADS      0x03
#define TOLERANCE      20U

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

Load loads[NUM_LOADS];


//Add more if necessary
void LoadInit(){
  loads[0] = (struct Load){LOAD0_R, LOAD0_W, 0, 0, 0, 1.0, 1.0, LOW};
  loads[1] = (struct Load){LOAD1_R, LOAD1_W, 0, 0, 0, 2.0, 2.0, LOW};
  loads[2] = (struct Load){LOAD2_R, LOAD2_W, 0, 0, 0, 3.0, 3.0, LOW};
}


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
  Serial.write("Inside WriteFloat\n");
  Serial.write("Writing : ");
   Serial.print(*c, DEC);
   Serial.print("\n");
  /* 
  Wire.write(*c);
  Wire.write(*(c+1));
  Wire.write(*(c+2));
  Wire.write(*(c+3));*/
  Wire.write('h');
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
  #ifdef DEBUG
  Serial.print("Inside cycLoadRead\n");
  #endif
  
  loads[0].DCL = analogRead(loads[0].readPin);
  loads[1].DCL = analogRead(loads[1].readPin);
  loads[2].DCL = analogRead(loads[2].readPin);
 // loads[3].DCL = analogRead(loads[3].readPin);
 
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
    NodeTotalLoad += loads[i].DCL;
  }
  
  #ifdef DEBUG
  Serial.print("Exit cycLoadCalc\n");
  #endif
}


void cycComm(){
  
   #ifdef DEBUG
  Serial.print("Inside cycComm\n");
  #endif
  
   /* Cyclical comm handled during cyclically */
   if(HIGH == digitalRead(dataSend)){
     // we need to send data
     Serial.print("Sending the data to Parent\n");
     Serial.print("Parent :");
     Serial.print(PARENT_ADDRESS);
     Serial.print("\n");
     Wire.beginTransmission(PARENT_ADDRESS);
  
     sendWord(NodeTotalLoad);
     sendWord(NodeTotalDemand);
     sendFloat(NodePrio);
     Serial.print("Float sent\n");
     Wire.write('H');
     Serial.print(NodePrio);
     Serial.print("\n");
 
     Wire.endTransmission();
         Serial.print("Sent the data to Parent\n");
   }
   
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
     // Serial.print(sum);
    // Serial.print("\n");
    product *= loads[i].dynPrio;
   // Serial.print(product);
   //  Serial.print("\n");
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
        if(loads[i].DynPrio > loads[index].DynPrio){
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
  
  // TO switch the lods off
  while(NodeTotalLoad > NodeAssignedLoad){
    unsigned char index = 0;
    
    for(int i =0; i<(NUM_LOADS - 1); i++){
           // is this the lowest priority
        if(loads[i].state == HIGH){
            if(isLowestPrio(i)){
            loads[i].state = LOW;
            NodeTotalLoad -= loads[i].DCL;
         }
      }
    }
   } 
  
  // To switch the loads ON
  else{
    //Assign whatever each one is demanding
    for(int i=0; i< NUM_LOADS; i++)
    {
      if(loads[i].state == LOW){
        if(NodeTotalLoad + loads[i].DCL  <= NodeAssignedLoad){  // we are in the safe zone
              loads[i].ASL = loads[i].DCL;    // Keep this in check
              loads[i].state = HIGH;    // alow everyone to function
        }
    }
  }
  
   for(int i=0; i< NUM_LOADS; i++){
        if(loads[i].DCL > loads[i].ASL + TOLERANCE){
             loads[i].state = LOW;
        }
     
  #ifdef DEBUG
  Serial.print("Exit cycLogic\n");
  #endif
}  
}
  
  
void NodeTask(){
  /*
    Cyclic Task to be executed by the Node
    */
  #ifdef DEBUG
  Serial.print("Inside NodeTask\n");
  #endif
  
    cycLogic();
    cycLoadRead();
    cycLoadWrite();
    cycLoadCalc();
    cycPrioCalc();
    cycComm();
    
  #ifdef DEBUG
  Serial.print("Exit NodeTask\n");
  #endif
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
            


