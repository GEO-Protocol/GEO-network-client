#include "TrustLinesInfluenceController.h"

TrustLinesInfluenceController::TrustLinesInfluenceController(
    Logger &log):
    mLog(log)
{
    mFirstParameter = 0;
    mSecondParameter = 0;
    mThirdParameter = 0;
    mForbidReceiveMessages = false;
    mThrowExceptionOnSourceInitializationStage = false;
    mThrowExceptionOnSourceProcessingResponseStage = false;
    mThrowExceptionOnSourceResumingStage = false;
    mThrowExceptionOnTargetStage = false;
    mTerminateProcessOnSourceInitializationStage = false;
    mTerminateProcessOnSourceProcessingResponseStage = false;
    mTerminateProcessOnSourceResumingStage = false;
    mTerminateProcessOnTargetStage = false;

    mTerminateProcessOnScheduler = false;
}

void TrustLinesInfluenceController::setFlags(size_t flags)
{
    debug() << "setFlags: " << flags;
    mForbidReceiveMessages = (flags & 0x4) != 0;

    mThrowExceptionOnSourceInitializationStage = (flags & 0x800) != 0;
    mThrowExceptionOnSourceProcessingResponseStage = (flags & 0x1000) != 0;
    mThrowExceptionOnSourceResumingStage = (flags & 0x2000) != 0;
    mThrowExceptionOnTargetStage = (flags & 0x4000) != 0;

    mTerminateProcessOnSourceInitializationStage = (flags & 0x200000) != 0;
    mTerminateProcessOnSourceProcessingResponseStage = (flags & 0x400000) != 0;
    mTerminateProcessOnSourceResumingStage = (flags & 0x800000) != 0;
    mTerminateProcessOnTargetStage = (flags & 0x1000000) != 0;
}

void TrustLinesInfluenceController::setFirstParameter(
    uint32_t firstParameter)
{
    debug() << "setFirstParameter " << firstParameter;
    mFirstParameter = firstParameter;
}

void TrustLinesInfluenceController::setSecondParameter(
    uint32_t secondParameter)
{
    debug() << "setSecondParameter " << secondParameter;
    mSecondParameter = secondParameter;
}

void TrustLinesInfluenceController::setThirdParameter(
    uint32_t thirdParameter)
{
    debug() << "setThirdParameter " << thirdParameter;
    mThirdParameter = thirdParameter;
}

bool TrustLinesInfluenceController::checkReceivedMessage(
    Message::MessageType receivedMessageType)
{
    if (!mForbidReceiveMessages) {
        return false;
    }
    if (receivedMessageType != mFirstParameter) {
        return false;
    }
    if (mSecondParameter == 0) {
        return false;
    }
    if (mThirdParameter != 0) {
        mThirdParameter--;
        return false;
    }
    mSecondParameter--;
    return true;
}

void TrustLinesInfluenceController::testThrowExceptionOnSourceInitializationStage(
    BaseTransaction::TransactionType transactionType)
{
    info() << "testThrowExceptionOnSourceInitializationStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mThrowExceptionOnSourceInitializationStage) {
        mThrowExceptionOnSourceInitializationStage = false;
        if (mSecondParameter == 1) {
            throw Exception("Test exception");
        } else if (mSecondParameter == 2) {
            throw IOError("Test IO exception");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnSourceProcessingResponseStage(
    BaseTransaction::TransactionType transactionType)
{
    info() << "testThrowExceptionOnSourceProcessingResponseStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mThrowExceptionOnSourceProcessingResponseStage) {
        mThrowExceptionOnSourceProcessingResponseStage = false;
        if (mSecondParameter == 1) {
            throw Exception("Test exception");
        } else if (mSecondParameter == 2) {
            throw IOError("Test IO exception");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnSourceResumingStage(
        BaseTransaction::TransactionType transactionType)
{
    info() << "testThrowExceptionOnSourceResumingStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mThrowExceptionOnSourceResumingStage) {
        mThrowExceptionOnSourceResumingStage = false;
        if (mSecondParameter == 1) {
            throw Exception("Test exception");
        } else if (mSecondParameter == 2) {
            throw IOError("Test IO exception");
        }
    }
}

void TrustLinesInfluenceController::testThrowExceptionOnTargetStage(
    BaseTransaction::TransactionType transactionType)
{
    info() << "testThrowExceptionOnTargetStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mThrowExceptionOnTargetStage) {
        mThrowExceptionOnTargetStage = false;
        if (mSecondParameter == 1) {
            throw Exception("Test exception");
        } else if (mSecondParameter == 2) {
            throw IOError("Test IO exception");
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnSourceInitializationStage(
    BaseTransaction::TransactionType transactionType)
{
    info() << "testTerminateProcessOnSourceInitializationStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mTerminateProcessOnSourceInitializationStage) {
        if (mSecondParameter == 1) {
            debug() << "TerminateProcess";
            exit(100);
        } else if (mSecondParameter == 2) {
            debug() << "TerminateProcess Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnSourceProcessingResponseStage(
    BaseTransaction::TransactionType transactionType)
{
    info() << "testTerminateProcessOnSourceProcessingResponseStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mTerminateProcessOnSourceProcessingResponseStage) {
        if (mSecondParameter == 1) {
            debug() << "TerminateProcess";
            exit(100);
        } else if (mSecondParameter == 2) {
            debug() << "TerminateProcess Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnSourceResumingStage(
    BaseTransaction::TransactionType transactionType)
{
    info() << "testTerminateProcessOnSourceResumingStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mTerminateProcessOnSourceResumingStage) {
        if (mSecondParameter == 1) {
            debug() << "TerminateProcess";
            exit(100);
        } else if (mSecondParameter == 2) {
            debug() << "TerminateProcess Scheduler";
            mTerminateProcessOnScheduler = true;
        }
    }
}

void TrustLinesInfluenceController::testTerminateProcessOnTargetStage(
    BaseTransaction::TransactionType transactionType)
{
    info() << "testTerminateProcessOnTargetStage " << transactionType;
    if (mFirstParameter != transactionType) {
        return;
    }
    if (mTerminateProcessOnTargetStage) {
        if (mSecondParameter == 1) {
            debug() << "TerminateProcess";
            exit(100);
        } else if (mSecondParameter == 2) {
            debug() << "TerminateProcess Scheduler";
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