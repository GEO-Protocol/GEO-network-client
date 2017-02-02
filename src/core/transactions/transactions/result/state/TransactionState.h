#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H

#include "../../../../network/messages/Message.h"

#include <stdint.h>
#include <vector>

using namespace std;

class TransactionState {
public:
    typedef shared_ptr<TransactionState> Shared;
    typedef shared_ptr<const TransactionState> SharedConst;

public:
    TransactionState(
            uint64_t timeout,
            bool serialize);

    TransactionState(
            Message::MessageTypeID transactionType,
            bool serialize);

    TransactionState(
            uint64_t timeout,
            Message::MessageTypeID transactionType,
            bool serialize);

    ~TransactionState();

public:
    const uint64_t timeout() const;

    const vector<Message::MessageTypeID> transactionsTypes() const;

    const bool needSerialize() const;

private:
    uint64_t mTimeout;
    vector<Message::MessageTypeID> mTypes;
    bool mSerialize;

};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
