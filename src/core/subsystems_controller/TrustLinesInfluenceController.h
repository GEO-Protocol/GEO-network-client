#ifndef GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECONTROLLER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECONTROLLER_H

#include "../network/messages/Message.hpp"
#include "../transactions/transactions/base/BaseTransaction.h"
#include "../logger/Logger.h"
#include "../common/exceptions/IOError.h"

class TrustLinesInfluenceController {

public:
    TrustLinesInfluenceController(
        Logger &log);

public:
    void setFlags(size_t flags);

    void setFirstParameter(
        uint32_t firstParameter);

    void setSecondParameter(
        uint32_t secondParameter);

    // on this method firstParameter - type of forbidden message and second parameter - count forbidden messages
    bool checkReceivedMessage(
        Message::MessageType receivedMessageType);

    // on throws methods first parameter - type of transaction and second parameter - type of exception
    void testThrowExceptionOnSourceInitializationStage(
        BaseTransaction::TransactionType transactionType);

    void testThrowExceptionOnSourceProcessingResponseStage(
        BaseTransaction::TransactionType transactionType);

    void testThrowExceptionOnSourceResumingStage(
        BaseTransaction::TransactionType transactionType);

    void testThrowExceptionOnTargetStage(
        BaseTransaction::TransactionType transactionType);

    // on terminating methods first parameter - type of transaction
    // and second parameter - place of terminating (1 - on transaction, 2 - after)
    void testTerminateProcessOnSourceInitializationStage(
        BaseTransaction::TransactionType transactionType);

    void testTerminateProcessOnSourceProcessingResponseStage(
        BaseTransaction::TransactionType transactionType);

    void testTerminateProcessOnSourceResumingStage(
        BaseTransaction::TransactionType transactionType);

    void testTerminateProcessOnTargetStage(
        BaseTransaction::TransactionType transactionType);

    bool isTerminateProcessOnScheduler();

protected:
    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    uint32_t mFirstParameter;
    uint32_t mSecondParameter;

    bool mForbidReceiveMessages;
    bool mThrowExceptionOnSourceInitializationStage;
    bool mThrowExceptionOnSourceProcessingResponseStage;
    bool mThrowExceptionOnSourceResumingStage;
    bool mThrowExceptionOnTargetStage;

    bool mTerminateProcessOnSourceInitializationStage;
    bool mTerminateProcessOnSourceProcessingResponseStage;
    bool mTerminateProcessOnSourceResumingStage;
    bool mTerminateProcessOnTargetStage;

    bool mTerminateProcessOnScheduler;

    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECONTROLLER_H
