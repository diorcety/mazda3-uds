#include "uds_j2534.h"


UDS_J2534::UDS_J2534(J2534ChannelPtr channel, UDS_PID tester, UDS_PID ecu, unsigned long protocolID, unsigned long flags) :
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

UDSMessagePtr UDS_J2534::send(const UDSMessagePtr request, TimeType timeout) {
    PASSTHRU_MSG MaskMsg;
    PASSTHRU_MSG PatternMsg;
    PASSTHRU_MSG FlowControlMsg;
    MaskMsg.ProtocolID = PatternMsg.ProtocolID = FlowControlMsg.ProtocolID = mProtocolID;
    MaskMsg.TxFlags = PatternMsg.TxFlags = FlowControlMsg.TxFlags = mFlags;
    MaskMsg.DataSize = PatternMsg.DataSize = FlowControlMsg.DataSize = 4;
    MaskMsg.Data[0] = MaskMsg.Data[1] = MaskMsg.Data[2] = MaskMsg.Data[3] = 0xFF;
    pid2Data(mTester, &PatternMsg.Data[0]);
    pid2Data(mEcu, &FlowControlMsg.Data[0]);
    J2534Channel::MessageFilter messageFilter = mChannel->startMsgFilter(FLOW_CONTROL_FILTER, &MaskMsg, &PatternMsg, &FlowControlMsg);

    UDSMessagePtr ret;
    try {
        mChannel->ioctl(CLEAR_RX_BUFFER, 0, 0);

        std::vector<PASSTHRU_MSG> msg;
        msg.resize(1);

        msg[0].ProtocolID = mProtocolID;
        msg[0].TxFlags = mFlags;
        const std::vector<uint8_t> &data = request->getData();
        pid2Data(mEcu, &msg[0].Data[0]);
        memcpy(&msg[0].Data[4], &data[0], data.size());

        /* Start of message */
        if(mChannel->readMsgs(msg, timeout) == 0 || !(msg[0].RxStatus & START_OF_MESSAGE)) {
            throw UDSException("Invalid state");
        }

        /* Read the message */
        if(mChannel->readMsgs(msg, timeout) == 0) {
            throw UDSException("Invalid state");
        }

        /* Sanity check */
        if(msg[0].DataSize < 4) {
            throw UDSException("Invalid data size");
        }
        ret = buildMessage(&msg[0].Data[4], msg[0].DataSize - 4);
        mChannel->stopMsgFilter(messageFilter);
    } catch(...) {
        mChannel->stopMsgFilter(messageFilter);
        throw;
    }
    return ret;
}