#include "jsonconfig.h"

#include <fstream>
#include "talk/base/stream.h"
#include "talk/base/scoped_ptr.h"



JsonConfig::JsonConfig():
    value_(Json::objectValue)
{
}

bool JsonConfig::FromString(const std::string &in)
{
    Json::Reader reader;
    if(!reader.parse(in, value_)) {
        return false;
    }

    return true;
}

std::string JsonConfig::ToString()
{
    Json::StyledWriter writer;
    return writer.write(value_);
}

bool JsonConfig::FromFile(const std::string &filename)
{


    if(filename.empty())
        return false;

//    talk_base::scoped_ptr<talk_base::FileStream> stream;
//    stream.reset(new talk_base::FileStream());
//    if(!stream->Open(filename,"r",NULL)){
//        return false;
//    }
//    size_t filesize ;
//    stream->GetSize(&filesize);

//    char* buffer = new char[filesize+1];
//    stream->Read(buffer,filesize,NULL,NULL);
//    stream->Close();
    // 获取文件大小
    std::ifstream f(filename.c_str());
    f.seekg(std::ios::beg, std::ios::end);
    std::streamsize file_size = f.tellg();
    if(file_size <= 0)
        return false;

    // 从文件读取数据
    f.seekg(0, std::ios::beg);
    char* buffer = new char[file_size+1];
    f.read(buffer, file_size);
    f.close();

    // 初始化
    bool b = FromString(buffer);
    delete buffer;
    buffer = NULL;

    return b;

}

bool JsonConfig::ToFile(const std::string &sSaveFile)
{
    if(sSaveFile.empty())
        return false;

    std::string sOut = ToString();

    std::ofstream f(sSaveFile.c_str());
    if(!f.good()) {
        return false;
    }

    f.write(sOut.c_str(), sOut.size());
    f.close();

    return true;
}

JsonConfig *JsonConfig::Instance()
{
    static JsonConfig config;
    return &config;
}
