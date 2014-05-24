#ifndef KE08RECORDER_H
#define KE08RECORDER_H

#include "libjingle_app/recordinterface.h"
#include "talk/base/thread.h"
#include "talk/base/buffer.h"

namespace talk_base{
class FileStream;
}

namespace  kaerp2p {

class Ke08RecordReader :
        public kaerp2p::RecordReaderInterface,
        public talk_base::MessageHandler
{
public:
    Ke08RecordReader();
    virtual ~Ke08RecordReader();
    void OnMessage(talk_base::Message *msg);
    bool StartRead(const std::string &filename);
    bool StopRead();
private:
    void SendMediaMsg(const char *data, int len);
    talk_base::Thread * media_thread_;
    talk_base::Buffer video_data_;
};

class Ke08RecordSaver : public RecordSaverInterface
{
public:
    Ke08RecordSaver();
    virtual ~Ke08RecordSaver();
    bool StartSave(const std::string &fileName);
    bool StopSave();
    void OnVideoData(const std::string &peerId, const char *data, int len);
    void OnAudioData(const std::string &peerId, const char *data, int len);
private:
    talk_base::FileStream * saveFile;
};

}
#endif // KE08RECORDER_H
