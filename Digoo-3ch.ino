#include "HomeSpan.h" 
#include "DigooTH.h"

#include "homeGW.h"
#include "digoo.h"
#include "weather.h"

#include <esp_task_wdt.h>

#define WDT_TIMEOUT 20 

DigooData s_ch1;
DigooData s_ch2;
DigooData s_ch3;

DigooData* DigooChannelArray[3] = {&s_ch1, &s_ch2, &s_ch3};

HomeGW gw(2); //  is the number of plugins to be registered 
digoo DigooStation;
weather WeatherStation;
uint64_t prev_p = 0;
uint8_t current_ch = 0;
#define RF_RECEIVER_PIN 22 // D2

void setup() {
  Serial.begin(115200);
  gw.setup(RF_RECEIVER_PIN);
  gw.registerPlugin(&DigooStation); 
  gw.registerPlugin(&WeatherStation); 
    
  LOG1("Configuring WDT...\n");
  esp_task_wdt_init(WDT_TIMEOUT, false);
  esp_task_wdt_add(NULL);
  
  esp_task_wdt_reset();
  homespanInit();
}

void homespanInit(){
  homeSpan.setApSSID("Digoo-AP");
  homeSpan.setApPassword("");
  homeSpan.setControlPin(0);
  homeSpan.setStatusPin(2);
  homeSpan.setLogLevel(1);

  homeSpan.setSketchVersion("0.0.3");
  homeSpan.enableOTA(); //homespan-ota
  
  homeSpan.begin(Category::Sensors,"Digoo");
  ///channel 1
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Digoo ch1 sensor"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("ch1"); 
      new Characteristic::FirmwareRevision("0.0.3"); 
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
  //gpio_intr_enable(gpio_num_t(RF_RECEIVER_PIN));
  
  if(WeatherStation.available()) {
    if((p = WeatherStation.getPacket())) {
       if(p == prev_p) {
        
        LOG1("Weather:    ");     LOG1(WeatherStation.getString(p));      LOG1(" ");
        LOG1("ID: ");             LOG1(WeatherStation.getId(p));          LOG1(" ");
        LOG1("Channel: ");        LOG1(WeatherStation.getChannel(p));     LOG1(" ");
        LOG1("Battery: ");        LOG1(WeatherStation.getBattery(p));     LOG1(" ");        
        LOG1("Temperature: ");    LOG1(WeatherStation.getTemperature(p)); LOG1(" ");
        LOG1("Humidity: ");       LOG1(WeatherStation.getHumidity(p));    LOG1("\n");
        
        p = 0;
      }
    prev_p = p;
    }
  }
  
  if(DigooStation.available()) 
    if((p = DigooStation.getPacket())) {
      if(p == prev_p) {
        current_ch = DigooStation.getChannel(p);
                
        DigooChannelArray[current_ch - 1] ->batt        = !DigooStation.getBattery(p);
        DigooChannelArray[current_ch - 1] ->temperature = DigooStation.getTemperature(p);
        DigooChannelArray[current_ch - 1] ->humidity    = (double)DigooStation.getHumidity(p);
        DigooChannelArray[current_ch - 1] ->updated     = millis();
        DigooChannelArray[current_ch - 1] ->isNew[0]    = true;
        DigooChannelArray[current_ch - 1] ->isNew[1]    = true;
                
        LOG1("Digoo:    ");     LOG1(DigooStation.getString(p));      LOG1(" ");
        LOG1("ID: ");           LOG1(DigooStation.getId(p));          LOG1(" ");
        LOG1("Channel: ");      LOG1(DigooStation.getChannel(p));     LOG1(" ");
        LOG1("Battery: ");      LOG1(DigooStation.getBattery(p));     LOG1(" ");        
        LOG1("Temperature: ");  LOG1(DigooStation.getTemperature(p)); LOG1(" ");
        LOG1("Humidity: ");     LOG1(DigooStation.getHumidity(p));    LOG1("\n");
        
        p = 0;
      }
      prev_p = p;
    }
  homeSpan.poll();
}
