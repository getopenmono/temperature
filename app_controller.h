// This software is part of OpenMono, see http://developer.openmono.com
// Released under the MIT license, see LICENSE.txt
#if !defined(__temperature_appcontroller_h)
#define __temperature_appcontroller_h
#include <mono.h>
#include "graph_view.h"
#include "internet_upload.h"

class AppController;

class Toucher : public mono::TouchResponder {
public:
	void RespondTouchEnd(mono::TouchEvent &event);
	Toucher(AppController *);
private:
	AppController * ctrl;
};

class AppController
:
	public mono::IApplication
{
public:
	static float getTemperatureInCelcius ();
	AppController ();
	void monoWakeFromReset ();
	void monoWakeFromSleep ();
	void monoWillGotoSleep ();
	void changeUnit ();

private:
	void blitChar(int index, uint8_t x, uint8_t y);
	void blitImage(mono::geo::Point const &p, uint8_t *data, int w, int h, mono::display::Color color, uint8_t preScale = 0xFF);
	void blitColor(mono::geo::Point const &p, int w, int h, mono::display::Color color, uint8_t preScale = 0xFF);
	void drawTemperature (mono::String temperature, uint8_t x, uint8_t y);
	void update ();
	void measureAndUpdate ();
	void showLogo ();
    void wifiDidStart();

	mono::Timer sleeper;
	Toucher toucher;
	GraphView graphView;
	mono::ui::BackgroundView bg;
    bool displayWifiLogo;
	mono::Timer timer;
	bool useCelcius;
    InternetUpload uploader;
};

#endif // __temperature_appcontroller_h
