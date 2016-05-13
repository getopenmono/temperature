// This software is part of OpenMono, see http://developer.openmono.com
// Released under the MIT license, see LICENSE.txt
#include "app_controller.h"
#include "mono_bitmap.h"
#include "temperature_menlo_bitmap.h"
#include "wifi_logo.h"

using mono::sensor::ITemperature;
using mono::display::Color;
using mono::TouchEvent;
using mono::geo::Point;
using mono::display::IDisplayController;
using mono::IApplicationContext;
using mono::String;

Toucher::Toucher (AppController * ctrl_)
:
	ctrl(ctrl_)
{
}

void Toucher::RespondTouchEnd(TouchEvent &)
{
	ctrl->changeUnit();
}

AppController::AppController ()
:
	toucher(this),
	graphView(40,220-87),
	bg(mono::display::BlackColor),
	timer(1000),
	useCelcius(false)
{
    displayWifiLogo = false;
}

void AppController::monoWakeFromReset ()
{
#ifdef LINEAR_SENSOR
	IApplicationContext::Instance->Temperature = &therm;
#endif
	bg.show();
	timer.setCallback<AppController>(this,&AppController::measureAndUpdate);
	timer.Start();

    uploader.wifiStarted.attach<AppController>(this, &AppController::wifiDidStart);
    Timer::callOnce<InternetUpload>(200, &uploader, &InternetUpload::init);
}

void AppController::monoWakeFromSleep ()
{
	mono::IApplicationContext::SoftwareResetToApplication();
}

void AppController::monoWillGotoSleep ()
{
    //remove power from exspansion connector
    CyPins_ClearPin(EXPANSION_PWR_ENABLE);
}

float AppController::getTemperatureInCelcius ()
{
#if 1
	const float flatten = 0.5;
	static float tempC = 25.0;
	ITemperature * temperature = IApplicationContext::Instance->Temperature;
	tempC = (tempC * flatten) + (temperature->ReadMilliCelcius() / 1000.0 * (1.0 - flatten));
	return tempC;
#else
	static float tempC = 0.0;
	static bool up = true;
	if (up) tempC += 0.5;
	else tempC -= 0.5;
	if (tempC > 50.0) up = false;
	if (tempC < 0.0) up = true;
	return tempC;
#endif
}

void AppController::changeUnit ()
{
	useCelcius = ! useCelcius;
	update();
}

String formatTemperature (int whole, int decimals, char unit)
{
	char string[20];
	if (decimals < 10) sprintf(string,"%i.%i0o%c",whole,decimals,unit);
	else sprintf(string,"%i.%io%c",whole,decimals,unit);
	return string;
}

void AppController::update ()
{
	showLogo();

    if (displayWifiLogo)
    {
        blitImage
        (
         Point(5,8),
         (uint8_t*)mono::display::wifi_logo,
         mono::display::wifi_logo_width,
         mono::display::wifi_logo_height,
         mono::display::CloudsColor
         );
    }

	String s;
	float tempC = getTemperatureInCelcius();
	if (useCelcius)
	{
		int wholeC = tempC;
		int decimalsC = (tempC - wholeC) * 100.0;
		if (decimalsC < 0.0) decimalsC = -decimalsC;
		s = formatTemperature(wholeC,decimalsC,'C');
	}
	else
	{
		float tempF = tempC * 9.0 / 5.0 + 32.0;
		int wholeF = tempF;
		int decimalsF = (tempF - wholeF) * 100.0;
		if (decimalsF < 0.0) decimalsF = -decimalsF;
		s = formatTemperature(wholeF,decimalsF,'F');
	}
	uint8_t x = 176-s.Length()*22;
	drawTemperature(s,x,220-45);
	blitColor(Point(0,220-45),x,45,mono::display::BlackColor);
}

void AppController::measureAndUpdate ()
{
	float tempC = getTemperatureInCelcius();
	graphView.setNextPoint(tempC);
	graphView.show();
	update();
}

uint8_t translateCharToFontIndex (char ch)
{
	// 0123456789.-oCF = 15
	if (ch >= '0' && ch <= '9') return ch-'0';
	else switch (ch)
	{
		case '.': return 10;
		case '-': return 11;
		case 'o': return 12;
		case 'C': return 13;
		case 'F': return 14;
	}
	return 0;
}

void AppController::drawTemperature (String temperature, uint8_t x, uint8_t y)
{
	int const width = 22;
	for (size_t i = 0; i < temperature.Length(); ++i)
	{
		blitChar(translateCharToFontIndex(temperature[i]),x+i*width,y);
	}
}

void AppController::blitChar (int index, uint8_t x, uint8_t y)
{
	// 0123456789.-oCF = 15
	int const width = 22;
	int const height = 42;
	int offset = 1 + index*width;
	if (index >= 2)
	{
		offset -= 1;
	}
	if (index >= 7)
	{
		offset -= 1;
	}
	if (index >= 8)
	{
		offset -= 1;
	}
	if (index >= 10)
	{
		offset -= 1;
	}
	if (index >= 14)
	{
		offset -= 1;
	}
	for (int line = 0; line < height; ++line)
	{
		blitImage
		(
			Point(x,y+line),
			(uint8_t*)mono::display::temp_bitmap + (line*326) + offset,
			width,
			1,
			mono::display::CloudsColor
		);
	}
}

void AppController::showLogo ()
{
	blitImage
	(
		Point((176-mono::display::mono_bitmap_width)/2,0),
		(uint8_t*)mono::display::mono_bitmap,
		mono::display::mono_bitmap_width,
		mono::display::mono_bitmap_height,
		mono::display::TurquoiseColor
	);
}

void AppController::blitImage(Point const &p, uint8_t *data, int w, int h, mono::display::Color color, uint8_t preScale)
{
	IDisplayController *ctrl = IApplicationContext::Instance->DisplayController;
	ctrl->setWindow(p.X(), p.Y(), w, h);
	int len = w*h;
	for (int b=0; b<len; b++) {
		ctrl->write(color.scale(data[b]).scale(preScale));
	}
}

void AppController::blitColor(Point const &p, int w, int h, mono::display::Color color, uint8_t preScale)
{
	IDisplayController *ctrl = IApplicationContext::Instance->DisplayController;
	ctrl->setWindow(p.X(), p.Y(), w, h);
	int len = w*h;
	for (int b=0; b<len; b++) {
		ctrl->write(color.scale(preScale));
	}
}

void AppController::wifiDidStart()
{
    displayWifiLogo = true;
}
