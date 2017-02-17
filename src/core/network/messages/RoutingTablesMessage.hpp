#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H

#include "Message.hpp"

#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

class RoutingTablesMessage : public Message {
public:
    typedef shared_ptr<RoutingTablesMessage> Shared;
    typedef uint64_t RecordsCount;


public:
    virtual const MessageType typeID() const = 0;

    const TransactionUUID &transactionUUID() const {

        throw NotImplementedError("RoutingTablesMessage: public Message::transactionUUID:"
                                      "Method not implemented.");
    }

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

protected:
    RoutingTablesMessage() {};

    RoutingTablesMessage(
        NodeUUID &senderUUID) :

        Message(senderUUID) {}

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;

};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
