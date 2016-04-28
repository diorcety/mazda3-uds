#pragma once

#ifndef _UDS_J2534_H
#define _UDS_J2534_H

#include <memory>

/*
 * DLL
 */
#ifdef _WIN32
#ifndef SWIG
#ifdef UDS_J2534_EXPORTS
#define UDS_J2534_API __declspec(dllexport)
#else
#define UDS_J2534_API __declspec(dllimport)
#endif // UDS_EXPORTS
#else // SWIG
#define UDS_J2534_API
#endif // SWIG
#else // _WIN32
#define UDS_J2534_API
#endif // _WIN32

#include "uds.h"
#include <j2534.h>

class UDS_J2534;
typedef std::shared_ptr<UDS_J2534> UDS_J2534Ptr;
typedef std::weak_ptr<UDS_J2534> UDS_J2534WeakPtr;

class UDS_J2534_API UDS_J2534: public UDS {
public:
    UDS_J2534(J2534ChannelPtr channel, UDS_PID tester, UDS_PID ecu, unsigned long protocolID, unsigned long flags);
    virtual ~UDS_J2534();


    virtual UDSMessagePtr send(const UDSMessagePtr request, TimeType timeout);

private:
    J2534ChannelPtr mChannel;
    unsigned long mProtocolID;
    unsigned long mFlags;
};

#endif /* _UDS_J2534_H */
