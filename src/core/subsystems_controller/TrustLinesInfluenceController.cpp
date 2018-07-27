#include "TrustLinesInfluenceController.h"

TrustLinesInfluenceController::TrustLinesInfluenceController(
    Logger &log):
    mLog(log)
{
    mCountForbiddenReceivedMessages = 0;
    mForbiddenReceiveMessageType = Message::Debug;
    mForbidReceiveMessages = false;
    mThrowExceptionOnTLModifyingStage = false;
    mThrowExceptionOnTLProcessingResponseStage = false;
    mThrowExceptionOnKeysSharingStage = false;
    mThrowExceptionOnKeysSharingResponseProcessingStage = false;
    mThrowExceptionOnKeysSharingReceiverStage = false;
    mThrowExceptionOnAuditStage = false;
    mThrowExceptionOnAuditResponseProcessingStage = false;
    mTerminateProcessOnTLModifyingStage = false;
    mTerminateProcessOnTLProcessingResponseStage = false;
    mTerminateProcessOnKeysSharingStage = false;
    mTerminateProcessOnKeysSharingResponseProcessingStage = false;
    mTerminateProcessOnKeysSharingReceiverStage = false;
    mTerminateProcessOnAuditStage = false;
    mTerminateProcessOnAuditResponseProcessingStage = false;
}

void TrustLinesInfluenceController::setFlags(size_t flags)
{
    debug() << "setFlags: " << flags;
    mForbidReceiveMessages = (flags & 0x4) != 0;

    mThrowExceptionOnTLModifyingStage = (flags & 0x800) != 0;
    mThrowExceptionOnTLProcessingResponseStage = (flags & 0x1000) != 0;
    mThrowExceptionOnKeysSharingStage = (flags & 0x2000) != 0;
    mThrowExceptionOnKeysSharingResponseProcessingStage = (flags & 0x4000) != 0;
    mThrowExceptionOnKeysSharingReceiverStage = (flags & 0x8000) != 0;
    mThrowExceptionOnAuditStage = (flags & 0x10000) != 0;
    mThrowExceptionOnAuditResponseProcessingStage = (flags & 0x20000) != 0;

    mTerminateProcessOnTLModifyingStage = (flags & 0x200000) != 0;
    mTerminateProcessOnTLProcessingResponseStage = (flags & 0x400000) != 0;
    mTerminateProcessOnKeysSharingStage = (flags & 0x800000) != 0;
    mTerminateProcessOnKeysSharingResponseProcessingStage = (flags & 0x1000000) != 0;
    mTerminateProcessOnKeysSharingReceiverStage = (flags & 0x2000000) != 0;
    mTerminateProcessOnAuditStage = (flags & 0x4000000) != 0;
    mTerminateProcessOnAuditResponseProcessingStage = (flags & 0x8000000) != 0;
}

void TrustLinesInfluenceController::setForbiddenReceiveMessageType(
    const Message::MessageType forbiddenMessageType)
{
    debug() << "setForbiddenReceiveMessageType " << forbiddenMessageType;
    mForbiddenReceiveMessageType = forbiddenMessageType;
}

void TrustLinesInfluenceController::setCountForbiddenReceivedMessages(
    uint32_t countForbiddenReceivedMessages)
{
    debug() << "setCountForbiddenReceivedMessages " << countForbiddenReceivedMessages;
    mCountForbiddenReceivedMessages = countForbiddenReceivedMessages;
}

bool TrustLinesInfluenceController::checkReceivedMessage(
    Message::MessageType receivedMessageType)
{
    if (!mForbidReceiveMessages) {
        return false;
    }
    if (mCountForbiddenReceivedMessages == 0) {
        return false;
    }
    if (receivedMessageType == mForbiddenReceiveMessageType) {
        mCountForbiddenReceivedMessages--;
        return true;
    }
    return false;
}

void TrustLinesInfluenceController::testThrowExceptionOnTLModifyingStage()
{
    if (mThrowExceptionOnTLModifyingStage) {
        throw IOError("Test exception on TL modifying stage");
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnTLProcessingResponseStage()
{
    if (mThrowExceptionOnTLProcessingResponseStage) {
        throw IOError("Test exception on TL response processing stage");
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnKeysSharingStage()
{
    if (mThrowExceptionOnKeysSharingStage) {
        throw IOError("testThrowExceptionOnKeysSharingStage");
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnKeysSharingProcessingResponseStage()
{
    if (mThrowExceptionOnKeysSharingResponseProcessingStage) {
        throw IOError("testThrowExceptionOnKeysSharingProcessingResponseStage");
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnKeysSharingReceiverStage()
{
    if (mThrowExceptionOnKeysSharingReceiverStage) {
        throw IOError("testThrowExceptionOnKeysSharingReceiverStage");
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnAuditStage()
{
    if (mThrowExceptionOnAuditStage) {
        throw IOError("testThrowExceptionOnAuditStage");
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnAuditResponseProcessingStage()
{
    if (mThrowExceptionOnAuditResponseProcessingStage) {
        throw IOError("testThrowExceptionOnAuditResponseProcessingStage");
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnTLModifyingStage()
{
    if (mTerminateProcessOnTLModifyingStage) {
        debug() << "testTerminateProcessOnTLModifyingStage";
        exit(100);
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnTLProcessingResponseStage()
{
    if (mTerminateProcessOnTLProcessingResponseStage) {
        debug() << "testTerminateProcessOnTLProcessingResponseStage";
        exit(100);
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnKeysSharingStage()
{
    if (mTerminateProcessOnKeysSharingStage) {
        debug() << "testTerminateProcessOnKeysSharingStage";
        exit(100);
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnKeysSharingProcessingResponseStage()
{
    if (mTerminateProcessOnKeysSharingResponseProcessingStage) {
        debug() << "testTerminateProcessOnKeysSharingProcessingResponseStage";
        exit(100);
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnKeysSharingReceiverStage()
{
    if (mTerminateProcessOnKeysSharingReceiverStage) {
        debug() << "testTerminateProcessOnKeysSharingReceiverStage";
        exit(100);
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnAuditStage()
{
    if (mTerminateProcessOnAuditStage) {
        debug() << "testTerminateProcessOnAuditStage";
        exit(100);
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnAuditResponseProcessingStage()
{
    if (mTerminateProcessOnAuditResponseProcessingStage) {
        debug() << "testTerminateProcessOnAuditResponseProcessingStage";
        exit(100);
    }
}

LoggerStream TrustLinesInfluenceController::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TrustLinesInfluenceController::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream TrustLinesInfluenceController::warning() const
{
    return mLog.warning(logHeader());
}

const string TrustLinesInfluenceController::logHeader() const
{
    stringstream s;
    s << "[TrustLinesInfluenceController]";
    return s.str();
}