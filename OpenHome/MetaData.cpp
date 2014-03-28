#include <OpenHome/MetaData.h>


using namespace OpenHome;
using namespace OpenHome::Av;



///////////////////////////////////////////////////////////////


IInfoMetadata* InfoMetadata::Empty()
{
    return(new InfoMetadata());
}


InfoMetadata::InfoMetadata()
    :iMetadata(NULL)
    ,iUri(Brx::Empty())
{
//    iMetadata = null;
//    iUri = null;
}


InfoMetadata::InfoMetadata(IMediaMetadata& aMetadata, const Brx& aUri)
    :iMetadata(&aMetadata)
    ,iUri(aUri)
{
}


IMediaMetadata& InfoMetadata::Metadata()
{
    return (*iMetadata);
}


Brn InfoMetadata::Uri()
{
    return iUri;
}


///////////////////////////////////////////////////////////////

InfoMetatext::InfoMetatext()
    :iMetatext(NULL)
{
//    iMetatext = null;
}


InfoMetatext::InfoMetatext(IMediaMetadata& aMetatext)
    :iMetatext(&aMetatext)
{
}


IMediaMetadata& InfoMetatext::Metatext()
{
    return(*iMetatext);
}

