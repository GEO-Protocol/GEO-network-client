#ifndef GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECOMMAND_H
#define GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/exceptions/ValueError.h"

class TrustLinesInfluenceCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TrustLinesInfluenceCommand> Shared;

public:
    TrustLinesInfluenceCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    size_t flags() const;

    const uint32_t firstParameter() const;

    const uint32_t secondParameter() const;

    const uint32_t thirdParameter() const;

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

//    11 - throw exception on source initialization stage  0x800  // 2048
//    12 - throw exception on source response processing stage  0x1000  // 4096
//    13 - throw exception on source resuming stage  0x2000  // 8192
//    14 - throw exception on target stage  0x4000  // 16384

//    21 - terminate process on source initialization stage  0x200000  // 2097152
//    22 - terminate process on source response processing stage  0x400000  // 4194304
//    23 - terminate process on source resuming stage  0x800000  // 8388608
//    24 - terminate process on target stage  0x1000000  // 16777216

    size_t mFlags;
    // this parameter used in different cases in different ways
    uint32_t mFirstParameter;
    uint32_t mSecondParameter;
    uint32_t mThirdParameter;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECOMMAND_H
