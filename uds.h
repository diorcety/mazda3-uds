#pragma once

#ifndef _UDS_H
#define _UDS_H

#include "iso14229.h"
#include <memory>
#include <vector>
#include <exception>

/*
 * DLL
 */
#ifdef _WIN32
#ifndef SWIG
#ifdef UDS_EXPORTS
#define UDS_API __declspec(dllexport)
#else
#define UDS_API __declspec(dllimport)
#endif // UDS_EXPORTS
#else // SWIG
#define UDS_API
#endif // SWIG
#else // _WIN32
#define UDS_API
#endif // _WIN32



class UDS_API UDSException : public std::exception {
public:
    UDSException(const char *error);

    virtual const char* what() const noexcept;

private:
    std::string mError;
};

typedef unsigned long UDS_PID;
typedef uint8_t UDS_SID;

class UDSMessage;
typedef std::shared_ptr<UDSMessage> UDSMessagePtr;
typedef std::weak_ptr<UDSMessage> UDSMessageWeakPtr;

class UDS_API UDSMessage {
public:
    UDSMessage(uint8_t *data, size_t length);
    virtual ~UDSMessage();

    virtual UDS_SID getServiceID() const;
    virtual const std::vector<uint8_t> &getData() const;

protected:
    std::vector<uint8_t> mData;
};

class UDS_API UDSNegativeResponseMessage: public UDSMessage {
public:
    UDSNegativeResponseMessage(uint8_t *data, size_t length);
    virtual ~UDSNegativeResponseMessage();
    virtual UDS_SID getRequestServiceID() const;
    virtual uint8_t getErrorCode() const;
};

class UDS;
typedef std::shared_ptr<UDS> UDSPtr;
typedef std::weak_ptr<UDS> UDSWeakPtr;


class UDS_API UDS: public std::enable_shared_from_this<UDS> {
public:
    typedef unsigned long TimeType;

    UDS(UDS_PID tester, UDS_PID ecu);
    virtual ~UDS();

    virtual UDSMessagePtr send(const UDSMessagePtr request, TimeType timeout) = 0;

protected:
    UDSMessagePtr buildMessage(uint8_t *data, size_t length) const;

    UDS_PID mTester;
    UDS_PID mEcu;
};

#endif /* _UDS_H */
