#ifndef GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECONTROLLER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECONTROLLER_H

#include "../network/messages/Message.hpp"
#include "../logger/Logger.h"
#include "../common/exceptions/IOError.h"

class TrustLinesInfluenceController {

public:
    TrustLinesInfluenceController(
        Logger &log);

public:
    void setFlags(size_t flags);

    void setForbiddenReceiveMessageType(
        const Message::MessageType forbiddenMessageType);

    void setCountForbiddenReceivedMessages(
        uint32_t countForbiddenReceivedMessages);

    bool checkReceivedMessage(
        Message::MessageType receivedMessageType);

    void testThrowExceptionOnTLModifyingStage();

    void testThrowExceptionOnTLProcessingResponseStage();

    void testThrowExceptionOnKeysSharingStage();

    void testThrowExceptionOnKeysSharingProcessingResponseStage();

    void testThrowExceptionOnKeysSharingReceiverStage();

    void testThrowExceptionOnAuditStage();

    void testThrowExceptionOnAuditResponseProcessingStage();

    void testTerminateProcessOnTLModifyingStage();

    void testTerminateProcessOnTLProcessingResponseStage();

    void testTerminateProcessOnKeysSharingStage();

    void testTerminateProcessOnKeysSharingProcessingResponseStage();

    void testTerminateProcessOnKeysSharingReceiverStage();

    void testTerminateProcessOnAuditStage();

    void testTerminateProcessOnAuditResponseProcessingStage();

    bool isTerminateProcessOnScheduler();

protected:
    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    Message::MessageType mForbiddenReceiveMessageType;
    uint32_t mCountForbiddenReceivedMessages;

    bool mForbidReceiveMessages;
    bool mThrowExceptionOnTLModifyingStage;
    bool mThrowExceptionOnTLProcessingResponseStage;
    bool mThrowExceptionOnKeysSharingStage;
    bool mThrowExceptionOnKeysSharingResponseProcessingStage;
    bool mThrowExceptionOnKeysSharingReceiverStage;
    bool mThrowExceptionOnAuditStage;
    bool mThrowExceptionOnAuditResponseProcessingStage;

    bool mTerminateProcessOnTLModifyingStage;
    bool mTerminateProcessOnTLProcessingResponseStage;
    bool mTerminateProcessOnKeysSharingStage;
    bool mTerminateProcessOnKeysSharingResponseProcessingStage;
    bool mTerminateProcessOnKeysSharingReceiverStage;
    bool mTerminateProcessOnAuditStage;
    bool mTerminateProcessOnAuditResponseProcessingStage;

    bool mTerminateProcessOnScheduler;

    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESINFLUENCECONTROLLER_H
