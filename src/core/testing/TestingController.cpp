#include "TestingController.h"

TestingController::TestingController(
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

void TestingController::setFlags(size_t flags)
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

bool TestingController::isNetworkOn()
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

bool TestingController::isCloseCycles() const
{
    return mIsCloseCycles;
}

void TestingController::turnOffNetwork()
{
    mCountForbiddenMessages = UINT32_MAX;
    mIsNetworkOn = false;
    debug() << "Network switched OFF";
}

void TestingController::turnOnNetwork()
{
    mCountForbiddenMessages = 0;
    mIsNetworkOn = true;
    debug() << "Network switched ON";
}

void TestingController::testForbidSendMessageToReceiverOnReservationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageToReceiverOnReservationStage) {
        debug() << "ForbidSendMessageToReceiverOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testForbidSendMessageToCoordinatorOnReservationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageToCoordinatorOnReservationStage) {
        debug() << "ForbidSendMessageToCoordinatorOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testForbidSendRequestToIntNodeOnReservationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendRequestToIntNodeOnReservationStage) {
        debug() << "ForbidSendRequestToIntNodeOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testForbidSendResponseToIntNodeOnReservationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendResponseToIntNodeOnReservationStage) {
        debug() << "ForbidSendResponseToIntNodeOnReservationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testForbidSendMessageWithFinalPathConfiguration(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageWithFinalPathConfiguration) {
        debug() << "ForbidSendMessageWithFinalPathConfiguration";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testForbidSendMessageOnFinalAmountClarificationStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageOnFinalAmountClarificationStage) {
        debug() << "ForbidSendMessageOnFinalAmountClarificationStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testForbidSendMessageOnVoteStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageOnVoteStage) {
        debug() << "ForbidSendMessageOnVoteStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testForbidSendMessageOnVoteConsistencyStage(
    uint32_t countForbiddenMessages)
{
    if (mForbidSendMessageOnVoteConsistencyStage) {
        debug() << "ForbidSendMessageOnVoteConsistencyStage";
        mCountForbiddenMessages = countForbiddenMessages;
    }
}

void TestingController::testThrowExceptionOnPreviousNeighborRequestProcessingStage()
{
    if (mThrowExceptionOnPreviousNeighborRequestProcessingStage) {
        throw Exception("Test exception on previous neighbor request processing stage");
    }
}

void TestingController::testThrowExceptionOnCoordinatorRequestProcessingStage()
{
    if (mThrowExceptionOnCoordinatorRequestProcessingStage) {
        throw Exception("Test exception on coordinator request processing stage");
    }
}

void TestingController::testThrowExceptionOnNextNeighborResponseProcessingStage()
{
    if (mThrowExceptionOnNextNeighborResponseProcessingStage) {
        throw Exception("Test exception on next neighbor response processing stage");
    }
}

void TestingController::testThrowExceptionOnVoteStage()
{
    if (mThrowExceptionOnVoteStage) {
        throw Exception("Test exception on vote stage");
    }
}

void TestingController::testThrowExceptionOnVoteConsistencyStage()
{
    if (mThrowExceptionOnVoteConsistencyStage) {
        throw Exception("Test exception on vote consistency stage");
    }
}

void TestingController::testTerminateProcessOnPreviousNeighborRequestProcessingStage()
{
    if (mTerminateProcessOnPreviousNeighborRequestProcessingStage) {
        exit(100);
    }
}

void TestingController::testTerminateProcessOnCoordinatorRequestProcessingStage()
{
    if (mTerminateProcessOnCoordinatorRequestProcessingStage) {
        exit(100);
    }
}

void TestingController::testTerminateProcessOnNextNeighborResponseProcessingStage()
{
    if (mTerminateProcessOnNextNeighborResponseProcessingStage) {
        exit(100);
    }
}

void TestingController::testTerminateProcessOnVoteStage()
{
    if (mTerminateProcessOnVoteStage) {
        exit(100);
    }
}

void TestingController::testTerminateProcessOnVoteConsistencyStage()
{
    if (mTerminateProcessOnVoteConsistencyStage) {
        exit(100);
    }
}

LoggerStream TestingController::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TestingController::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream TestingController::error() const
{
    return mLog.error(logHeader());
}

const string TestingController::logHeader() const
{
    stringstream s;
    s << "[TestingController]";
    return s.str();
}