#ifndef RECORDREADERINTERFACE_H
#define RECORDREADERINTERFACE_H

#include <string>
#include "talk/base/sigslot.h"
#include "KeMsgProcess.h"
#include "defaults.h"

namespace  kaerp2p {

class RecordReaderInterface
{
public:
    virtual bool StartRead(const std::string & filename) = 0;
    virtual bool StopRead() = 0;
    sigslot::signal1<RecordReaderInterface *> SignalRecordEnd;
    sigslot::signal2<const char *,int> SignalVideoData;
    sigslot::signal2<const char *,int> SignalAudioData;
    VideoInfo recordInfo;
protected:
    RecordReaderInterface(){}
};


class RecordSaverInterface: public sigslot::has_slots<>
{
public:
   virtual bool StartSave(const std::string & fileName) = 0;
   virtual bool StopSave() = 0;
   virtual void OnVideoData(const std::string & peerId,const char *data,int len) =0;
   virtual void OnAudioData(const std::string & peerId,const char *data,int len) =0;

protected:
    RecordSaverInterface(){}

};

}
#endif // RECORDREADERINTERFACE_H
