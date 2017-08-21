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
    [[deprecated]]
    virtual void parse(
        const string &){}

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

//    21 - throw exception on previous neighbor request processing stage  0x200000  // 2097152
//    22 - throw exception on Coordinator Request Processing Stage  0x400000  // 4194304
//    23 - throw exception on Next Neighbor Response Processing Stage  0x800000  // 8388608
//    24 - throw exception on vote stage  0x1000000  // 16777216
//    25 - throw exception on vote consistency stage  0x2000000  // 33554432

    size_t mFlags;
    // this parameter used for forbid send messages only for this node,
    // if parameter is 0, forbid send messages to all nodes
    NodeUUID mForbiddenNodeUUID;
    // this parameter used for forbid send message with this reservation amount
    // works in pair with mForbiddenNodeUUID
    TrustLineAmount mForbiddenAmount;
};

#endif // SUBSYSTEMSINFLUENCECOMMAND_H
