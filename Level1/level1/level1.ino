/*
Sketch for the level1 of the setup 
Data to child sent over I2C, data to parent sent over bluetooth
*/

#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial.child1(

//#define DEBUG
#define NODE_ADDRESS   0x01
#define CHILD1RX       0x02
#define CHILD1TX       0x03
#define CHILD2RX       0x04
#define CHILD2TX       0x05
//#define PARENT_ADDRESS 0x00
#define BufferSize     0x0A
#define child1Addr     0x02
#define child2Addr     0x03
#define CommdurationH   100
#define CommdurationL   100


SoftwareSerial.child1(CHILD1RX, CHILD1TX);
SoftwareSerial.child1(CHILD2RX, CHILD3TX);
static unsigned int I2CCount = 0;

unsigned int aliveLED = 13;
unsigned int aliveLEDState = LOW;
unsigned int I2Cled = 12;
unsigned int I2CledState = LOW;
unsigned int dataSend1 = 5;    // Signal to send data slave1
unsigned int dataSend2 = 6;    // signal to send data slave2
unsigned char dataSendState1 = LOW;
unsigned char dataSendState2 = LOW;
unsigned int childAssignedLoad = 0x00;

//unsigned int recvData = 6;
char recvBuffer[BufferSize];

unsigned int child1TotalLoad = 0; 
unsigned int child1DemandedLoad = 0; 
float  child1Prio = 3.0; 
unsigned int child1AssignedLoad = 0;

unsigned int child2TotalLoad = 0; 
unsigned int child2DemandedLoad = 0; 
float  child2Prio = 3.0; 
unsigned int child2AssignedLoad =0;

unsigned int NodeTotalLoad = 1234;
unsigned int NodeTotalDemand = 1234;
//unsigned int NodeAssignedLoad = 1234;
float NodePrio = 12.34; 
unsigned int NodeReserve = 0;

void toggleLED(){
  if(HIGH == aliveLEDState){
    aliveLEDState = LOW;
  }
  else
    I2CledState = HIGH;
    digitalWrite(aliveLED, aliveLEDState);
}




