
#include <SoftwareSerial.h>
// IMPORT ALL REQUIRED LIBRARIES
#include <NewPing.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
   
//**********ENTER IP ADDRESS OF SERVER******************//

#define HOST_IP     "172.16.192.48"   // REPLACE WITH IP ADDRESS OF SERVER ( IP ADDRESS OF COMPUTER THE BACKEND IS RUNNING ON) 
#define HOST_PORT   "8080"            // REPLACE WITH SERVER PORT (BACKEND FLASK API PORT)
#define route       "api/update"      // LEAVE UNCHANGED 
#define idNumber    "620160532"       // REPLACE WITH YOUR ID NUMBER 

// WIFI CREDENTIALS
#define SSID        "MonaConnect"      // "REPLACE WITH YOUR WIFI's SSID"   
#define password    "c4hHyBmqksdy"  // "REPLACE WITH YOUR WiFi's PASSWORD" 

#define stay         100
 
//**********PIN DEFINITIONS******************//

 
#define espRX         11
#define espTX         10
#define espTimeout_ms 300

#define TRIGGER_PIN 7
#define ECHO_PIN 6
 
//other definitions
#define rad 30.75
#define max_height 77.763
 
/* Declare your functions below */
void espInit();
void espSend(char command[] );
void espUpdate(char mssg[]);
double getWaterHeight();
 

SoftwareSerial esp(espRX, espTX); 
NewPing sonar(TRIGGER_PIN, ECHO_PIN); 


void setup(){

  Serial.begin(115200); 
  // Configure GPIO pins here

 

  espInit();  
 
}

void loop(){ 
   
  // send updates with schema ‘{"id": "student_id", "type": "ultrasonic", "radar": 0, "waterheight": 0, "reserve": 0, "percentage": 0}’
  double radar = sonar.ping_in() - 16.737;
  double waterHeight = max_height - radar;

  double reserve = calcReserve(waterHeight);
  double percentage = (waterHeight / max_height) * 100;
  
  StaticJsonDocument<1000> doc;
  char mssg[290] = {0};

  Serial.println(radar);

  doc["id"] = idNumber;
  doc["type"] = "ultrasonic";
  doc["radar"] = radar;
  doc["waterheight"] = waterHeight;
  doc["reserve"] = reserve;
  doc["percentage"] = percentage;

  serializeJson(doc, mssg);

  // snprintf(mssg,sizeof(mssg),"{\"id\": \"%s\", \"type\": \"ultrasonic\", \"radar\": 0, \"waterheight\": %.2f, \"reserve\": %.2f, \"percentage\": %.2f}",idNumber,waterHeight,reserve,percentage);

  // Serial.println(mssg);
  espUpdate(mssg);


  delay(1000);  
}

 
void espSend(char command[] ){   
    esp.print(command); // send the read character to the esp    
    while(esp.available()){ Serial.println(esp.readString());}    
}


void espUpdate(char mssg[]){ 
    char espCommandString[50] = {0};
    char post[500]            = {0};

    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",HOST_IP,HOST_PORT); 
    espSend(espCommandString);    //starts the connection to the server
    delay(2000);
    while(esp.available()){
  Serial.println(esp.readString());
   }

    // GET REQUEST 
    // snprintf(post,sizeof(post),"GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);

    // POST REQUEST
    snprintf(post,sizeof(post),"POST /%s HTTP/1.1\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);
  
    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSEND=%d\r\n", strlen(post));
    espSend(espCommandString);    //sends post length
    delay(stay);
    Serial.println(post);
    espSend(post);                //sends POST request with the parameters 
    delay(2000);
    
    espSend("AT+CIPCLOSE\r\n");   //closes server connection
   }

void espInit(){
    char connection[100] = {0};
    esp.begin(115200); 
    Serial.println("Initiallizing");
    esp.println("AT"); 
    delay(1000);
    esp.println("AT+CWMODE=1");
    delay(1000);
    while(esp.available()){ Serial.println(esp.readString());} 

    snprintf(connection, sizeof(connection),"AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,password);
    esp.print(connection);

    delay(3000);  //delay time to get IP

    if(esp.available()){   Serial.print(esp.readString());}
    
    Serial.println("\nFinish Initializing");    
   
}

//***** Design and implement all util functions below ******
 
double calcReserve(float height){
  return (M_PI * pow(rad,2) * height) / 231.0;
}
