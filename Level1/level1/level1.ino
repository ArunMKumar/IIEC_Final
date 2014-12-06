/*
Sketch for the level1 of the setup 
Data to child sent over I2C, data to parent sent over bluetooth
*/

#include <Wire.h>


#define NODE_ADDRESS   0x01
//#define PARENT_ADDRESS 0x00
#define BufferSize     0x0A
#define child1Addr     0x02
#define child2Addr     0x03
#define Commduration   50

unsigned int led = 13;
unsigned int ledState = LOW;
unsigned int dataSend1 = 12;    // Signal to send data slave1
unsigned int dataSend2 = 6;    // signal to send data slave2
unsigned char dataSendState1 = LOW;
unsigned char dataSendState2 = HIGH;

unsigned int recvData = 6;
char recvBuffer[BufferSize];

unsigned int child1TotalLoad = 0; 
unsigned int child1DemandedLoad = 0; 
float  child1Prio = 3.0; 
unsigned int child2TotalLoad = 0; 
unsigned int child2DemandedLoad = 0; 
float  child2Prio = 3.0; 


unsigned int NodeTotalLoad = 1234;
unsigned int NodeTotalDemand = 1234;
unsigned int NodeAssignedLoad = 1234;
float NodePrio = 12.34; 

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
  pinMode(dataSend1, OUTPUT);
  pinMode(dataSend2, OUTPUT);
  pinMode(recvData, INPUT);
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
void cycLoadCalc(){
  NodeTotalLoad = child1TotalLoad + child2TotalLoad;
  NodeTotalDemand = child1DemandedLoad + child2DemandedLoad;
   Serial.print("Tload:");
   Serial.print(NodeTotalLoad);
   Serial.print("\n");
    Serial.print("Dload:");
   Serial.print(NodeTotalDemand);
   Serial.print("\n");
}

void cycPrioCalc(){
  float sum = 0.0, product = 1.0;
  
  sum = child1Prio + child2Prio;
  product = child1Prio * child2Prio;
  NodePrio = product/sum;
  
   Serial.print("Prio:");
   Serial.print(NodePrio);
   Serial.print("\n");
}

void cycComm(){
  
  Serial.print("Pulling 1 high \n");
  
  while(dataSendState1 == LOW){
    // request data from slave1
  //  Serial.print("Inside DataSend 1\n");
   //   Serial.print("DataSend1  : ");
  //  Serial.print(dataSendState1);
  //  Serial.print("\n");
    
      dataSendState1 = HIGH;
      //   Serial.print("DataSend1  : ");
  //  Serial.print(dataSendState1);
 //   Serial.print("\n");
    
      digitalWrite(dataSend1, dataSendState1);
   //    Serial.print("Waait for I2C\n");
      delay(Commduration);  // hold it high so that slave sends data
      //if(dataSendState1 == LOW) break;
  } 
  dataSendState1 =LOW;   // remove afterwards
  digitalWrite(dataSend1, dataSendState1); // remove afterwads 
  Serial.print("Pulling 1 LOW \n");
  /*
  while(dataSendState2 == LOW){
    // request data from slave1
      dataSendState1 = HIGH;
      digitalWrite(dataSend1, dataSendState2);
      delay(10);  // hold it high so that slave sends data
       if(dataSendState2 == LOW) break;
  }*/ 
}

void SerialDEbug(){
  
  Serial.print("child1TotalLoad :");
  Serial.print(child1TotalLoad);
  Serial.print("   child1DemandedLoad :");
  Serial.print(child1DemandedLoad);
  Serial.print("   child1Prio :");
  Serial.print(child1Prio);
  
  Serial.print("\nchild2TotalLoad :");
  Serial.print(child2TotalLoad);
  Serial.print("    child2DemandedLoad :");
  Serial.print(child2DemandedLoad);
  Serial.print("   child2Prio :");
  Serial.print(child2Prio)
  
   Serial.print("child1TotalLoad :");
  Serial.print(child1TotalLoad);
  Serial.print("   child1DemandedLoad :");
  Serial.print(child1DemandedLoad);
  Serial.print("   child1Prio :");
  Serial.print(child1Prio);


unsigned int NodeTotalLoad = 1234;
unsigned int NodeTotalDemand = 1234;
unsigned int NodeAssignedLoad = 1234;
float NodePrio = 12.34; 
  
void NodeTask(){
  /*
    Cyclic Task to be executed by the Node
    */
    //cycLoadRead();
   // Serial.print("Task Started \n");
     cycLoadCalc();
    cycPrioCalc();
   // Serial.print("Cyc Comm \n");
    cycComm();
  //  Serial.print("Cyc Comm Exit \n");
  //   Serial.print("Task Exited \n\n\n\n");
}



void loop(){
    
    NodeTask();
    Serial.write("Alive\n");
    digitalWrite(led, LOW);
   
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                     ISR
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

void receiveEvent(int howMany){
 
  digitalWrite(led, HIGH);
  if(dataSendState1 == HIGH){
    while(0 < Wire.available()){
      for(int i =0; i< BufferSize; i++){
        recvBuffer[i] = Wire.read();
      }
    }
    dataSendState1 == LOW;
   child1TotalLoad = *((unsigned int*)recvBuffer);
   child1DemandedLoad = *(((unsigned int*)recvBuffer) + 1);
   child1Prio =   *(((float*)recvBuffer) + 2*sizeof(int));
  }
  
  // was child 2 requested
  if(dataSendState2 == HIGH){
    while(0 < Wire.available()){
      for(int i =0; i< BufferSize; i++){
        recvBuffer[i] = Wire.read();
      }
    }
    dataSendState2 == LOW;
   child2TotalLoad = *((unsigned int*)recvBuffer);
   child2DemandedLoad = *(((unsigned int*)recvBuffer) + 1);
   child2Prio =   *(((float*)recvBuffer) + 2*sizeof(int));
  }
  dataSendState1  = LOW;
  digitalWrite(dataSend1, dataSendState1);
  dataSendState2  = LOW; 
  digitalWrite(dataSend2, dataSendState2);

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
