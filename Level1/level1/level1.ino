/*
Sketch for the level1 of the setup 
Data to child sent over I2C, data to parent sent over bluetooth
*/

#include <Wire.h>
#include <SoftwareSerial.h>



#define DEBUG
#define NODE_ADDRESS   0x01
#define SEND_DATA      72U
#define CHILD1RX       0x02
#define CHILD1TX       0x03
#define CHILD2RX       0x04
#define CHILD2TX       0x05
//#define PARENT_ADDRESS 0x00
#define BufferSize     0x0A
#define FRAME_SIZE     0x08
#define child1Addr     0x02
#define child2Addr     0x03
//#define CommdurationH   100
//#define CommdurationL   100



SoftwareSerial child1(CHILD1RX, CHILD1TX);
SoftwareSerial child2(CHILD2RX, CHILD2TX);
static unsigned int I2CCount = 0;
unsigned char recvBuffer1[BufferSize];
unsigned char recvBuffer2[BufferSize];

unsigned int aliveLED = 13;
unsigned int aliveLEDState = LOW;
unsigned int I2Cled = 12;
unsigned int I2CledState = LOW;
//unsigned int child1 = 5;    // Signal to send data slave1
//unsigned int dataSend2 = 6;    // signal to send data slave2
unsigned char child1DataReq = LOW;
unsigned char child2DataReq = LOW;
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
    aliveLEDState = HIGH;
    digitalWrite(aliveLED, aliveLEDState);
}




