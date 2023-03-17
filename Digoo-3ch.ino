//#include <ArduinoJson.h>

#include "HomeSpan.h" // homespan-ota
#include "DigooTH.h"

#include "homeGW.h"
#include "digoo.h"
#include "weather.h"
#include "fanju.h"

#include <esp_task_wdt.h>

#define WDT_TIMEOUT 100 

HomeGW gw(3); //  is the number of plugins to be registered 
digoo   DigooStation;
weather WeatherStation;
fanju   FanjuStation;

DigooData s_ch1, s_ch2, s_ch3;
DigooData w_ch1, w_ch2, w_ch3;
DigooData f_ch1, f_ch2, f_ch3;

DigooData *DigooChannelArray[3]   = {&s_ch1, &s_ch2, &s_ch3};
DigooData *WeatherChannelArray[3] = {&w_ch1, &w_ch2, &w_ch3};
DigooData *FanjuChannelArray[3]   = {&f_ch1, &f_ch2, &f_ch3};

Plugin    *WeatherPlugin[3]    = {&DigooStation, &WeatherStation, &FanjuStation};
String    PluginName[3]        = {"Digoo:    ", "Weather:   ", "Fanju:    "};
DigooData **AllPluginArrays[3] = {DigooChannelArray, WeatherChannelArray, FanjuChannelArray};

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

  homeSpan.setSketchVersion("0.0.4");
  homeSpan.enableOTA(); //homespan-ota
  homeSpan.enableWebLog(50,"pool.ntp.org","UTC-3:00","digoo"); 
  
  homeSpan.begin(Category::Sensors,"Digoo");
  ///channel 1
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Digoo ch1 sensor"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("DG00001"); 
      new Characteristic::Model("ch1"); 
      new Characteristic::FirmwareRevision("0.0.4"); 
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
      new Characteristic::FirmwareRevision("0.0.4"); 
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
      new Characteristic::FirmwareRevision("0.0.4"); 
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
      new Characteristic::FirmwareRevision("0.0.4"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new Digoo(&w_ch1); 
    WEBLOG("Homekit enabled");
}

