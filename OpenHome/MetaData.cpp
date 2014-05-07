#include <OpenHome/MetaData.h>


using namespace OpenHome;
using namespace OpenHome::Av;



///////////////////////////////////////////////////////////////

IInfoMetadata* InfoMetadata::iEmpty = new InfoMetadata();

IInfoMetadata* InfoMetadata::Empty()
{
    return(iEmpty);
}


InfoMetadata::InfoMetadata()
    :iMetadata(NULL)
{
}


InfoMetadata::InfoMetadata(IMediaMetadata* aMetadata, const Brx& aUri)
    :iMetadata(aMetadata)
    ,iUri(aUri)
{
}


InfoMetadata::~InfoMetadata()
{
    if (iMetadata!=NULL)
    {
        delete iMetadata;
    }
}


IMediaMetadata& InfoMetadata::Metadata()
{
    return (*iMetadata);
}


const Brx& InfoMetadata::Uri()
{
    return (iUri);
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

