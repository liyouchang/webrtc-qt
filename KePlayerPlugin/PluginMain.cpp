#include "KePlayerPlugin.h"
#include <QApplication>
#include "qtbrowserplugin.h"
//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);
//    KePlayerPlugin w;
//    w.show();

//    return a.exec();
//}
//! [0]

QTNPFACTORY_BEGIN("Qt-based player Plugin", "A Qt-based player plug-in to support kaer camera")
    QTNPCLASS(KePlayerPlugin)
QTNPFACTORY_END()
//! [0]

#ifdef QAXSERVER

//! [1]
#include <QAxFactory>

QAXFACTORY_BEGIN("{5DDA12C6-FDD9-4309-9462-61D2ED98696D}", "{DF5CDC71-2EEE-4559-B0CC-7110FD09C48C}")
    QAXCLASS(KePlayerPlugin)
QAXFACTORY_END()

//! [1]

#endif

