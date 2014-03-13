//#include <QCoreApplication>

#include "p2pthread.h"
#include <iostream>


// Split the message into two parts by the first delimiter.
static bool SplitByDelimiter(const std::string& message,
                             const char delimiter,
                             std::string* field1,
                             std::string* field2) {
    // Find the first delimiter
    size_t pos = message.find(delimiter);
    if (pos == std::string::npos) {
        return false;
    }
    *field1 = message.substr(0, pos);
    // The rest is the value.
    *field2 = message.substr(pos + 1);
    return true;
}

int main(int argc,char *argv[])
{

    //QCoreApplication a(argc,argv);
//    talk_base::Win32Thread w32_thread;
//    talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

    talk_base::LogMessage::ConfigureLogging("tstamp thread verbose debug",NULL);

    kaerp2p::P2PThread p2p;
    p2p.Start();
    while(true){
        std::string msg;
        std::cout<<"input command"<<std::endl;
        std::cin>>msg;

        std::string cmd;
        std::string data;
        if(!SplitByDelimiter(msg,':',&cmd,&data)){
            std::cout<<msg<<std::endl;
        }
        else{
            if(cmd.compare("stop")==0){
                p2p.Post(NULL,kaerp2p::MSG_DONE);
            }else if(cmd.compare("connect")==0){
                int peer_id = atoi(data.c_str());
                talk_base::TypedMessageData<int> *msgData =
                        new  talk_base::TypedMessageData<int>(peer_id);
                p2p.Post(&p2p,kaerp2p::MSG_CONNECT_TO_PEER,msgData);
            }else if(cmd.compare("send")==0){
                talk_base::TypedMessageData<std::string> *msgData =
                        new  talk_base::TypedMessageData<std::string>(data);
                p2p.Post(&p2p,kaerp2p::MSG_SEND_STRING,msgData);
            }else if(cmd.compare("sendfile")==0){
                std::string filename = data;
                talk_base::FileStream file;
                int error;
                if (!file.Open(filename.c_str(), "rb", &error)) {
                    std::cerr << "Error opening <" << filename << ">: "
                              << std::strerror(error) << std::endl;
                    continue;
                }
                bool fileReadEnd = false;
                LOG(INFO)<<"read file start";
                while(!fileReadEnd){
                    char buffer[2048];
                    size_t buffer_len = 0;
                    size_t count = 0;
                    while (buffer_len < sizeof(buffer)) {
                        talk_base::StreamResult result = file.Read(buffer + buffer_len,
                                           sizeof(buffer) - buffer_len,
                                           &count, &error);
                        if (result == talk_base::SR_SUCCESS) {
                            buffer_len += count;
                            continue;
                        }
                        if (result == talk_base::SR_EOS) {
                            fileReadEnd = true;
                            LOG(INFO) << "End of file";
                            break;
                        } else if (result == talk_base::SR_BLOCK) {
                            LOG(WARNING)<< "File blocked unexpectedly on read" ;
                        } else {
                            LOG(WARNING)<< "File read error: " << error ;
                        }
                        break;
                    }
                    talk_base::Buffer sendBuffer(buffer,buffer_len);
                    talk_base::TypedMessageData<talk_base::Buffer> *msgData =
                            new  talk_base::TypedMessageData<talk_base::Buffer>(sendBuffer);
                    p2p.Post(&p2p,kaerp2p::MSG_WRITE_BUFFER,msgData);
                }
                LOG(INFO)<<"read file end";
            }
            else if(cmd.compare("sendtopeer")==0){
                talk_base::TypedMessageData<std::string> *msgData =
                        new  talk_base::TypedMessageData<std::string>(data);
                p2p.Post(&p2p,kaerp2p::MSG_SEND_TO_PEER,msgData);

            }else if(cmd.compare("sendstream")==0){

                std::string strPackSize = data;
                int buffer_len = atoi(strPackSize.c_str());
                if(buffer_len == 0){
                    buffer_len= 1024;
                }
                std::cout<<"send "<<buffer_len<<" bytes per 40ms"<<std::endl;
                talk_base::TypedMessageData<int> *msgData =
                        new  talk_base::TypedMessageData<int>(buffer_len);
                p2p.Post(&p2p,kaerp2p::MSG_SEND_STREAM,msgData);

//                char * buffer = new char[buffer_len];
//                for(int i=0;i<buffer_len;i++){
//                    buffer[i] = static_cast<char>(rand());
//                }
//                while(true){
//                    talk_base::Buffer sendBuffer(buffer,buffer_len);
//                    talk_base::TypedMessageData<talk_base::Buffer> *msgData =
//                            new  talk_base::TypedMessageData<talk_base::Buffer>(sendBuffer);
//                    p2p.Post(&p2p,kaerp2p::MSG_WRITE_BUFFER,msgData);

//                    talk_base::Thread::SleepMs(40);
//                }


            }else{

                std::cout << "not support cmd";
            }


        }

        talk_base::Thread::SleepMs(10);
    }
    //    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);
    //    PeerConnectionClient client;
    //    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor(
    //                new talk_base::RefCountedObject<kaerp2p::ServerConductor>(&client));

    //    std::string serverIp = "192.168.40.195";
    //    conductor->StartLogin(serverIp,8888);
    //    std::cout<<"connect end"<<std::endl;

    //    std::vector<uint32> ids;
    //    ids.push_back(kaerp2p::MSG_DONE);
    //    ids.push_back(kaerp2p::MSG_LOGIN_FAILED);
    //    Loop(ids);
    //   return a.exec();
    return 0;
}