void setup(){
  pinMode(I2Cled,OUTPUT);
  pinMode(aliveLED, OUTPUT);
//  pinMode(dataSend1, OUTPUT);
//  pinMode(dataSend2, OUTPUT);
//  pinMode(recvData, INPUT);
//  Wire.begin(NODE_ADDRESS);
//  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  child1.begin(9600);
  child2.begin(9600);
  child1.print("Hello World");
  child2.print("Hello World");
  Serial.print("Node_Address :");
  Serial.print(NODE_ADDRESS);
  Serial.print("\n");
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                      Conversions Task
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

void sendFloat(float data, SoftwareSerial* child){
  #ifdef DEBUG
  Serial.print("Inside send Float\n");
  #endif
  char *c = (char*)&data; 
  child->write(*c);
  child->write(*(c+1));
  child->write(*(c+2));
  child->write(*(c+3));
  #ifdef DEBUG
  Serial.print("Exit send Float\n");
  #endif
}

void sendWord(unsigned int data, SoftwareSerial* child){
  #ifdef DEBUG
  Serial.print("Inside send Word\n");
  #endif
  char *c = (char*)&data; 
  child->write(*c);
  child->write(*(c+1));
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

void transmitAssignedLoad(SoftwareSerial* child1, SoftwareSerial* child2){
  #ifdef DEBUG
  Serial.print("Inside transmitAssignedLoad \n");
  #endif
   Serial.print("Sending to child1 \n");
 // Wire.beginTransmission(child1Addr);
  sendWord(child1AssignedLoad, child1);
 // Wire.endTransmission();
   Serial.print("Sending to child2 \n");
  //Wire.beginTransmission(child2Addr, child);
  sendWord(child2AssignedLoad, child2);
  Serial.print("BeforeEndTransmisison\n");
  //Wire.endTransmission();
    Serial.print("afterEndTransmisison\n");
  
  #ifdef DEBUG
  Serial.print("Exit transmitAssignedLoad \n");
  #endif
}
  

void cycAssignedLoadCalc(){
  #ifdef DEBUG
  Serial.print("Inside cycAssigned Load Calc\n");
  #endif
  
//  Serial.print("Here\n");
  /*
  The loads to be assigned to the childs 
  */
  childAssignedLoad = analogRead(A0) * 6;  // for six loads in this project
  // Serial.print("Here1\n");
  if(NodeTotalDemand < childAssignedLoad){
    child1AssignedLoad = child1DemandedLoad;
    child2AssignedLoad = child2DemandedLoad;
    NodeReserve = childAssignedLoad -NodeTotalDemand ;
  }
 //  Serial.print("Here2\n");
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
    // Serial.print("Here3\n");
    transmitAssignedLoad(&child1, &child2);
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
  Serial.print("\nchild1DataReq :");
  Serial.print(child1DataReq, DEC);
  Serial.print("\nchild2DataReq :");
  Serial.print(child2DataReq, DEC);
  Serial.print("\n");
 child1.write(SEND_DATA); // ----------- Temp
 child2.write(SEND_DATA);
  if(child1DataReq == LOW){
  
      child1DataReq = HIGH;
      Serial.print("Requesting child 1 \n");
      child1.write(SEND_DATA);
     // delay(CommdurationH);
     // dataSendState1 =LOW;   // pull the line low now
    //  digitalWrite(dataSend1, dataSendState1);
    //  delay(CommdurationL);    
   //   Serial.print("Pulling 1 LOW \n");
  } 
  

   if(child2DataReq == LOW){
  
      child2DataReq = HIGH;
      Serial.print("Requesting child2 \n");
      child2.write(SEND_DATA);
     // delay(CommdurationH);
     // dataSendState2 =LOW;   // pull the line low now
     // digitalWrite(dataSend2, dataSendState2);
    //  delay(CommdurationL);    
    //  Serial.print("Pulling 2 LOW \n");
  } 
  
  #ifdef DEBUG
  Serial.print("Exit cycComm\n");
  #endif
}

void debug(){
   #ifdef DEBUG
  Serial.print("Inside Debug \n");
  #endif
  Serial.print("\n=================================PARENT=============================================\n");
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

void cycListen(){
  /*
  here we receive data from the child*/
  #ifdef DEBUG
  Serial.print("Inside cyc Listen \n");
  #endif
  static int i=0,j=0;
  
  if(child1.available()){
    Serial.print("ParentReceived Something\n");
    while(child1.available()){
      recvBuffer1[i] = child1.read();
      i++;
    }
  }
  
   if(child2.available()){
    while(child2.available()){
      recvBuffer2[i] = child1.read();
      j++;
    }
  }
  
//  if(i == FRAME_SIZE){
    
    child1TotalLoad =*((unsigned int*)recvBuffer1[0]); 
    Serial.print("child1TotalLoad: ");Serial.print(child1TotalLoad, DEC); Serial.print(" ");
    child1DemandedLoad = *((unsigned int*)recvBuffer1[0]+ 1);  
    Serial.print("child1DemandedLoad: ");Serial.print(child1DemandedLoad, DEC); Serial.print(" ");
    child1Prio = *((float*)recvBuffer1[0]+ 1);
    Serial.print("child1Prio: ");Serial.print(child1Prio, DEC); Serial.print(" ");
    i = 0;

//  }
//  if(j == FRAME_SIZE){
    child2TotalLoad =*((unsigned int*)recvBuffer2[0]); 
    child2DemandedLoad = *((unsigned int*)recvBuffer2[0]+ 1);  
    child2Prio = *((float*)recvBuffer2[0]+ 1);
    Serial.print("child1TotalLoad: ");Serial.print(child2TotalLoad, DEC); Serial.print(" ");
    child1DemandedLoad = *((unsigned int*)recvBuffer1[0]+ 1);  
    Serial.print("child1DemandedLoad: ");Serial.print(child2DemandedLoad, DEC); Serial.print(" ");
    child1Prio = *((float*)recvBuffer1[0]+ 1);
    Serial.print("child1Prio: ");Serial.print(child2Prio, DEC); Serial.print(" ");
    j = 0;
//  }
  
   #ifdef DEBUG
  Serial.print("Exit cyc Loisten \n");
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
    cycListen();
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

/*void receiveEvent(int howMany){
 
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
  */
/*  // was child 2 requested
  if(dataSendState2 == HIGH){
    while(0 < Wire.available()){
      for(int i =0; i< BufferSize; i++){
        recvBuffer[i] = Wire.read();
      }
    }*/
   // dataSendState2 == LOW;
 //   if(recvBuffer[0] = 0x03){
//   child2TotalLoad = *((unsigned int*)recvBuffer);
//   child2DemandedLoad = *(((unsigned int*)recvBuffer) + 1);
//   child2Prio =   *(((float*)recvBuffer) + 2*sizeof(int));
//  }
//  dataSendState1  = LOW;
///  digitalWrite(dataSend1, dataSendState1);
//  dataSendState2  = LOW; 
//  digitalWrite(dataSend2, dataSendState2);

//}
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
