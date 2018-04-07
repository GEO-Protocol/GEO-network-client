#include "SubsystemsController.h"

SubsystemsController::SubsystemsController(
    Logger &log):
    mLog(log)
{
    mIsNetworkOn = true;
    mIsRunCycleClosingTransactions = true;
    mIsRunPaymentTransactions = true;
    mIsRunTrustLineTransactions = true;
    mIsWriteVisualResults = false;

    mForbidSendMessageToReceiverOnReservationStage = false;
    mForbidSendMessageToCoordinatorOnReservationStage = false;
    mForbidSendRequestToIntNodeOnReservationStage = false;
    mForbidSendResponseToIntNodeOnReservationStage = false;
    mForbidSendMessageWithFinalPathConfiguration = false;
    mForbidSendMessageOnFinalAmountClarificationStage = false;
    mForbidSendMessageToNextNodeOnVoteStage = false;
    mForbidSendMessageToCoordinatorOnVoteStage = false;
    mForbidSendMessageOnVoteConsistencyStage = false;

    mThrowExceptionOnPreviousNeighborRequestProcessingStage = false;
    mThrowExceptionOnCoordinatorRequestProcessingStage = false;
    mThrowExceptionOnNextNeighborResponseProcessingStage = false;
    mThrowExceptionOnVoteStage = false;
    mThrowExceptionOnVoteConsistencyStage = false;
    mThrowExceptionOnCoordinatorAfterApproveBeforeSendMessage = false;

    mTerminateProcessOnPreviousNeighborRequestProcessingStage = false;
    mTerminateProcessOnCoordinatorRequestProcessingStage = false;
    mTerminateProcessOnNextNeighborResponseProcessingStage = false;
    mTerminateProcessOnVoteStage = false;
    mTerminateProcessOnVoteConsistencyStage = false;
    mTerminateProcessOnCoordinatorAfterApproveBeforeSendMessage = false;

    mSleepOnPreviousNeighborRequestProcessingStage = false;
    mSleepOnCoordinatorRequestProcessingStage = false;
    mSleepOnNextNeighborResponseProcessingStage = false;
    mSleepOnFinalAmountClarificationStage = false;
    mSleepOnVoteStage = false;
    mSleepOnVoteConsistencyStage = false;

    mCountForbiddenMessages = 0;
    mForbiddenNodeUUID = NodeUUID::empty();
}

