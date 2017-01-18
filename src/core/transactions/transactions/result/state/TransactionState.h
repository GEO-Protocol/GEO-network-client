#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H

#include <stdint.h>
#include <vector>
#include "../../../../network/messages/Message.h"


using namespace std;

class TransactionState {
public:
    typedef shared_ptr<TransactionState> Shared;
    typedef shared_ptr<const TransactionState> SharedConst;

public:
    TransactionState(
            uint64_t timeout);

    TransactionState(
            Message::MessageTypeID transactionType);

    TransactionState(
            uint64_t timeout,
            Message::MessageTypeID transactionType);

    ~TransactionState();

public:
    const uint64_t timeout() const;

    const vector<Message::MessageTypeID> transactionsTypes() const;

private:
    uint64_t mTimeout;
    vector<Message::MessageTypeID> mTypes;

};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
