#include "TestingController.h"

TestingController::TestingController(
    Logger &log):
    mLog(log)
{
    mIsNetworkOn = true;
    mForbidSendMessageToCoordinatorOnReservationStage = false;
    mForbidSendMessageToIntNodeOnReservationStage = false;
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
}

void TestingController::setFlags(size_t flags)
{
    debug() << "setFlags: " << flags;
    mIsNetworkOn = (flags & 0x1) == 0;
    mForbidSendMessageToCoordinatorOnReservationStage = (flags & 0x2) > 0;
    mForbidSendMessageToIntNodeOnReservationStage = (flags & 0x4) > 0;
    mForbidSendMessageOnVoteStage = (flags & 0x8) > 0;
    mForbidSendMessageOnVoteConsistencyStage = (flags & 0x10) > 0;

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
    debug() << "nerwork on " << mIsNetworkOn;
}

const bool TestingController::isNetworkOn() const
{
    return mIsNetworkOn;
}

void TestingController::turnOffNetwork()
{
    mIsNetworkOn = false;
    debug() << "Network switched OFF";
}

void TestingController::turnOnNetwork()
{
    mIsNetworkOn = true;
    debug() << "Network switched ON";
}

void TestingController::testForbidSendMessageToCoordinatorOnReservationStage()
{
    if (mForbidSendMessageToCoordinatorOnReservationStage) {
        debug() << "ForbidSendMessageToCoordinatorOnReservationStage";
        turnOffNetwork();
    }
}

void TestingController::testForbidSendMessageToIntNodeOnReservationStage()
{
    if (mForbidSendMessageToIntNodeOnReservationStage) {
        debug() << "ForbidSendMessageToIntNodeOnReservationStage";
        turnOffNetwork();
    }
}

void TestingController::testForbidSendMessageOnVoteStage()
{
    if (mForbidSendMessageOnVoteStage) {
        debug() << "ForbidSendMessageOnVoteStage";
        turnOffNetwork();
    }
}

void TestingController::testForbidSendMessageOnVoteConsistencyStage()
{
    if (mForbidSendMessageOnVoteConsistencyStage) {
        debug() << "ForbidSendMessageOnVoteConsistencyStage";
        turnOffNetwork();
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
    if (mTerminateProcessOnPreviousNeighborRequestProcessingStage) {
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