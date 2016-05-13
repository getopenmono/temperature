

#include "internet_upload.h"
#include <io/file.h>
#include <wireless/redpine_module.h>
#include <application_context_interface.h>

using namespace mono;
using namespace mono::io;
using namespace mono::network;

const char *InternetUpload::ssidFilename = "/sd/ssid.txt";
const char *InternetUpload::passFilename = "/sd/pass.txt";
const char *InternetUpload::urlFileName = "/sd/url.txt";

InternetUpload::InternetUpload() :
    sdfs(SD_SPI_MOSI, SD_SPI_MISO, SD_SPI_CLK, SD_SPI_CS, "sd"),
    rpSpi(RP_SPI_MOSI, RP_SPI_MISO, RP_SPI_CLK),
    spiComm(rpSpi, RP_SPI_CS, RP_nRESET, RP_INTERRUPT)
{
    fsExist = false;
    lastSendTime = 0;
}

void InternetUpload::init()
{
    //try FS
    FILE *ssidFile = 0;
    for (int t=0; t<10; t++) {
        ssidFile = fopen(ssidFilename, "r");
        if (ssidFile != 0)
            break;
    }

    if (ssidFile == 0)
    {
        debug("No SD card or ssid.txt file found!\r\n");
        return;
    }

    fclose(ssidFile);
    ssid = File::readFirstLine(ssidFilename);
    password = File::readFirstLine(passFilename);
    url = File::readFirstLine(urlFileName);
    connectWifi();
}

void InternetUpload::connectWifi()
{
    redpine::Module::initialize(&spiComm);

    redpine::Module::setNetworkReadyCallback<InternetUpload>(this, &InternetUpload::wifiConnected);

    redpine::Module::setupWifiOnly(ssid, password);
}


/// MARK: Protected Methods

void InternetUpload::wifiConnected()
{
    if (!redpine::Module::IsNetworkReady())
        return;

    beginDownload();
    wifiStarted.call();
}

void InternetUpload::beginDownload()
{
    if (!redpine::Module::IsNetworkReady())
        return;
    int mcelcius = IApplicationContext::Instance->Temperature->ReadMilliCelcius();
    String s = String::Format("%s?celcius=%i",url(),mcelcius);
    client = HttpClient(s);
    client.setCompletionCallback<InternetUpload>(this, &InternetUpload::httpCompletion);
    lastSendTime = us_ticker_read();
}

void InternetUpload::httpCompletion(network::INetworkRequest::CompletionEvent *evnt)
{

    if (!evnt->Context->HasFailed())
    {
        if (us_ticker_read() - lastSendTime >= 1000000)
        {
            beginDownload();
        }
        else
        {
            int delay = lastSendTime+1000000 - us_ticker_read();
            Timer::callOnce<InternetUpload>(delay>0?delay/1000:0, this, &InternetUpload::beginDownload);
        }

    }
}

void InternetUpload::httpData(network::HttpClient::HttpResponseData const &data)
{
    printf("%s",data.bodyChunk());
}
