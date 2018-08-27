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

#include "uds_j2534.h"
#include <string.h>

#define J2534_DATA_OFFSET 4

UDS_J2534::UDS_J2534(J2534ChannelPtr channel, UDS_PID tester, UDS_PID ecu, unsigned long protocolID,
                     unsigned long flags) :
        UDS(tester, ecu), mChannel(channel), mProtocolID(protocolID), mFlags(flags) {
            
    // Set BS and STMIN
    SCONFIG CfgItem[2];
    SCONFIG_LIST Input;

    CfgItem[0].Parameter = ISO15765_BS;
    CfgItem[0].Value = 0x20; /* BlockSize is 32 frames */
    CfgItem[1].Parameter = ISO15765_STMIN;
    CfgItem[1].Value = 0x02; /* SeparationTime is 2 millisecond */
    Input.NumOfParams = 2; /* Configuration list has 2 items */
    Input.ConfigPtr = CfgItem;
    channel->ioctl(SET_CONFIG, &Input, NULL);
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
    size_t data_offset = J2534_DATA_OFFSET;
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
        size_t size;
        mChannel->ioctl(CLEAR_RX_BUFFER, 0, 0);

        std::vector <PASSTHRU_MSG> msgs;
        msgs.resize(1);
        PASSTHRU_MSG &msg = msgs[0];

        if (request) {
            msg.ProtocolID = mProtocolID;
            msg.TxFlags = mFlags;
            const std::vector <uint8_t> &data = request->getData();
            pid2Data(mEcu, &msg.Data[0]);
            memcpy(&(msg.Data[data_offset]), &(data[0]), data.size());
            msg.DataSize = data.size() + data_offset;

            /* Write of message */
            size = mChannel->writeMsgs(msgs, timeout);
            if (size != 1) {
                throw UDSException("Can't send the message");
            }
        }

        /* Get message */
        while(true) {
            size = mChannel->readMsgs(msgs, timeout);
            if (size == 0) {
                throw UDSException("No message read");
            }
            if (!(msg.RxStatus & START_OF_MESSAGE)) {
                break;
            }
        }

        /* Sanity check */
        if (msg.DataSize < data_offset) {
            throw UDSException("Invalid data size");
        }
        ret = buildMessage(&(msg.Data[data_offset]), msg.DataSize - data_offset);
        mChannel->stopMsgFilter(messageFilter);
    } catch (...) {
        mChannel->stopMsgFilter(messageFilter);
        throw;
    }
    return ret;
}
