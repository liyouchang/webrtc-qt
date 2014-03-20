#include "KeVideoSimulator.h"
#include "talk/base/pathutils.h"
KeVideoSimulator::KeVideoSimulator()
{
    file_thread_ = new talk_base::Thread();
    file_thread_->Start();


}

KeVideoSimulator::~KeVideoSimulator()
{
    delete file_thread_;
}

bool KeVideoSimulator::ReadVideoData(std::string file_name)
{
    talk_base::Pathname path;
    bool result = talk_base::Filesystem::GetAppPathname(&path);
    if(!result){
        return false;
    }
    path.AppendPathname(file_name);

    talk_base::FileStream * stream = talk_base::Filesystem::OpenFile(path,"r");
    size_t fileSize;
    result = stream->GetSize(fileSize);
    if(!result){
        return false;
    }

    video_data_.SetLength(fileSize);
    size_t readData;
    int error;
    talk_base::StreamResult sr = stream->Read(video_data_.data(),fileSize,&readData,&error);
    if(sr != talk_base::SR_SUCCESS){
        return false;
    }

    return true;
}
