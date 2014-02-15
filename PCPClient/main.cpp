#include "mainwindow.h"
#include <QApplication>
#include "conductor.h"
#include "peer_connection_client.h"
#include "talk/base/win32socketinit.h"
#include "talk/base/win32socketserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    talk_base::EnsureWinsockInit();
    talk_base::Win32Thread w32_thread;
    talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);



    MainWindow w;

    PeerConnectionClient client;
    talk_base::scoped_refptr<Conductor> conductor(
                new talk_base::RefCountedObject<Conductor>(&client, &w));

    w.show();

    return a.exec();
}
