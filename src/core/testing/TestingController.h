#ifndef GEO_NETWORK_CLIENT_TESTINGCONTROLLER_H
#define GEO_NETWORK_CLIENT_TESTINGCONTROLLER_H


#include "../logger/Logger.h"

class TestingController {
public:
    TestingController(
        Logger &log);

public:
    void setFlags(size_t flags);

    bool isNetworkOn() const;

    bool isCloseCycles() const;

    void turnOffNetwork();

    void turnOnNetwork();

    void testForbidSendMessageToCoordinatorOnReservationStage();

    void testForbidSendRequestToIntNodeOnReservationStage();

    void testForbidSendResponseToIntNodeOnReservationStage();

    void testForbidSendMessageOnFinalAmountClarificationStage();

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
    bool mIsCloseCycles;

    bool mForbidSendMessageToCoordinatorOnReservationStage;
    bool mForbidSendRequestToIntNodeOnReservationStage;
    bool mForbidSendResponseToIntNodeOnReservationStage;
    bool mForbidSendMessageOnFinalAmountClarificationStage;
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
