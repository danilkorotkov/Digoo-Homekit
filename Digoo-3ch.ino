#include "HomeSpan.h" 
#include "DigooTH.h"

#include "homeGW.h"
#include "digoo.h"

#include <esp_task_wdt.h>

#define WDT_TIMEOUT 20 

DigooData s_ch1;
DigooData s_ch2;
DigooData s_ch3;
DigooData* s_ch0;

HomeGW gw(1); // 1 is the number of plugins to be registered 
digoo DigooStation;
uint64_t prev_p = 0;
uint8_t current_ch = 0;
#define RF_RECEIVER_PIN 22 // D2

void setup() {
  Serial.begin(115200);
  homespanInit();
  LOG1("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, false);
  esp_task_wdt_add(NULL);
  
  while (!homeSpan.connected){
    homeSpan.poll();
    }
  esp_task_wdt_reset();
  gw.setup(RF_RECEIVER_PIN);
  gw.registerPlugin(&DigooStation); 

  

}

void homespanInit(){
  homeSpan.setApSSID("Digoo-AP");
  homeSpan.setApPassword("");
  homeSpan.setControlPin(0);
  homeSpan.setStatusPin(2);
  homeSpan.setLogLevel(1);

  homeSpan.setSketchVersion("0.0.2");
  homeSpan.enableOTA(); //homespan-ota
  
  homeSpan.begin(Category::Sensors,"Digoo");
  ///channel 1
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Digoo ch1 sensor"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("ch1"); 
      new Characteristic::FirmwareRevision("0.0.2"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new Digoo(&s_ch1);  
  
  ///channel 2
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Digoo ch2 sensor"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("ch2"); 
      new Characteristic::FirmwareRevision("0.0.2"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new Digoo(&s_ch2);
  ///channel 3
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Digoo ch3 sensor"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("ch3"); 
      new Characteristic::FirmwareRevision("0.0.2"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new Digoo(&s_ch3);
}

void loop() {
  esp_task_wdt_reset();
  uint64_t p = 0;
  homeSpan.poll();
  //gpio_intr_enable(gpio_num_t(RF_RECEIVER_PIN));
  if(DigooStation.available()) 
    if((p = DigooStation.getPacket())) {
      if(p == prev_p) {
        LOG1("smth catched\n");
        current_ch = DigooStation.getChannel(p);
        switch (current_ch) {
          case 1:
            s_ch0 = &s_ch1;
            LOG1("ch1 ");
            break;
          case 2:
            s_ch0 = &s_ch2;
            LOG1("ch2 ");
            break;
          case 3:
            s_ch0 = &s_ch3;
            LOG1("ch3 ");
            break;
        }
        s_ch0 ->batt        = !DigooStation.getBattery(p);
        LOG1(s_ch0 ->batt);LOG1(" ");
        s_ch0 ->temperature = DigooStation.getTemperature(p);
        LOG1(s_ch0 ->temperature);LOG1(" ");
        s_ch0 ->humidity    = (double)DigooStation.getHumidity(p);
        LOG1(s_ch0 ->humidity);LOG1(" \n");
        s_ch0 ->updated     = millis();
        s_ch0 ->isNew[0]    = true;
        s_ch0 ->isNew[1]    = true;
        p = 0;
      }
      prev_p = p;
    }
}
