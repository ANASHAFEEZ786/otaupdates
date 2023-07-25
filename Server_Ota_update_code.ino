#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

const char * ssid = "A";
const char * password = "123456789";


String FirmwareVer = {
  "0.9.4"
};
// #define URL_fw_Version "https://raw.githubusercontent.com/EuroSistemi/Otaupdate/main/bin_version.txt"
#define URL_fw_Version "https://prezzomedio.archigia.it/Firmware/bin_version.txt"
#define URL_fw_Bin "https://prezzomedio.archigia.it/Firmware/firmware.bin"

//#define URL_fw_Version "http://cade-make.000webhostapp.com/version.txt"
//#define URL_fw_Bin "http://cade-make.000webhostapp.com/firmware.bin"

void connect_wifi();
void firmwareUpdate();
int FirmwareVersionCheck();

unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long previousMillis_2 = 0;
const long interval = 60000;
const long mini_interval = 1000;
void repeatedCall() {
  static int num=0;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (FirmwareVersionCheck()) {
      firmwareUpdate();
    }
  }
  if ((currentMillis - previousMillis_2) >= mini_interval) {
    previousMillis_2 = currentMillis;
    Serial.print("idle loop...");
    Serial.print(num++);
    Serial.print(" Active fw version:");
    Serial.println(FirmwareVer);
   if(WiFi.status() == WL_CONNECTED) 
   {
       Serial.println("wifi connected");
   }
   else
   {
    connect_wifi();
   }
  }
}

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button_boot = {
  0,
  0,
  false
};
/*void IRAM_ATTR isr(void* arg) {
    Button* s = static_cast<Button*>(arg);
    s->numberKeyPresses += 1;
    s->pressed = true;
}*/

void IRAM_ATTR isr() {
  button_boot.numberKeyPresses += 1;
  button_boot.pressed = true;
}


void setup() {
  pinMode(button_boot.PIN, INPUT);
  attachInterrupt(button_boot.PIN, isr, RISING);
  Serial.begin(115200);
  Serial.print("Active firmware version:");
  Serial.println(FirmwareVer);
  pinMode(LED_BUILTIN, OUTPUT);
  connect_wifi();
}
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(100);   
  repeatedCall();
}

void connect_wifi() {
  Serial.println("Waiting for WiFi");
  WiFi.begin(ssid, password);
  delay(100);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void firmwareUpdate(void) {
  WiFiClientSecure client;
  
  client.setInsecure();
  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}


int FirmwareVersionCheck(void)
{
String payload;
WiFiClientSecure client;
String baseURL = URL_fw_Version;
HTTPClient https;

int httpCode = https.GET();
delay(100);
// if (client) 
//   {
    client.setInsecure();
    
 if (https.begin(client, baseURL)) 
    
    { 
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
  
  

  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}