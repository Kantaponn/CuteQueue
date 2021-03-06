#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define RXD2 16
#define TXD2 17
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
/*#define WIFI_SSID "HOME WIFI_2.4G"
#define WIFI_PASSWORD "43567890"
*/
#define WIFI_SSID "Redacted"
#define WIFI_PASSWORD "Redacted"

// Insert Firebase project API Key
#define API_KEY "Redacted"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "Redacted" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

void firebase_write_reset(int input){
  if (Firebase.ready() && signupOK){
    if (Firebase.RTDB.setInt(&fbdo, "reset", input)){
      Serial.println("RESET");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("RESET FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
void setup(){
  Serial2.begin(9600,SERIAL_8N1,RXD2,TXD2);
  Serial2.setTimeout(100);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(28800-3600);
  Serial.println("Start Reset");
  firebase_write_reset(1);
  delay(5000);
  firebase_write_reset(0);
  Serial.println("Reset finished");
  
}

void firebase_write(int input,String tim){
  FirebaseJson data;
  data.set("queue", input);
  data.set("time", tim);
  data.set("id", input);
  data.set("status", "queue details");
  data.set("remain", 0);
  if (Firebase.ready() && signupOK){
    //String q=input+'0';
    //String c = String(input);
    if (Firebase.RTDB.setJSON(&fbdo, "queueData/"+String(input)+"/" +String(input), &data)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
bool last =false;
int queue=1;
void loop(){
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
 // Serial.print("HOUR: ");
  //Serial.println(timeStamp);
   String recieve_string = Serial2.readString();
  //Serial.println(recieve_string[0]);
  if(recieve_string[0]=='1' && last ==false){
    firebase_write(queue,timeStamp);
    delay(200);
    Serial.println(queue);
    queue+=1;
    last=true;
  }
  else if(recieve_string[0]=='0')
  {
    //firebase_write(false);
    Serial.println("off");
    last=false;
  }
    
 
}
