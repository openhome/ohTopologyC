#ifndef HEADER_UI_DISPLAY_DRIVER
#define HEADER_UI_DISPLAY_DRIVER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Private/Thread.h>
#include <vector>


namespace OpenHome
{

namespace Ui
{


// Special display driver that sends pixel data out to the network
class NetworkDisplayDriver
{
private:
    static const TUint kMaxNameBytes = 20;
    static const TUint kHeaderBytes = kMaxNameBytes+8; // 4 bytes each for iWidth & iHeight

public:
    NetworkDisplayDriver(Environment& aEnv, const Brx& aName, FrameBuffer& aFrameBuffer);

    void Refresh();

private:
    void Send();

private:
    SocketUdp* iSocket;
    Bws<kMaxNameBytes> iName;
    FrameBuffer& iFrameBuffer;
    TUint32 iWidth;
    TUint32 iHeight;
    TUint iDataBytes;
    Bwh iPixels;
    const TByte* iDataPtr;
};



///////////////////////////////////////////////////////////////////////


} // namespace Ui

} // namespace OpenHome


#endif //HEADER_UI_DISPLAY_DRIVER



