void SubsystemsController::setFlags(size_t flags)
{
    debug() << "setFlags: " << flags;
    mIsNetworkOn = (flags & 0x1) == 0;
    if (!mIsNetworkOn) {
        mCountForbiddenMessages = UINT32_MAX;
    } else {
        mCountForbiddenMessages = 0;
    }
    mIsRunCycleClosingTransactions = (flags & 0x2) == 0;

    mForbidSendMessageToReceiverOnReservationStage = (flags & 0x4) > 0;
    mForbidSendMessageToCoordinatorOnReservationStage = (flags & 0x8) > 0;
    mForbidSendRequestToIntNodeOnReservationStage = (flags & 0x10) > 0;
    mForbidSendResponseToIntNodeOnReservationStage = (flags & 0x20) > 0;
    mForbidSendMessageWithFinalPathConfiguration = (flags & 0x40) > 0;
    mForbidSendMessageOnFinalAmountClarificationStage = (flags & 0x80) > 0;
    mForbidSendMessageToNextNodeOnVoteStage = (flags & 0x100) > 0;
    mForbidSendMessageToCoordinatorOnVoteStage = (flags & 0x200) > 0;
    mForbidSendMessageOnVoteConsistencyStage = (flags & 0x400) > 0;

    mThrowExceptionOnPreviousNeighborRequestProcessingStage = (flags & 0x800) > 0;
    mThrowExceptionOnCoordinatorRequestProcessingStage = (flags & 0x1000) > 0;
    mThrowExceptionOnNextNeighborResponseProcessingStage = (flags & 0x2000) > 0;
    mThrowExceptionOnVoteStage = (flags & 0x4000) > 0;
    mThrowExceptionOnVoteConsistencyStage = (flags & 0x8000) > 0;
    mThrowExceptionOnCoordinatorAfterApproveBeforeSendMessage = (flags & 0x10000) > 0;

    mTerminateProcessOnPreviousNeighborRequestProcessingStage = (flags & 0x200000) > 0;
    mTerminateProcessOnCoordinatorRequestProcessingStage = (flags & 0x400000) > 0;
    mTerminateProcessOnNextNeighborResponseProcessingStage = (flags & 0x800000) > 0;
    mTerminateProcessOnVoteStage = (flags & 0x1000000) > 0;
    mTerminateProcessOnVoteConsistencyStage = (flags & 0x2000000) > 0;
    mTerminateProcessOnCoordinatorAfterApproveBeforeSendMessage = (flags & 0x4000000) > 0;

    mSleepOnPreviousNeighborRequestProcessingStage = (flags & 0x80000000) > 0;
    mSleepOnCoordinatorRequestProcessingStage = (flags & 0x100000000) > 0;
    mSleepOnNextNeighborResponseProcessingStage = (flags & 0x200000000) > 0;
    mSleepOnFinalAmountClarificationStage = (flags & 0x400000000) > 0;
    mSleepOnVoteStage = (flags & 0x800000000) > 0;
    mSleepOnVoteConsistencyStage = (flags & 0x1000000000) > 0;

    mIsWriteVisualResults = (flags & 0x10000000000) != 0;
    mIsRunPaymentTransactions = (flags & 0x20000000000) == 0;
    mIsRunTrustLineTransactions = (flags & 0x40000000000) == 0;

    debug() << "network on " << mIsNetworkOn;
    debug() << "payment transactions " << mIsRunPaymentTransactions;
    debug() << "trust line transactions " << mIsRunTrustLineTransactions;
    debug() << "close cycles " << mIsRunCycleClosingTransactions;
    debug() << "write visual results " << mIsWriteVisualResults;
}

void SubsystemsController::setForbiddenNodeUUID(
    const NodeUUID &nodeUUID)
{
    mForbiddenNodeUUID = nodeUUID;
    debug() << "setForbiddenNodeUUID " << mForbiddenNodeUUID;
}

void SubsystemsController::setForbiddenAmount(
    const TrustLineAmount &forbiddenAmount)
{
    mForbiddenAmount = forbiddenAmount;
    debug() << "setForbiddenAmount " << mForbiddenAmount;
}

bool SubsystemsController::isNetworkOn()
{
    if (mCountForbiddenMessages > 0) {
        mCountForbiddenMessages--;
        if (mCountForbiddenMessages == 0) {
            mIsNetworkOn = true;
            debug() << "Network switched ON";
        }
        return false;
    }
    return true;
}

bool SubsystemsController::isRunPaymentTransactions() const
{
    return mIsRunPaymentTransactions;
}

bool SubsystemsController::isRunTrustLineTransactions() const
{
    return mIsRunTrustLineTransactions;
}

bool SubsystemsController::isRunCycleClosingTransactions() const
{
    return mIsRunCycleClosingTransactions;
}

bool SubsystemsController::isWriteVisualResults() const
{
    return mIsWriteVisualResults;
}

void SubsystemsController::turnOffNetwork()
{
    mCountForbiddenMessages = UINT32_MAX;
    mIsNetworkOn = false;
    debug() << "Network switched OFF";
}

void SubsystemsController::turnOnNetwork()
{
    mCountForbiddenMessages = 0;
    mIsNetworkOn = true;
    debug() << "Network switched ON";
}

void SubsystemsController::testForbidSendMessageToReceiverOnReservationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageToReceiverOnReservationStage) {
        debug() << "ForbidSendMessageToReceiverOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testForbidSendMessageToCoordinatorOnReservationStage(
    const NodeUUID &previousNodeUUID,
    const TrustLineAmount &forbiddenAmount,
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageToCoordinatorOnReservationStage) {
        if (mForbiddenNodeUUID == NodeUUID::empty() || previousNodeUUID == NodeUUID::empty()) {
            debug() << "ForbidSendMessageToCoordinatorOnReservationStage";
            mCountForbiddenMessages = countForbiddenMessages;
        } else if (mForbiddenNodeUUID == previousNodeUUID) {
            if (mForbiddenAmount == 0) {
                debug() << "ForbidSendMessageToCoordinatorOnReservationStage previous node " << previousNodeUUID;
                mCountForbiddenMessages = countForbiddenMessages;
            } else if (mForbiddenAmount == forbiddenAmount) {
                debug() << "ForbidSendMessageToCoordinatorOnReservationStage previous node " << previousNodeUUID;
                mCountForbiddenMessages = countForbiddenMessages;
            }
        }
    }
}

