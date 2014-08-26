#include <iostream>
#include <string>
//#include "zmq.hpp"
//#include "zhelpers.hpp"

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
const int times = 10;

int main()
{

//    std::string patten = "-\d{1,2}:\d{2}";
//    std::string patten = "w+";
//    regex_t reg;
//    int nErrCode = 0;
//    std::string text = "www";
//    if((nErrCode = regcomp(&reg,patten.c_str(),0)) == 0){
//        if ((nErrCode = regexec(&reg, text.c_str(), 0, NULL, 0)) ==REG_NOERROR )
//        {
//            printf("%s matches %s\n", text.c_str(), patten.c_str());
//        }else{
//            std::cout<<"not matche "<<nErrCode<<std::endl;

//        }

//    }
//    std::cout<<"end"<<std::endl;
//    regfree(&reg);

    std::string zone = "-:";

    int zonesecond = 0*60*60;
    if(zone.size() > 2 ){
        int pos = zone.find(':');
        if(pos != std::string::npos){
            int hour = atoi(zone.substr(1,pos).c_str());
            int minute = atoi(zone.substr(pos+1).c_str());
            if(zone[0] == '+'){
                zonesecond = hour*60*60 + minute*60;
            }else if(zone[0] == '-'){
                zonesecond = -(hour*60*60 + minute*60);
            }else{
//                LOG_F(WARNING)<<" zone format error";
            }
        }else{
//            LOG_F(WARNING)<<" zone format error";
        }
    }
    std::cout<<zonesecond<<std::endl;
    return 0 ;

}

