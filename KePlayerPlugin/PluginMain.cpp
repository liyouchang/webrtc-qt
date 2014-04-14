#include "KePlayerPlugin.h"
#include <QApplication>
#include "qtbrowserplugin.h"


QTNPFACTORY_BEGIN("Qt-based player Plugin", "A Qt-based player plug-in to support kaer camera")
    QTNPCLASS(KePlayerPlugin)
QTNPFACTORY_END()

#ifdef QAXSERVER

#include <QAxFactory>

QAXFACTORY_BEGIN("{5DDA12C6-FDD9-4309-9462-61D2ED98696D}", "{DF5CDC71-2EEE-4559-B0CC-7110FD09C48C}")
    QAXCLASS(KePlayerPlugin)
QAXFACTORY_END()


#endif