void SubsystemsController::testForbidSendRequestToIntNodeOnReservationStage(
    const NodeUUID &receiverMessageNode,
    const TrustLineAmount &forbiddenAmount,
    uint32_t countForbiddenMessages)
{
    if (mForbidSendRequestToIntNodeOnReservationStage) {
        if (mForbiddenNodeUUID == NodeUUID::empty()) {
            debug() << "ForbidSendRequestToIntNodeOnReservationStage";
            mCountForbiddenMessages = countForbiddenMessages;
        } else if (mForbiddenNodeUUID == receiverMessageNode) {
            if (mForbiddenAmount == 0) {
                debug() << "ForbidSendRequestToIntNodeOnReservationStage to " << receiverMessageNode;
                mCountForbiddenMessages = countForbiddenMessages;
            } else if (mForbiddenAmount == forbiddenAmount) {
                debug() << "ForbidSendRequestToIntNodeOnReservationStage to " << receiverMessageNode;
                mCountForbiddenMessages = countForbiddenMessages;
            }
        }
    }
}

void SubsystemsController::testForbidSendResponseToIntNodeOnReservationStage(
    const NodeUUID &receiverMessageNode,
    const TrustLineAmount &forbiddenAmount,
    uint32_t countForbiddenMessages)
{
    if (mForbidSendResponseToIntNodeOnReservationStage) {
        if (mForbiddenNodeUUID == NodeUUID::empty()) {
            debug() << "ForbidSendResponseToIntNodeOnReservationStage";
            mCountForbiddenMessages = countForbiddenMessages;
        } else if (mForbiddenNodeUUID == receiverMessageNode) {
            if (mForbiddenAmount == 0) {
                debug() << "ForbidSendResponseToIntNodeOnReservationStage to " << receiverMessageNode;
                mCountForbiddenMessages = countForbiddenMessages;
            } else if (mForbiddenAmount == forbiddenAmount) {
                debug() << "ForbidSendResponseToIntNodeOnReservationStage to " << receiverMessageNode;
                mCountForbiddenMessages = countForbiddenMessages;
            }
        }
    }
}

