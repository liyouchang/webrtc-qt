#include <iostream>


#include "zmq.hpp"
#include "zhelpers.hpp"
#include "asyndealer.h"
#include "talk/base/thread.h"
using namespace std;

int main()
{
    AsynDealer *dealer = new AsynDealer();
    dealer->initialize("12345","tcp://192.168.0.182:5555");
    //talk_base::Thread::SleepMs(100);
    for(int i =0 ;i<1;i++)
        dealer->send("12345","testdata");
    std::cout<<"run!"<<std::endl;
    talk_base::Thread::Current()->Run();
    delete dealer;
    std::cout<<"end!"<<std::endl;

    return 0;
}

