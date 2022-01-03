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

libesp::ErrorType MyApp::onInit() {
	ErrorType et;
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());

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
}

static bool Down = true;

ErrorType MyApp::onRun() {
#if 0
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
      }
      break;
    }
  }
	return et;
#endif
}


