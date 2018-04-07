#ifndef SUBSYSTEMSINFLUENCECOMMAND_H
#define SUBSYSTEMSINFLUENCECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/exceptions/ValueError.h"

class SubsystemsInfluenceCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<SubsystemsInfluenceCommand> Shared;

public:
    SubsystemsInfluenceCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    size_t flags() const;

    const NodeUUID &forbiddenNodeUUID() const;

    const TrustLineAmount &forbiddenAmount() const;

protected:
//    bits order:
//    0 - Network in general   0x1  // 1
//    1 - Possibility closing cycles 0x2 // 2

//    2 - Forbid Send Message To Receiver On Reservation Stage   0x4  // 4
//    3 - Forbid Send Message To Coordinator On Reservation Stage   0x8  // 8
//    4 - Forbid Send Request To Intermediate Node On Reservation Stage   0x10  // 16
//    5 - Forbid Send Response To Intermediate Node On Reservation Stage   0x20  // 32
//    6 - Forbid Send Message With Final Path Configuration   0x40  // 64
//    7 - Forbid Send Message On Final Amount Clarification Stage   0x80  // 128
//    8 - Forbid Send Message On Vote Stage To Next Node   0x100  // 256
//    9 - Forbid Send Message On Vote Stage To Coordinator   0x200  // 512
//    10 - Forbid Send Message On Vote Consistency Stage   0x400  // 1024

//    11 - throw exception on previous neighbor request processing stage  0x800  // 2048
//    12 - throw exception on Coordinator Request Processing Stage  0x1000  // 4096
//    13 - throw exception on Next Neighbor Response Processing Stage  0x2000  // 8192
//    14 - throw exception on vote stage  0x4000  // 16384
//    15 - throw exception on vote consistency stage  0x8000  // 32768
//    16 - throw exception on coordinator after approve before send message  0x10000 // 65536

//    21 - terminate process on previous neighbor request processing stage  0x200000  // 2097152
//    22 - terminate process on Coordinator Request Processing Stage  0x400000  // 4194304
//    23 - terminate process on Next Neighbor Response Processing Stage  0x800000  // 8388608
//    24 - terminate process on vote stage  0x1000000  // 16777216
//    25 - terminate process on vote consistency stage  0x2000000  // 33554432
//    26 - terminate process on coordinator after approve before send message  0x4000000 // 67108864

//    31 - sleep on Previous Neighbor Request Processing Stage  0x80000000  //  2147483648
//    32 - sleep on Coordinator Request Processing Stage  0x100000000  //  4294967296
//    33 - sleep on Next Neighbor Response Processing Stage  0x200000000  //  8589934592
//    34 - sleep on Final Amount Clarification  0x400000000  // 17179869184
//    35 - sleep on Vote Stage  0x800000000  //  34359738368
//    36 - sleep on Vote Consistency Stage  0x1000000000  //  68719476736

//    40 - write visual results 0x10000000000  //  1099511627776
//    41 - forbid run payment transactions  0x20000000000  //  2199023255552
//    42 - forbid run trust line transactions  0x40000000000  //  4398046511104
//    43 - set node as gateway 0x80000000000  // 8796093022208

    size_t mFlags;
    // this parameter used for forbid send messages only for this node,
    // if parameter is 0, forbid send messages to all nodes
    NodeUUID mForbiddenNodeUUID;
    // this parameter used for forbid send message with this reservation amount
    // works in pair with mForbiddenNodeUUID
    TrustLineAmount mForbiddenAmount;
};

#endif // SUBSYSTEMSINFLUENCECOMMAND_H
