#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Ui/Display.h>
#include <OpenHome/Ui/DisplayDriver.h>
#include <OpenHome/Private/Debug.h>


using namespace OpenHome;
using namespace OpenHome::Ui;



NetworkDisplayDriver::NetworkDisplayDriver(Environment& aEnv, const Brx& aName, FrameBuffer& aFrameBuffer)
    :iSocket(new SocketUdp(aEnv))
    ,iName(aName)
    ,iFrameBuffer(aFrameBuffer)
    ,iWidth(aFrameBuffer.Width())
    ,iHeight(aFrameBuffer.Height())
    ,iDataBytes(iFrameBuffer.PixelBytes())
    ,iPixels(kHeaderBytes+iDataBytes)
    ,iDataPtr(iPixels.Ptr()+kHeaderBytes)
{
    iPixels.SetBytes(iPixels.MaxBytes());
    iFrameBuffer.AddReaderCallback(MakeFunctor(*this, &NetworkDisplayDriver::Refresh));
}


void NetworkDisplayDriver::Refresh()
{
    OpenHome::Log::Print(">NetworkDisplayDriver::Refresh() \n");
    Bwn buf(iDataPtr, iDataBytes);
    iFrameBuffer.Read(buf);
    ASSERT(buf.Bytes()==iDataBytes);
    Send();
    OpenHome::Log::Print("<NetworkDisplayDriver::Refresh() \n");
}


void NetworkDisplayDriver::Send()
{
    OpenHome::Log::Print(">NetworkDisplayDriver::Send()  iPixels.Bytes() = %d\n", iPixels.Bytes());
    Endpoint ep(239, Brn("239.239.239.239"));
    iSocket->Send(iPixels, ep);
}
