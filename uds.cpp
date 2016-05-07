#include "uds.h"

static UDSMessagePtr buildMessage(uint8_t *data, size_t length) {
    if (length > UDS_SID_OFFSET) {
        UDS_SID serviceID = data[UDS_SID_OFFSET];
        if ((serviceID & UDS_SID_MASK) == UDS_SERVICE_ERR) {
            return std::make_shared<UDSNegativeResponseMessage>(data, length);
        }
    }
    return std::make_shared<UDSMessage>(data, length);
}

UDSException::UDSException(const char *error) {
    if (error != NULL) {
        mError.append(error);
    }
}

const char *UDSException::what() const noexcept {
    return mError.c_str();
}

UDS::UDS(UDS_PID tester, UDS_PID ecu) : mTester(tester), mEcu(ecu) {

}

UDS::~UDS() {

}

UDSMessagePtr UDS::buildMessage(uint8_t *data, size_t length) const {
    return ::buildMessage(data, length);
}


UDSMessage::UDSMessage(uint8_t *data, size_t length) {
    mData.resize(length);
    memcpy(&(mData[0]), data, length);
}

UDSMessage::~UDSMessage() {

}

UDS_SID UDSMessage::getServiceID() const {
    return mData[UDS_SID_OFFSET];
}

const std::vector <uint8_t> &UDSMessage::getData() const {
    return mData;
}

UDSNegativeResponseMessage::UDSNegativeResponseMessage(uint8_t *data, size_t length) : UDSMessage(data, length) {

}

UDSNegativeResponseMessage::~UDSNegativeResponseMessage() {

}

UDS_SID UDSNegativeResponseMessage::getRequestServiceID() const {
    return mData[UDS_ERR_SID_OFFSET];
}

uint8_t UDSNegativeResponseMessage::getErrorCode() const {
    return mData[UDS_ERR_CODE_OFFSET];
}