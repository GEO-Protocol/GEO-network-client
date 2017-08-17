#ifndef GEO_NETWORK_CLIENT_SUBSYSTEMSCONTROLLER_H
#define GEO_NETWORK_CLIENT_SUBSYSTEMSCONTROLLER_H


#include "../logger/Logger.h"

class SubsystemsController {
public:
    SubsystemsController(
        Logger &log);

public:
    void setFlags(size_t flags);

    void setForbiddenNodeUUID(
        const NodeUUID &nodeUUID);

    bool isNetworkOn();

    bool isCloseCycles() const;

    void turnOffNetwork();

    void turnOnNetwork();

    void testForbidSendMessageToReceiverOnReservationStage(
        uint32_t countForbiddenMessages = 1);

    void testForbidSendMessageToCoordinatorOnReservationStage(
        const NodeUUID &previousNodeUUID,
        uint32_t countForbiddenMessages = 1);

    void testForbidSendRequestToIntNodeOnReservationStage(
        const NodeUUID &receiverMessageNode,
        uint32_t countForbiddenMessages = 1);

    void testForbidSendResponseToIntNodeOnReservationStage(
        const NodeUUID &receiverMessageNode,
        uint32_t countForbiddenMessages = 1);

    void testForbidSendMessageWithFinalPathConfiguration(
        uint32_t countForbiddenMessages = 1);

    void testForbidSendMessageOnFinalAmountClarificationStage(
        uint32_t countForbiddenMessages = 1);

    void testForbidSendMessageToNextNodeOnVoteStage(
        uint32_t countForbiddenMessages = 1);

    void testForbidSendMessageToCoordinatorOnVoteStage(
        uint32_t countForbiddenMessages = 1);

    void testForbidSendMessageOnVoteConsistencyStage(
        uint32_t countForbiddenMessages = 1);

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
    uint32_t mCountForbiddenMessages;

    bool mIsCloseCycles;

    bool mForbidSendMessageToReceiverOnReservationStage;
    bool mForbidSendMessageToCoordinatorOnReservationStage;
    bool mForbidSendRequestToIntNodeOnReservationStage;
    bool mForbidSendResponseToIntNodeOnReservationStage;
    bool mForbidSendMessageWithFinalPathConfiguration;
    bool mForbidSendMessageOnFinalAmountClarificationStage;
    bool mForbidSendMessageToNextNodeOnVoteStage;
    bool mForbidSendMessageToCoordinatorOnVoteStage;
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

    NodeUUID mForbiddenNodeUUID;

    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_SUBSYSTEMSCONTROLLER_H
