#include "uds_j2534.h"
#include <string.h>


UDS_J2534::UDS_J2534(J2534ChannelPtr channel, UDS_PID tester, UDS_PID ecu, unsigned long protocolID,
                     unsigned long flags) :
        UDS(tester, ecu), mChannel(channel), mProtocolID(protocolID), mFlags(flags) {

}

UDS_J2534::~UDS_J2534() {

}

static void pid2Data(UDS_PID pid, uint8_t *data) {
    data[0] = 0x1F & (pid >> 24);
    data[1] = 0xFF & (pid >> 16);
    data[2] = 0xFF & (pid >> 8);
    data[3] = 0xFF & (pid >> 0);
}

UDSMessagePtr UDS_J2534::send(const UDSMessagePtr &request, TimeType timeout) {
    PASSTHRU_MSG MaskMsg;
    PASSTHRU_MSG PatternMsg;
    PASSTHRU_MSG FlowControlMsg;
    MaskMsg.ProtocolID = PatternMsg.ProtocolID = FlowControlMsg.ProtocolID = mProtocolID;
    MaskMsg.TxFlags = PatternMsg.TxFlags = FlowControlMsg.TxFlags = mFlags;
    MaskMsg.DataSize = PatternMsg.DataSize = FlowControlMsg.DataSize = 4;
    MaskMsg.Data[0] = MaskMsg.Data[1] = MaskMsg.Data[2] = MaskMsg.Data[3] = 0xFF;
    pid2Data(mTester, &PatternMsg.Data[0]);
    pid2Data(mEcu, &FlowControlMsg.Data[0]);
    J2534Channel::MessageFilter messageFilter = mChannel->startMsgFilter(FLOW_CONTROL_FILTER, &MaskMsg, &PatternMsg,
                                                                         &FlowControlMsg);

    UDSMessagePtr ret;
    try {
        size_t ret;
        mChannel->ioctl(CLEAR_RX_BUFFER, 0, 0);

        std::vector <PASSTHRU_MSG> msgs;
        msgs.resize(1);
        PASSTHRU_MSG *msg = &msgs[0];

        msg->ProtocolID = mProtocolID;
        msg->TxFlags = mFlags;
        const std::vector <uint8_t> &data = request->getData();
        pid2Data(mEcu, &msg->Data[0]);
        memcpy(&(msg->Data[4]), &(data[0]), data.size());
        msg->DataSize = data.size() + 4;

        /* Write of message */
        ret = mChannel->writeMsgs(msgs, timeout);
        if (ret != 1) {
            throw UDSException("Can't send the message");
        }

        /* Start of message */
        ret = mChannel->readMsgs(msgs, timeout);
        if (ret == 0) {
            throw UDSException("No message read");
        }
        if (!(msg->RxStatus & START_OF_MESSAGE)) {
            throw UDSException("Not start of message");
        }

        /* Read the message */
        ret = mChannel->readMsgs(msgs, timeout);
        if (ret == 0) {
            throw UDSException("No message read");
        }

        /* Sanity check */
        if (msg->DataSize < 4) {
            throw UDSException("Invalid data size");
        }
        ret = buildMessage(&(msg->Data[4]), msg->DataSize - 4);
        mChannel->stopMsgFilter(messageFilter);
    } catch (...) {
        mChannel->stopMsgFilter(messageFilter);
        throw;
    }
    return ret;
}