// This software is part of OpenMono, see http://developer.openmono.com
// Released under the MIT license, see LICENSE.txt
#include <mono.h>
#include "app_controller.h"

int main()
{
    AppController app;
    mono::IApplicationContext::Instance->setMonoApplication(&app);
    app.enterRunLoop();
	return 0;
}
