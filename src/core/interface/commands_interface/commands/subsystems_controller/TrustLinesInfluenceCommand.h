#ifndef GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECOMMAND_H
#define GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/exceptions/ValueError.h"
#include "../../../../network/messages/Message.hpp"

class TrustLinesInfluenceCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TrustLinesInfluenceCommand> Shared;

public:
    TrustLinesInfluenceCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    size_t flags() const;

    const Message::MessageType forbiddenReceiveMessageType() const;

    const uint32_t countForbiddenReceivedMessages() const;

protected:
//    bits order:
//    0 - Network in general   0x1  // 1
//    1 -  0x2 // 2

//    2 - Forbid Receive Message of Specify Type   0x4  // 4
//    3 - Forbid Send Message of Specify Type   0x8  // 8
//    4 -    0x10  // 16
//    5 -    0x20  // 32
//    6 -    0x40  // 64
//    7 -    0x80  // 128
//    8 -    0x100  // 256
//    9 -    0x200  // 512
//    10 -    0x400  // 1024

//    11 - throw exception on TL modifying stage  0x800  // 2048
//    12 - throw exception on TL response processing Stage  0x1000  // 4096
//    13 - throw exception on keys sharing Stage  0x2000  // 8192
//    14 - throw exception on keys sharing response processing stage  0x4000  // 16384
//    15 - throw exception on receiver keys sharing Stage  0x8000  // 32768
//    16 - throw exception on audit stage  0x10000  // 65536
//    17 - throw exception on audit response processing stage  0x20000 // 131072

//    21 - terminate process on TL modifying stage  0x200000  // 2097152
//    22 - terminate process on TL response processing stage  0x400000  // 4194304
//    23 - terminate process on keys sharing stage  0x800000  // 8388608
//    24 - terminate process on keys sharing response processing stage  0x1000000  // 16777216
//    25 - terminate process on receiver keys sharing stage  0x2000000  // 33554432
//    26 - terminate process on audit stage  0x4000000  // 67108864
//    27 - terminate process on audit response processing stage  0x8000000 // 134217728

//    31 - sleep on Previous Neighbor Request Processing Stage  0x80000000  //  2147483648
//    32 - sleep on Coordinator Request Processing Stage  0x100000000  //  4294967296
//    33 - sleep on Next Neighbor Response Processing Stage  0x200000000  //  8589934592
//    34 - sleep on Final Amount Clarification  0x400000000  // 17179869184
//    35 - sleep on Vote Stage  0x800000000  //  34359738368
//    36 - sleep on Vote Consistency Stage  0x1000000000  //  68719476736

    size_t mFlags;
    // this parameter used for forbid receiving message of specified type
    Message::MessageType mForbiddenReceiveMessageType;
    uint32_t mCountForbiddenReceivedMessages;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECOMMAND_H
