#include "TunnelClientUI.h"
#include <QApplication>

int main(int argc, char *argv[])
{


    QApplication a(argc, argv);
    TunnelClientUI w;
    w.show();
    //talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);

    return a.exec();
}