void setup(){
  pinMode(I2Cled,OUTPUT);
  pinMode(aliveLED, OUTPUT);
  pinMode(dataSend1, OUTPUT);
  pinMode(dataSend2, OUTPUT);
//  pinMode(recvData, INPUT);
  Wire.begin(NODE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  Serial.print("Node_Address :");
  Serial.print(NODE_ADDRESS);
  Serial.print("\n");
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                      Conversions Task
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

void sendFloat(float data){
  #ifdef DEBUG
  Serial.print("Inside send Float\n");
  #endif
  char *c = (char*)&data; 
  Wire.write(*c);
  Wire.write(*(c+1));
  Wire.write(*(c+2));
  Wire.write(*(c+3));
  #ifdef DEBUG
  Serial.print("Exit send Float\n");
  #endif
}

void sendWord(unsigned int data){
  #ifdef DEBUG
  Serial.print("Inside send Word\n");
  #endif
  char *c = (char*)&data; 
  Wire.write(*c);
  Wire.write(*(c+1));
  #ifdef DEBUG
  Serial.print("Exit send Word\n");
  #endif
}


/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                      Cyclic Task
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
void cycLoadCalc(){
  #ifdef DEBUG
  Serial.print("Inside cycLoadCalc\n");
  #endif
  
  NodeTotalLoad = child1TotalLoad + child2TotalLoad;
  NodeTotalDemand = child1DemandedLoad + child2DemandedLoad;
   Serial.print("Tload:");
   Serial.print(NodeTotalLoad);
   Serial.print("\n");
    Serial.print("Dload:");
   Serial.print(NodeTotalDemand);
   Serial.print("\n");
   
  #ifdef DEBUG
  Serial.print("Exit cycLoadCalc\n");
  #endif
}

void transmitAssignedLoad(){
 // #ifdef DEBUG
  Serial.print("Inside transmitAssignedLoad \n");
  //#endif
   Serial.print("Sending to child1 \n");
  Wire.beginTransmission(child1Addr);
  sendWord(child1AssignedLoad);
  Wire.endTransmission();
   Serial.print("Sending to child2 \n");
  Wire.beginTransmission(child2Addr);
  sendWord(child2AssignedLoad);
  Serial.print("BeforeEndTransmisison\n");
  Wire.endTransmission();
    Serial.print("afterEndTransmisison\n");
  
  #ifdef DEBUG
  Serial.print("Exit transmitAssignedLoad \n");
  #endif
}
  

void cycAssignedLoadCalc(){
  #ifdef DEBUG
  Serial.print("Inside cycAssigned Load Calc\n");
  #endif
  
  Serial.print("Here\n");
  /*
  The loads to be assigned to the childs 
  */
  childAssignedLoad = analogRead(A0) * 6;  // for six loads in this project
   Serial.print("Here1\n");
  if(NodeTotalDemand < childAssignedLoad){
    child1AssignedLoad = child1DemandedLoad;
    child2AssignedLoad = child2DemandedLoad;
    NodeReserve = childAssignedLoad -NodeTotalDemand ;
  }
   Serial.print("Here2\n");
   if(NodeTotalDemand > childAssignedLoad){
    if(child1Prio < child2Prio){
        child1AssignedLoad = child1DemandedLoad;
        childAssignedLoad -= child1AssignedLoad;
        child2AssignedLoad = childAssignedLoad;
        NodeReserve = 0;
    }
    else{
        child2AssignedLoad = child2DemandedLoad;
        childAssignedLoad -= child2AssignedLoad;
        child1AssignedLoad = childAssignedLoad;
       NodeReserve = 0; 
    }
   }
     Serial.print("Here3\n");
    transmitAssignedLoad();
  #ifdef DEBUG
  Serial.print("exit cycAssigned Load Calc\n");
  #endif
      
}


void cycPrioCalc(){
  
  #ifdef DEBUG
  Serial.print("Inside cycPrioCalc\n");
  #endif
  
  float sum = 0.0, product = 1.0;
  
  sum = child1Prio + child2Prio;
  product = child1Prio * child2Prio;
  NodePrio = product/sum;
  
   Serial.print("Prio:");
   Serial.print(NodePrio);
   Serial.print("\n");
   
  #ifdef DEBUG
  Serial.print("Exit cycPrioCalc\n");
  #endif
}

void cycComm(){
  
  #ifdef DEBUG
  Serial.print("Inside cycComm\n");
  #endif
  Serial.print("\ndataSendState1 :");
  Serial.print(dataSendState1, DEC);
  Serial.print("\ndataSendState2 :");
  Serial.print(dataSendState2, DEC);
  Serial.print("\n");
 
  if(dataSendState1 == LOW){
  
      dataSendState1 = HIGH;
      Serial.print("Pulling 1 high \n");
      digitalWrite(dataSend1, dataSendState1);
      delay(CommdurationH);
      dataSendState1 =LOW;   // pull the line low now
      digitalWrite(dataSend1, dataSendState1);
      delay(CommdurationL);    
      Serial.print("Pulling 1 LOW \n");
  } 
  

   if(dataSendState2 == LOW){
  
      dataSendState2 = HIGH;
      Serial.print("Pulling 2 high \n");
      digitalWrite(dataSend2, dataSendState2);
      delay(CommdurationH);
      dataSendState2 =LOW;   // pull the line low now
      digitalWrite(dataSend2, dataSendState2);
      delay(CommdurationL);    
      Serial.print("Pulling 2 LOW \n");
  } 
  
  #ifdef DEBUG
  Serial.print("Exit cycComm\n");
  #endif
}

void debug(){
   #ifdef DEBUG
  Serial.print("Inside Debug \n");
  #endif
  Serial.print("\n==============================================================================\n");
  Serial.print("child1TotalLoad :");
  Serial.print(child1TotalLoad, DEC);
  Serial.print("   child1DemandedLoad :");
  Serial.print(child1DemandedLoad, DEC);
  Serial.print("   child1Prio :");
  Serial.print(child1Prio);
  
  Serial.print("\nchild2TotalLoad :");
  Serial.print(child2TotalLoad, DEC);
  Serial.print("    child2DemandedLoad :");
  Serial.print(child2DemandedLoad,DEC);
  Serial.print("   child2Prio :");
  Serial.print(child2Prio);
  
  Serial.print("\nI2CCount : ");
  Serial.print(I2CCount, DEC);
  Serial.print("\n");

  Serial.print("\n==============================================================================\n");
  
  #ifdef DEBUG
  Serial.print("Exit Debug \n");
  #endif
}


  
void NodeTask(){
 #ifdef DEBUG
  Serial.print("Inside NodeTask \n");
  #endif
  
    cycLoadCalc();
    cycPrioCalc();
    cycComm();
    cycAssignedLoadCalc();
    debug();
    
 #ifdef DEBUG
  Serial.print("Exit NodeTask \n");
  #endif
}



void loop(){
  #ifdef DEBUG
  Serial.print("Inside loop \n");
  #endif
  toggleLED();
    NodeTask();
    //Serial.write("Alive\n");
   // delay(3000);
   // digitalWrite(I2Cled, LOW);
  #ifdef DEBUG
  Serial.print("Exit loop \n");
  #endif
   
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                     ISR
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

void receiveEvent(int howMany){
 
  toggleLED();
  I2CCount++;
    while(0 < Wire.available()){
      for(int i =0; i< BufferSize; i++){
        recvBuffer[i] = Wire.read();
      }
    }
  // dataSendState1 == LOW;
  if(recvBuffer[0] = 0x02){
   child1TotalLoad = *((unsigned int*)&recvBuffer[1]);
   child1DemandedLoad = *(((unsigned int*)&recvBuffer[1]) + 1);
   child1Prio =   *(((float*)&recvBuffer[1]) + 1);
  }
  
/*  // was child 2 requested
  if(dataSendState2 == HIGH){
    while(0 < Wire.available()){
      for(int i =0; i< BufferSize; i++){
        recvBuffer[i] = Wire.read();
      }
    }*/
   // dataSendState2 == LOW;
    if(recvBuffer[0] = 0x03){
   child2TotalLoad = *((unsigned int*)recvBuffer);
   child2DemandedLoad = *(((unsigned int*)recvBuffer) + 1);
   child2Prio =   *(((float*)recvBuffer) + 2*sizeof(int));
  }
//  dataSendState1  = LOW;
///  digitalWrite(dataSend1, dataSendState1);
//  dataSendState2  = LOW; 
//  digitalWrite(dataSend2, dataSendState2);

}
/*
void receiveEvent(int howMany)
{
  digitalWrite(led, HIGH);
  while(0 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
//    Serial.print(c, DEC);         // print the character
 //   Serial.print("\n");
  }
//  Serial.print("\n\n\n\n");
 // int x = Wire.read();    // receive byte as an integer
 // Serial.println(x);         // print the integer
}*/
