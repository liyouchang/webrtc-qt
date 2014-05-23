/*
 * Copyright 2014 Kaer Electric Co.,Ltd
 * Listens see <http://www.gnu.org/licenses/>.
 * AUTHORS lht
 *
 * Ke08RecordReader : read h264 data from old record format of kaer in 08 platform
 *
 */


#ifndef KE08RECORDREADER_H
#define KE08RECORDREADER_H

#include "libjingle_app/recordinterface.h"
#include "talk/base/thread.h"
#include "talk/base/buffer.h"


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

#endif // KE08RECORDREADER_H
