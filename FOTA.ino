#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

unsigned long Last_Attempt=0;
char Global_Firmware_File[30] = "";


void FOTA_Check() {
    if ( (millis() - Last_Attempt) > 60000) {
      Last_Attempt=millis();
      FOTA_Download(Global_Firmware_File);
    }
}

void FOTA_Download(char* f) {
  char buf[20];
  
  char fwUrlBase[100] = "http://vzb.great-site.net/vzb.txt" ;
 /*
  char f2[50] ="";
  if (strcmp(f,"1") ==0) 
      strcpy(f2,"ring.ino.nodemcu"); 
    else { 
      strcpy(f2,"ring.");
      strcat(f2,f);
    }
  
  strcat(fwUrlBase,f2);
  strcat(fwUrlBase,".bin");
*/ 
  Serial.print("Searching for file:"); Serial.println(fwUrlBase);

  WiFiClient client;
  HTTPClient httpClient;
  httpClient.begin( client, fwUrlBase );
  int httpCode = httpClient.GET();
  if( httpCode == 200 ) {
      String newFWVersion = httpClient.getString();

      Serial.print( "Preparing to update from file=" );Serial.println(fwUrlBase);
      
      t_httpUpdate_return ret = ESPhttpUpdate.update( client, fwUrlBase );


      switch(ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          Serial.println();
          itoa (ESPhttpUpdate.getLastError(),buf,10);
//          Web_NewSendURL("log","Firmware_Update:_HTTP_UPDATE_FAILED" ,buf,"","","",WEB_HOST);
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
//          Web_NewSendURL("log","Firmware_Update:HTTP_UPDATE_NO_UPDATES" ,"","","","",WEB_HOST);
          break;
      }
  }
  else {
    Serial.print( "Firmware version check failed, got HTTP response code " );
    Serial.println( httpCode );
//    Web_NewSendURL("log","Firmware_Update:version_check_failed" ,"","","","",WEB_HOST);
  }
  httpClient.end();
 

}
