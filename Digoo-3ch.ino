#include <ArduinoJson.h>

#include "HomeSpan.h" // homespan-ota
#include "DigooTH.h"

#include "homeGW.h"
#include "digoo.h"
#include "weather.h"
#include "fanju.h"

#include <esp_task_wdt.h>

#define WDT_TIMEOUT 100 

DigooData s_ch1;
DigooData s_ch2;
DigooData s_ch3;

DigooData w_ch1;
DigooData w_ch2;
DigooData w_ch3;

DigooData f_ch1;
DigooData f_ch2;
DigooData f_ch3;

DigooData* DigooChannelArray[3]   = {&s_ch1, &s_ch2, &s_ch3};
DigooData* WeatherChannelArray[3] = {&w_ch1, &w_ch2, &w_ch3};
DigooData* FanjuChannelArray[3]   = {&f_ch1, &f_ch2, &f_ch3};

HomeGW gw(3); //  is the number of plugins to be registered 
digoo DigooStation;
weather WeatherStation;
fanju FanjuStation;

uint64_t prev_p = 0;
uint8_t current_ch = 0;
#define RF_RECEIVER_PIN 22 // D2

void setup() {
  Serial.begin(115200);
  gw.setup(RF_RECEIVER_PIN);
  gw.registerPlugin(&DigooStation); 
  gw.registerPlugin(&WeatherStation);
  gw.registerPlugin(&FanjuStation);  
    
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
      new Characteristic::SerialNumber("DG00001"); 
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
      new Characteristic::SerialNumber("DG00002"); 
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
      new Characteristic::SerialNumber("DG00003"); 
      new Characteristic::Model("ch3"); 
      new Characteristic::FirmwareRevision("0.0.2"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new Digoo(&s_ch3);


  ///weather station sensor channel 1
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("WS ch1 sensor"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("WS00001"); 
      new Characteristic::Model("ch1"); 
      new Characteristic::FirmwareRevision("0.0.3"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new Digoo(&w_ch1); 
}

void loop() {
  esp_task_wdt_reset();
  uint64_t p = 0;
  StaticJsonDocument<160> root;
  //gpio_intr_enable(gpio_num_t(RF_RECEIVER_PIN));
  
  if(WeatherStation.available()) {
    if((p = WeatherStation.getPacket())) {
       if(p == prev_p) {
        current_ch = WeatherStation.getChannel(p);

        if (!WeatherStation.isValidWeather(p)) {
          WeatherChannelArray[current_ch - 1] ->batt        = !WeatherStation.getBattery(p);
          WeatherChannelArray[current_ch - 1] ->temperature = WeatherStation.getTemperature(p);
          WeatherChannelArray[current_ch - 1] ->humidity    = (double)WeatherStation.getHumidity(p);
          WeatherChannelArray[current_ch - 1] ->updated     = millis();
          WeatherChannelArray[current_ch - 1] ->isNew[0]    = true;
          WeatherChannelArray[current_ch - 1] ->isNew[1]    = true;
        }
                
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
  
  if(DigooStation.available()) {
    if((p = DigooStation.getPacket())) {
      if(p == prev_p) {
        current_ch = DigooStation.getChannel(p);

        if (!DigooStation.isValidWeather(p)){
          DigooChannelArray[current_ch - 1] ->batt        = !DigooStation.getBattery(p);
          DigooChannelArray[current_ch - 1] ->temperature = DigooStation.getTemperature(p);
          DigooChannelArray[current_ch - 1] ->humidity    = (double)DigooStation.getHumidity(p);
          DigooChannelArray[current_ch - 1] ->updated     = millis();
          DigooChannelArray[current_ch - 1] ->isNew[0]    = true;
          DigooChannelArray[current_ch - 1] ->isNew[1]    = true;
        }
        
                
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
  }

  if(FanjuStation.available()) { 
    if((p = FanjuStation.getPacket())) {
      if(p == prev_p) {
        current_ch = FanjuStation.getChannel(p);
                
        FanjuChannelArray[current_ch - 1] ->batt        = !FanjuStation.getBattery(p);
        FanjuChannelArray[current_ch - 1] ->temperature = FanjuStation.getTemperature(p);
        FanjuChannelArray[current_ch - 1] ->humidity    = (double)FanjuStation.getHumidity(p);
        FanjuChannelArray[current_ch - 1] ->updated     = millis();
        FanjuChannelArray[current_ch - 1] ->isNew[0]    = true;
        FanjuChannelArray[current_ch - 1] ->isNew[1]    = true;
                
        LOG1("Fanju:    ");     LOG1(FanjuStation.getString(p));      LOG1(" ");
        LOG1("ID: ");           LOG1(FanjuStation.getId(p));          LOG1(" ");
        LOG1("Channel: ");      LOG1(FanjuStation.getChannel(p));     LOG1(" ");
        LOG1("Battery: ");      LOG1(FanjuStation.getBattery(p));     LOG1(" ");        
        LOG1("Temperature: ");  LOG1(FanjuStation.getTemperature(p)); LOG1(" ");
        LOG1("Humidity: ");     LOG1(FanjuStation.getHumidity(p));    LOG1("\n");
        
        p = 0;
      }
      prev_p = p;
    }
  }
           
  homeSpan.poll();
}