void loop() {
  esp_task_wdt_reset();
  uint64_t p = 0;
  homeSpan.poll();

  for (uint8_t plugin = 0; plugin < 3; plugin++){
    if(WeatherPlugin[plugin]->available()){
      if((p = WeatherPlugin[plugin]->getPacket())){
        if(p == prev_p){
          current_ch = (WeatherPlugin[plugin]->getChannel(p))-1;
          String sens = PluginName[plugin];
                      
          DigooData *tData = AllPluginArrays[plugin][current_ch];
                             
          if (!WeatherPlugin[plugin]->isValidWeather(p)) {
            
            tData ->batt        = !WeatherPlugin[plugin]->getBattery(p); LOG1("batt ok\n "); 
            tData ->temperature = WeatherPlugin[plugin]->getTemperature(p); LOG1("temp ok\n "); 
            tData ->humidity    = (double)WeatherPlugin[plugin]->getHumidity(p); LOG1("hum ok\n "); 
            tData ->updated     = millis(); LOG1("millis ok\n "); 
            tData ->isNew[0]    = true;
            tData ->isNew[1]    = true;
          }

          //LOG1(sens);               LOG1(WeatherPlugin[plugin]->getString(p));      LOG1(" ");
          //LOG1("ID: ");             LOG1(WeatherPlugin[plugin]->getId(p));          LOG1(" ");
          //LOG1("Channel: ");        LOG1(WeatherPlugin[plugin]->getChannel(p));     LOG1(" ");
          //LOG1("Battery: ");        LOG1(WeatherPlugin[plugin]->getBattery(p));     LOG1(" ");        
          //LOG1("Temperature: ");    LOG1(WeatherPlugin[plugin]->getTemperature(p)); LOG1(" ");
          //LOG1("Humidity: ");       LOG1(WeatherPlugin[plugin]->getHumidity(p));    LOG1("\n");
                
          WEBLOG("%s %s; ID: %d; Channel: %d; Battery: %s; Temperature: %.2f; Humidity: %d",
                sens,
                WeatherPlugin[plugin]->getString(p), 
                WeatherPlugin[plugin]->getId(p), 
                WeatherPlugin[plugin]->getChannel(p),
                WeatherPlugin[plugin]->getBattery(p)?"FULL":"LOW",
                WeatherPlugin[plugin]->getTemperature(p),
                WeatherPlugin[plugin]->getHumidity(p)); 

          p = 0;
        }
        prev_p = p;
      }
    }
  }

/*
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
                
        //LOG1("Weather:    ");     LOG1(WeatherStation.getString(p));      LOG1(" ");
        //LOG1("ID: ");             LOG1(WeatherStation.getId(p));          LOG1(" ");
        //LOG1("Channel: ");        LOG1(WeatherStation.getChannel(p));     LOG1(" ");
        //LOG1("Battery: ");        LOG1(WeatherStation.getBattery(p));     LOG1(" ");        
        //LOG1("Temperature: ");    LOG1(WeatherStation.getTemperature(p)); LOG1(" ");
        //LOG1("Humidity: ");       LOG1(WeatherStation.getHumidity(p));    LOG1("\n");

        WEBLOG("Weather: %s; ID: %d; Channel: %d; Battery: %s; Temperature: %.2f; Humidity: %d",
                WeatherStation.getString(p), 
                WeatherStation.getId(p), 
                WeatherStation.getChannel(p),
                WeatherStation.getBattery(p)?"FULL":"LOW",
                WeatherStation.getTemperature(p),
                WeatherStation.getHumidity(p));
        
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
        
                
        //LOG1("Digoo:    ");     LOG1(DigooStation.getString(p));      LOG1(" ");
        //LOG1("ID: ");           LOG1(DigooStation.getId(p));          LOG1(" ");
        //LOG1("Channel: ");      LOG1(DigooStation.getChannel(p));     LOG1(" ");
        //LOG1("Battery: ");      LOG1(DigooStation.getBattery(p));     LOG1(" ");        
        //LOG1("Temperature: ");  LOG1(DigooStation.getTemperature(p)); LOG1(" ");
        //LOG1("Humidity: ");     LOG1(DigooStation.getHumidity(p));    LOG1("\n");

        WEBLOG("Digoo: %s; ID: %d; Channel: %d; Battery: %s; Temperature: %.2f; Humidity: %d",
                DigooStation.getString(p), 
                DigooStation.getId(p), 
                DigooStation.getChannel(p),
                DigooStation.getBattery(p)?"FULL":"LOW",
                DigooStation.getTemperature(p),
                DigooStation.getHumidity(p));
        
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
                
        //LOG1("Fanju:    ");     LOG1(FanjuStation.getString(p));      LOG1(" ");
        //LOG1("ID: ");           LOG1(FanjuStation.getId(p));          LOG1(" ");
        //LOG1("Channel: ");      LOG1(FanjuStation.getChannel(p));     LOG1(" ");
        //LOG1("Battery: ");      LOG1(FanjuStation.getBattery(p));     LOG1(" ");        
        //LOG1("Temperature: ");  LOG1(FanjuStation.getTemperature(p)); LOG1(" ");
        //LOG1("Humidity: ");     LOG1(FanjuStation.getHumidity(p));    LOG1("\n");

        WEBLOG("Fanju: %s; ID: %d; Channel: %d; Battery: %s; Temperature: %.2f; Humidity: %d",
                FanjuStation.getString(p), 
                FanjuStation.getId(p), 
                FanjuStation.getChannel(p),
                FanjuStation.getBattery(p)?"FULL":"LOW",
                FanjuStation.getTemperature(p),
                FanjuStation.getHumidity(p));
        
        p = 0;
      }
      prev_p = p;
    }
  }
 */         
}