void SubsystemsController::testForbidSendMessageWithFinalPathConfiguration(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageWithFinalPathConfiguration) {
        debug() << "ForbidSendMessageWithFinalPathConfiguration";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testForbidSendMessageOnFinalAmountClarificationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageOnFinalAmountClarificationStage) {
        debug() << "ForbidSendMessageOnFinalAmountClarificationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testForbidSendMessageToNextNodeOnVoteStage(
        uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageToNextNodeOnVoteStage) {
        debug() << "ForbidSendMessageToNextNodeOnVoteStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testForbidSendMessageToCoordinatorOnVoteStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageToCoordinatorOnVoteStage) {
        debug() << "ForbidSendMessageToCoordinatorOnVoteStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testForbidSendMessageOnVoteConsistencyStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageOnVoteConsistencyStage) {
        debug() << "ForbidSendMessageOnVoteConsistencyStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testThrowExceptionOnPreviousNeighborRequestProcessingStage()
{
    if (mThrowExceptionOnPreviousNeighborRequestProcessingStage) {
        throw Exception("Test exception on previous neighbor request processing stage");
    }
}

void SubsystemsController::testThrowExceptionOnCoordinatorRequestProcessingStage()
{
    if (mThrowExceptionOnCoordinatorRequestProcessingStage) {
        throw Exception("Test exception on coordinator request processing stage");
    }
}

void SubsystemsController::testThrowExceptionOnNextNeighborResponseProcessingStage()
{
    if (mThrowExceptionOnNextNeighborResponseProcessingStage) {
        throw Exception("Test exception on next neighbor response processing stage");
    }
}

void SubsystemsController::testThrowExceptionOnVoteStage()
{
    if (mThrowExceptionOnVoteStage) {
        throw Exception("Test exception on vote stage");
    }
}

void SubsystemsController::testThrowExceptionOnVoteConsistencyStage()
{
    if (mThrowExceptionOnVoteConsistencyStage) {
        throw Exception("Test exception on vote consistency stage");
    }
}

void SubsystemsController::testThrowExceptionOnCoordinatorAfterApproveBeforeSendMessage()
{
    if (mThrowExceptionOnCoordinatorAfterApproveBeforeSendMessage) {
        throw Exception("Test exception on coordinator after approve before send message");
    }
}

void SubsystemsController::testTerminateProcessOnPreviousNeighborRequestProcessingStage()
{
    if (mTerminateProcessOnPreviousNeighborRequestProcessingStage) {
        debug() << "testTerminateProcessOnPreviousNeighborRequestProcessingStage";
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnCoordinatorRequestProcessingStage()
{
    if (mTerminateProcessOnCoordinatorRequestProcessingStage) {
        debug() << "testTerminateProcessOnCoordinatorRequestProcessingStage";
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnNextNeighborResponseProcessingStage()
{
    if (mTerminateProcessOnNextNeighborResponseProcessingStage) {
        debug() << "testTerminateProcessOnNextNeighborResponseProcessingStage";
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnVoteStage()
{
    if (mTerminateProcessOnVoteStage) {
        debug() << "testTerminateProcessOnVoteStage";
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnVoteConsistencyStage()
{
    if (mTerminateProcessOnVoteConsistencyStage) {
        debug() << "testTerminateProcessOnVoteConsistencyStage";
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnCoordinatorAfterApproveBeforeSendMessage()
{
    if (mTerminateProcessOnCoordinatorAfterApproveBeforeSendMessage) {
        debug() << "testTerminateProcessOnCoordinatorAfterApproveBeforeSendMessage";
        exit(100);
    }
}

void SubsystemsController::testSleepOnPreviousNeighborRequestProcessingStage(
    uint32_t millisecondsDelay)
{
    if (mSleepOnPreviousNeighborRequestProcessingStage) {
        debug() << "testSleepOnPreviousNeighborRequestProcessingStage";
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsDelay));
    }
}

void SubsystemsController::testSleepOnCoordinatorRequestProcessingStage(
    uint32_t millisecondsDelay)
{
    if (mSleepOnCoordinatorRequestProcessingStage) {
        debug() << "testSleepOnCoordinatorRequestProcessingStage";
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsDelay));
    }
}

void SubsystemsController::testSleepOnNextNeighborResponseProcessingStage(
    uint32_t millisecondsDelay)
{
    if (mSleepOnNextNeighborResponseProcessingStage) {
        debug() << "testSleepOnNextNeighborResponseProcessingStage";
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsDelay));
    }
}

void SubsystemsController::testSleepOnFinalAmountClarificationStage(
    uint32_t millisecondsDelay)
{
    if (mSleepOnFinalAmountClarificationStage) {
        debug() << "testSleepOnFinalAmountClarificationStage";
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsDelay));
    }
}

void SubsystemsController::testSleepOnOnVoteStage(
    uint32_t millisecondsDelay)
{
    if (mSleepOnVoteStage) {
        debug() << "testSleepOnOnVoteStage";
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsDelay));
    }
}

void SubsystemsController::testSleepOnVoteConsistencyStage(
    uint32_t millisecondsDelay)
{
    if (mSleepOnVoteConsistencyStage) {
        debug() << "testSleepOnVoteConsistencyStage";
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsDelay));
    }
}

LoggerStream SubsystemsController::info() const
{
    return mLog.info(logHeader());
}

LoggerStream SubsystemsController::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream SubsystemsController::warning() const
{
    return mLog.warning(logHeader());
}

const string SubsystemsController::logHeader() const
{
    stringstream s;
    s << "[SubsystemsController]";
    return s.str();
}