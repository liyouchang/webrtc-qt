#ifndef OBJECTSAFETYIMPL_H
#define OBJECTSAFETYIMPL_H

#include <QAxAggregated>

#include <objsafe.h>
#include <QUuid>

class ObjectSafetyImpl : public QAxAggregated,
                         public IObjectSafety
{
public:
    ObjectSafetyImpl() {}

    long queryInterface( const QUuid &iid, void **iface )
    {
        *iface = 0;
        if ( iid == IID_IObjectSafety )
            *iface = (IObjectSafety*)this;
        else
            return E_NOINTERFACE;

        AddRef();
        return S_OK;
    }

    QAXAGG_IUNKNOWN;

    HRESULT WINAPI GetInterfaceSafetyOptions( REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions )
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        return S_OK;
    }
    HRESULT WINAPI SetInterfaceSafetyOptions( REFIID riid, DWORD pdwSupportedOptions, DWORD pdwEnabledOptions )
    {
        return S_OK;
    }
};

#endif // OBJECTSAFETYIMPL_H
