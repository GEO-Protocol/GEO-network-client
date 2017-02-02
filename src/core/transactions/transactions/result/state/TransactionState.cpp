#include "TransactionState.h"

TransactionState::TransactionState(
        uint64_t timeout,
        bool serialize) :

        mTimeout(timeout) {

    mSerialize = serialize;
}

TransactionState::TransactionState(
        Message::MessageTypeID transactionType,
        bool serialize) {

    mTypes.push_back(transactionType);
    mSerialize = serialize;
}

TransactionState::TransactionState(
        uint64_t timeout,
        Message::MessageTypeID transactionType,
        bool serialize) :

        mTimeout(timeout) {

    mTypes.push_back(transactionType);
    mSerialize = serialize;
}

TransactionState::~TransactionState() {
}

const uint64_t TransactionState::timeout() const {

    return mTimeout;
}

const vector<Message::MessageTypeID> TransactionState::transactionsTypes() const {

    return mTypes;
}

const bool TransactionState::needSerialize() const {

    return mSerialize;
}


