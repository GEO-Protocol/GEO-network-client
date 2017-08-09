#ifndef TOGGLENETWORKCOMMAND_H
#define TOGGLENETWORKCOMMAND_H

#include "BaseTestCommand.h"

#include "../../../../common/exceptions/ValueError.h"


#ifdef TESTS

/**
 * This command is used for enabling and disabling network
 */
class ToggleNetworkCommand:
    public BaseTestCommand {

public:
    typedef shared_ptr<ToggleNetworkCommand> Shared;

public:
    ToggleNetworkCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    size_t flags() const;

protected:
    [[deprecated]]
    virtual void parse(
        const string &){}

protected:
//    bits order:
//    0 - Network in general   0x1  // 1
//    1 - Possibility closing cycles 0x2 // 2
//    2 - Forbid Send Message To Coordinator On Reservation Stage   0x4  // 4
//    3 - Forbid Send Request To Intermediate Node On Reservation Stage   0x8  // 8
//    4 - Forbid Send Response To Intermediate Node On Reservation Stage   0x10  // 16
//    4 - Forbid Send Message On Final Amount Clarification Stage   0x20  // 32
//    5 - Forbid Send Message On Vote Stage   0x40  // 64
//    6 - Forbid Send Message On Vote Consistency Stage   0x80  // 128

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
};

#endif

#endif // TOGGLENETWORKCOMMAND_H
