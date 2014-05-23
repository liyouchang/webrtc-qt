#ifndef RECORDREADERINTERFACE_H
#define RECORDREADERINTERFACE_H

#include <string>
#include "talk/base/sigslot.h"


namespace  kaerp2p {

class RecordReaderInterface
{
public:
    virtual bool StartRead(const std::string & filename) = 0;
    virtual bool StopRead() = 0;
    sigslot::signal0<> SignalRecordEnd;
    sigslot::signal2<const char *,int> SignalVideoData;
    sigslot::signal2<const char *,int> SignalAudioData;
protected:
    RecordReaderInterface(){}
};

}
#endif // RECORDREADERINTERFACE_H
