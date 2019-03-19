//
// Created by minyor on 14.03.19.
//

#ifndef GEO_NETWORK_CLIENT_TAILMANAGER_H
#define GEO_NETWORK_CLIENT_TAILMANAGER_H


#include <list>
#include <deque>
#include "../../../messages/Message.hpp"
#include "../../../../logger/Logger.h"

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace as = boost::asio;
namespace signals = boost::signals2;

class TailManager {
public:
    struct Msg : Message::Shared {
        Msg(const Message::Shared &ptr) : Message::Shared(ptr) {}
        DateTime time;
    };
    typedef std::deque<Msg> MsgList;

    struct Tail : MsgList {
        explicit Tail(TailManager &manager);
        TailManager &mManager;
    };
    typedef std::list<Tail *> TailList;

private:
    const uint32_t kUpdatingTimerPeriodSeconds = 1 * 60;

    static const byte kCleanHours = 0;
    static const byte kCleanMinutes = 5;
    static const byte kCleanSeconds = 0;

    static Duration &kCleanDuration() {
        static auto duration = Duration(
            kCleanHours,
            kCleanMinutes,
            kCleanSeconds);
        return duration;
    }

public:
    TailManager(
        as::io_service &ioService,
        Logger &logger
    );
    ~TailManager();

private:
    void update(const boost::system::error_code &err);

public:
    Tail &getFlowTail() { return mFlowTail; }
    Tail &getCyclesFiveTail() { return mCyclesFiveTail; }
    Tail &getCyclesSixTail() { return mCyclesSixTail; }
    Tail &getRoutingTableTail() { return mRoutingTableTail; }

private:
    LoggerStream info() const;
    LoggerStream debug() const;
    LoggerStream warning() const;
    const string logHeader() const;

private:
    Logger &mLog;
    TailList mTails;

    Tail mFlowTail;
    Tail mCyclesFiveTail;
    Tail mCyclesSixTail;
    Tail mRoutingTableTail;

    as::io_service &mIOService;
    unique_ptr<as::steady_timer> mUpdatingTimer;
};


#endif //GEO_NETWORK_CLIENT_TAILMANAGER_H
