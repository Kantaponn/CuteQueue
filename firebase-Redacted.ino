#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

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
#define WIFI_SSID "REDACTED"
#define WIFI_PASSWORD "REDACTED"

// Insert Firebase project API Key
#define API_KEY "REDACTED"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "REDACTED"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

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

  Serial.println("Start Reset");
  firebase_write_reset(1);
  delay(5000);
  firebase_write_reset(0);
  Serial.println("Reset finished");
}

void firebase_write(int input){
  if (Firebase.ready() && signupOK){
    if (Firebase.RTDB.setInt(&fbdo, "sensor", input)){
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
   String recieve_string = Serial2.readString();
  //Serial.println(recieve_string[0]);
  if(recieve_string[0]=='1' && last ==false){
    firebase_write(queue);
    delay(200);
    firebase_write(0);
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
