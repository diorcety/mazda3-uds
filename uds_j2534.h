/*
  Copyright (C) 2016 Yann Diorcet

  This file is part of IDS.  IDS is free software: you can
  redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation, version 2.
 
  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
 
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

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

typedef std::shared_ptr <UDS_J2534> UDS_J2534Ptr;
typedef std::weak_ptr <UDS_J2534> UDS_J2534WeakPtr;

class UDS_J2534_API UDS_J2534 : public UDS {
public:
    UDS_J2534(J2534ChannelPtr channel, UDS_PID tester, UDS_PID ecu, unsigned long protocolID, unsigned long flags);

    virtual ~UDS_J2534();

    virtual UDSMessagePtr send(const UDSMessagePtr &request, TimeType timeout);

private:
    J2534ChannelPtr mChannel;
    unsigned long mProtocolID;
    unsigned long mFlags;
};

#endif /* _UDS_J2534_H */
