/*
 * app.cpp
 *
 * Author: cmdc0de
 */

#include "app.h"
#include <esp_log.h>
#include <system.h>
#include <spibus.h>

#include <driver/uart.h>
#include <driver/gpio.h>
#include "device/leds/noclockprgled.h"
#include "spibus.h"
#include "freertos.h"
#include "fatfsvfs.h"
#include "pinconfig.h"
#include "etl/vector.h"
#include "net/wifi.h"
#include "net/wifieventhandler.h"

using libesp::ErrorType;
using libesp::System;
using libesp::FreeRTOS;
using libesp::RGB;
using libesp::NoClkLedStrip;
using libesp::SPIBus;
using libesp::APA106;

const char *MyApp::LOGTAG = "AppTask";
const char *MyApp::sYES = "Yes";
const char *MyApp::sNO = "No";


const char *MyErrorMap::toString(int32_t err) {
	return "TODO";
}

MyApp MyApp::mSelf;

MyApp &MyApp::get() {
	return mSelf;
}

MyApp::MyApp() : AppErrors(), CurrentMode(ONE), LastTime(0) {
	ErrorType::setAppDetail(&AppErrors);
}

MyApp::~MyApp() {

}

NoClkLedStrip LedControl = NoClkLedStrip::create(APA106::get(),127,9);
libesp::WiFiEventHandler MyHandler;

#define TEST_CODE
libesp::ErrorType MyApp::onInit() {
	ErrorType et;
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
#ifdef TEST_CODE
  //esp_netif_init();
  //esp_event_loop_create_default();
  //esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  //assert(sta_netif);

  //wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  //esp_wifi_init(&cfg);
  //ESP_LOGI(LOGTAG,"after wifi config");

  libesp::WiFi wifi;
  wifi.setWifiEventHandler(&MyHandler);
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
  et = wifi.init(WIFI_MODE_STA);
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
  if(et.ok()) {
    etl::vector<libesp::WiFiAPRecord,16> results;
    wifi_scan_config_t conf;
    memset(&conf,0,sizeof(conf));
    conf.show_hidden = true;
    conf.scan_type = WIFI_SCAN_TYPE_PASSIVE; //WIFI_SCAN_TYPE_ACTIVE
	  ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
    et = wifi.scan(results, conf);
	  ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
    etl::vector<libesp::WiFiAPRecord,16>::iterator it;
    for(it=results.begin();it!=results.end();++it) {
      ESP_LOGI(LOGTAG,"%s", (*it).toString().c_str());
    }
  }
  const char * ssid = "dac-nh";
  const char * pass = "XXXXXX";
  et = wifi.connect(ssid,pass,WIFI_AUTH_OPEN);
  if(!et.ok()) {
    ESP_LOGI(LOGTAG, "connect error: %d %s", et.getErrT(), et.toString());
  } else {
    ESP_LOGI(LOGTAG, "connect call successful");
  }
  
  return et;

#else

	et = LedControl.init(GPIO_NUM_19,static_cast<rmt_channel_t>(0));
	if(!et.ok()) {
		return et;
	} else {
		ESP_LOGI(LOGTAG,"LedControl inited");
	}

	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());

	LedControl.fillColor(RGB::BLUE);

	LedControl.send();
	ESP_LOGI(LOGTAG,"OnInit:After Send: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
    LastTime = FreeRTOS::getTimeSinceStart();

	return et;
#endif
}

static uint32_t count = 1;
ErrorType MyApp::onRun() {
#ifdef TEST_CODE
  return ErrorType();
#else
  ErrorType et;
  uint32_t timeSinceLast = FreeRTOS::getTimeSinceStart()-LastTime;
  if(timeSinceLast>=TIME_BETWEEN_PULSES) {
    LastTime = FreeRTOS::getTimeSinceStart();
    switch(CurrentMode) {
    case ONE:
      {
        LedControl.fillColor(RGB::GREEN);
        LedControl.send();
        CurrentMode = TWO;
      }
      break;
    case TWO:
      {
        LedControl.fillColor(RGB::RED);
        LedControl.send();
        CurrentMode = THREE;
      }
      break;
    case THREE:
      {
        LedControl.fillColor(RGB::WHITE);
        LedControl.send();
        CurrentMode = FOUR;
      }
      break;
    case FOUR:
      {
	uint8_t b = LedControl.getBrightness();
	ESP_LOGI(LOGTAG,"brightness: %u", static_cast<uint32_t>(b));
	if(b<40) {
		LedControl.setBrightness(255);
		CurrentMode = FIVE;
	} else {
		b = b-40;
		LedControl.setBrightness(b);
	}
        LedControl.send();
      }
      break;
    case FIVE:
      {
	RGB black(0,0,0);
	LedControl.fillColor(black);
	for(uint32_t i=0;i<count;++i) {
		LedControl.setColor(i,RGB::BLUE);
	}
	if(count>8) {
	   count=1;
	   CurrentMode = ONE;
	} else {
	   ++count;
	}
        LedControl.send();

      }
      break;
    }
  }
	return et;
#endif
}


