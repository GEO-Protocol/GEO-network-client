#include "TransactionState.h"

TransactionState::TransactionState(
        uint64_t timeout) :
        mTimeout(timeout) {}

TransactionState::TransactionState(
        Message::MessageTypeID transactionType) {
    mTypes.push_back(transactionType);
}

TransactionState::TransactionState(
        uint64_t timeout,
        Message::MessageTypeID transactionType) :
        mTimeout(timeout) {
    mTypes.push_back(transactionType);
}

TransactionState::~TransactionState() {
}

const uint64_t TransactionState::timeout() const {
    return mTimeout;
}

const vector<Message::MessageTypeID> TransactionState::transactionsTypes() const {
    return mTypes;
}


