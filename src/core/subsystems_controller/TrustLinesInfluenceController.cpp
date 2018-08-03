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

    mTerminateProcessOnScheduler = false;
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
        if (mForbiddenReceiveMessageType == Message::Debug) {
            throw IOError("Test IO exception on TL modifying stage");
        } else {
            throw Exception("Test exception on TL modifying stage");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnTLProcessingResponseStage()
{
    if (mThrowExceptionOnTLProcessingResponseStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            throw IOError("Test IO exception on TL response processing stage");
        } else {
            throw Exception("Test exception on TL response processing stage");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnKeysSharingStage()
{
    if (mThrowExceptionOnKeysSharingStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            throw IOError("testThrowExceptionOnKeysSharingStage IO");
        } else {
            throw Exception("testThrowExceptionOnKeysSharingStage");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnKeysSharingProcessingResponseStage()
{
    if (mThrowExceptionOnKeysSharingResponseProcessingStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            throw IOError("testThrowExceptionOnKeysSharingProcessingResponseStage IO");
        } else {
            throw Exception("testThrowExceptionOnKeysSharingProcessingResponseStage");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnKeysSharingReceiverStage()
{
    if (mThrowExceptionOnKeysSharingReceiverStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            throw IOError("testThrowExceptionOnKeysSharingReceiverStage IO");
        } else {
            throw Exception("testThrowExceptionOnKeysSharingReceiverStage");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnAuditStage()
{
    if (mThrowExceptionOnAuditStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            throw IOError("testThrowExceptionOnAuditStage IO");
        } else {
            throw Exception("testThrowExceptionOnAuditStage");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnAuditResponseProcessingStage()
{
    if (mThrowExceptionOnAuditResponseProcessingStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            throw IOError("testThrowExceptionOnAuditResponseProcessingStage IO");
        } else {
            throw Exception("testThrowExceptionOnAuditResponseProcessingStage");
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnTLModifyingStage()
{
    if (mTerminateProcessOnTLModifyingStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            debug() << "testTerminateProcessOnTLModifyingStage";
            exit(100);
        } else {
            debug() << "testTerminateProcessOnTLModifyingStage Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnTLProcessingResponseStage()
{
    if (mTerminateProcessOnTLProcessingResponseStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            debug() << "testTerminateProcessOnTLProcessingResponseStage";
            exit(100);
        } else {
            debug() << "testTerminateProcessOnTLProcessingResponseStage Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnKeysSharingStage()
{
    if (mTerminateProcessOnKeysSharingStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            debug() << "testTerminateProcessOnKeysSharingStage";
            exit(100);
        } else {
            debug() << "testTerminateProcessOnKeysSharingStage Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnKeysSharingProcessingResponseStage()
{
    if (mTerminateProcessOnKeysSharingResponseProcessingStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            debug() << "testTerminateProcessOnKeysSharingProcessingResponseStage";
            exit(100);
        } else {
            debug() << "testTerminateProcessOnKeysSharingProcessingResponseStage Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnKeysSharingReceiverStage()
{
    if (mTerminateProcessOnKeysSharingReceiverStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            debug() << "testTerminateProcessOnKeysSharingReceiverStage";
            exit(100);
        } else {
            debug() << "testTerminateProcessOnKeysSharingReceiverStage Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnAuditStage()
{
    if (mTerminateProcessOnAuditStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            debug() << "testTerminateProcessOnAuditStage";
            exit(100);
        } else {
            debug() << "testTerminateProcessOnAuditStage Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnAuditResponseProcessingStage()
{
    if (mTerminateProcessOnAuditResponseProcessingStage) {
        if (mForbiddenReceiveMessageType == Message::Debug) {
            debug() << "testTerminateProcessOnAuditResponseProcessingStage";
            exit(100);
        } else {
            debug() << "testTerminateProcessOnAuditResponseProcessingStage Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

bool TrustLinesInfluenceController::isTerminateProcessOnScheduler()
{
    return mTerminateProcessOnScheduler;
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