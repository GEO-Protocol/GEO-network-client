#ifndef GEO_NETWORK_CLIENT_TESTINGCONTROLLER_H
#define GEO_NETWORK_CLIENT_TESTINGCONTROLLER_H


#include "../logger/Logger.h"

class TestingController {
public:
    TestingController(
        Logger &log);

public:
    void setFlags(size_t flags);

    const bool isNetworkOn() const;

    void turnOffNetwork();

    void turnOnNetwork();

    void testForbidSendMessageToCoordinatorOnReservationStage();

    void testForbidSendMessageToIntNodeOnReservationStage();

    void testForbidSendMessageOnVoteStage();

    void testForbidSendMessageOnVoteConsistencyStage();

    void testThrowExceptionOnPreviousNeighborRequestProcessingStage();

    void testThrowExceptionOnCoordinatorRequestProcessingStage();

    void testThrowExceptionOnNextNeighborResponseProcessingStage();

    void testThrowExceptionOnVoteStage();

    void testThrowExceptionOnVoteConsistencyStage();

    void testTerminateProcessOnPreviousNeighborRequestProcessingStage();

    void testTerminateProcessOnCoordinatorRequestProcessingStage();

    void testTerminateProcessOnNextNeighborResponseProcessingStage();

    void testTerminateProcessOnVoteStage();

    void testTerminateProcessOnVoteConsistencyStage();

protected:
    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    bool mIsNetworkOn;

    bool mForbidSendMessageToCoordinatorOnReservationStage;
    bool mForbidSendMessageToIntNodeOnReservationStage;
    bool mForbidSendMessageOnVoteStage;
    bool mForbidSendMessageOnVoteConsistencyStage;

    bool mThrowExceptionOnPreviousNeighborRequestProcessingStage;
    bool mThrowExceptionOnCoordinatorRequestProcessingStage;
    bool mThrowExceptionOnNextNeighborResponseProcessingStage;
    bool mThrowExceptionOnVoteStage;
    bool mThrowExceptionOnVoteConsistencyStage;

    bool mTerminateProcessOnPreviousNeighborRequestProcessingStage;
    bool mTerminateProcessOnCoordinatorRequestProcessingStage;
    bool mTerminateProcessOnNextNeighborResponseProcessingStage;
    bool mTerminateProcessOnVoteStage;
    bool mTerminateProcessOnVoteConsistencyStage;

    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TESTINGCONTROLLER_H
