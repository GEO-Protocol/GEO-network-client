#include "SubsystemsController.h"

SubsystemsController::SubsystemsController(
    Logger &log):
    mLog(log)
{
    mIsNetworkOn = true;
    mIsCloseCycles = true;

    mForbidSendMessageToReceiverOnReservationStage = false;
    mForbidSendMessageToCoordinatorOnReservationStage = false;
    mForbidSendRequestToIntNodeOnReservationStage = false;
    mForbidSendResponseToIntNodeOnReservationStage = false;
    mForbidSendMessageWithFinalPathConfiguration = false;
    mForbidSendMessageOnFinalAmountClarificationStage = false;
    mForbidSendMessageOnVoteStage = false;
    mForbidSendMessageOnVoteConsistencyStage = false;

    mThrowExceptionOnPreviousNeighborRequestProcessingStage = false;
    mThrowExceptionOnCoordinatorRequestProcessingStage = false;
    mThrowExceptionOnNextNeighborResponseProcessingStage = false;
    mThrowExceptionOnVoteStage = false;
    mThrowExceptionOnVoteConsistencyStage = false;

    mTerminateProcessOnPreviousNeighborRequestProcessingStage = false;
    mTerminateProcessOnCoordinatorRequestProcessingStage = false;
    mTerminateProcessOnNextNeighborResponseProcessingStage = false;
    mTerminateProcessOnVoteStage = false;
    mTerminateProcessOnVoteConsistencyStage = false;

    mCountForbiddenMessages = 0;
}

void SubsystemsController::setFlags(size_t flags)
{
    debug() << "setFlags: " << flags;
    mIsNetworkOn = (flags & 0x1) == 0;
    if (!mIsNetworkOn) {
        mCountForbiddenMessages = UINT32_MAX;
    }
    mIsCloseCycles = (flags & 0x2) == 0;

    mForbidSendMessageToReceiverOnReservationStage = (flags & 0x4) > 0;
    mForbidSendMessageToCoordinatorOnReservationStage = (flags & 0x8) > 0;
    mForbidSendRequestToIntNodeOnReservationStage = (flags & 0x10) > 0;
    mForbidSendResponseToIntNodeOnReservationStage = (flags & 0x20) > 0;
    mForbidSendMessageWithFinalPathConfiguration = (flags & 0x40) > 0;
    mForbidSendMessageOnFinalAmountClarificationStage = (flags & 0x80) > 0;
    mForbidSendMessageOnVoteStage = (flags & 0x100) > 0;
    mForbidSendMessageOnVoteConsistencyStage = (flags & 0x200) > 0;

    mThrowExceptionOnPreviousNeighborRequestProcessingStage = (flags & 0x800) > 0;
    mThrowExceptionOnCoordinatorRequestProcessingStage = (flags & 0x1000) > 0;
    mThrowExceptionOnNextNeighborResponseProcessingStage = (flags & 0x2000) > 0;
    mThrowExceptionOnVoteStage = (flags & 0x4000) > 0;
    mThrowExceptionOnVoteConsistencyStage = (flags & 0x8000) > 0;

    mTerminateProcessOnPreviousNeighborRequestProcessingStage = (flags & 0x200000) > 0;
    mTerminateProcessOnCoordinatorRequestProcessingStage = (flags & 0x400000) > 0;
    mTerminateProcessOnNextNeighborResponseProcessingStage = (flags & 0x800000) > 0;
    mTerminateProcessOnVoteStage = (flags & 0x1000000) > 0;
    mTerminateProcessOnVoteConsistencyStage = (flags & 0x2000000) > 0;
    debug() << "network on " << mIsNetworkOn;
    debug() << "close cycles " << mIsCloseCycles;
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

bool SubsystemsController::isCloseCycles() const
{
    return mIsCloseCycles;
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
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageToCoordinatorOnReservationStage) {
        debug() << "ForbidSendMessageToCoordinatorOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testForbidSendRequestToIntNodeOnReservationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendRequestToIntNodeOnReservationStage) {
        debug() << "ForbidSendRequestToIntNodeOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void SubsystemsController::testForbidSendResponseToIntNodeOnReservationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendResponseToIntNodeOnReservationStage) {
        debug() << "ForbidSendResponseToIntNodeOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
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

void SubsystemsController::testForbidSendMessageOnVoteStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageOnVoteStage) {
        debug() << "ForbidSendMessageOnVoteStage";
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

void SubsystemsController::testTerminateProcessOnPreviousNeighborRequestProcessingStage()
{
    if (mTerminateProcessOnPreviousNeighborRequestProcessingStage) {
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnCoordinatorRequestProcessingStage()
{
    if (mTerminateProcessOnCoordinatorRequestProcessingStage) {
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnNextNeighborResponseProcessingStage()
{
    if (mTerminateProcessOnNextNeighborResponseProcessingStage) {
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnVoteStage()
{
    if (mTerminateProcessOnVoteStage) {
        exit(100);
    }
}

void SubsystemsController::testTerminateProcessOnVoteConsistencyStage()
{
    if (mTerminateProcessOnVoteConsistencyStage) {
        exit(100);
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

LoggerStream SubsystemsController::error() const
{
    return mLog.error(logHeader());
}

const string SubsystemsController::logHeader() const
{
    stringstream s;
    s << "[SubsystemsController]";
    return s.str();
}