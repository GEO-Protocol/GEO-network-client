#include "TailManager.h"

TailManager::Tail::Tail(TailManager &manager) :
    mManager(manager)
{
    mManager.mTails.push_back(this);
}

TailManager::TailManager(
    as::io_service &ioService,
    Logger &logger):
    mIOService(ioService),
    mLog(logger),
    mTails(),
    mFlowTail(*this),
    mCyclesFiveTail(*this),
    mCyclesSixTail(*this),
    mRoutingTableTail(*this)
{
    mUpdatingTimer = make_unique<as::steady_timer>(
        mIOService);
    mUpdatingTimer->expires_from_now(
        std::chrono::seconds(
            kUpdatingTimerPeriodSeconds));
    mUpdatingTimer->async_wait(
        boost::bind(
            &TailManager::update,
            this,
            as::placeholders::error));
}

TailManager::~TailManager()
{}

void TailManager::update(const boost::system::error_code &err)
{
    if (err) {
        warning() << err.message();
    }
    mUpdatingTimer->cancel();
    mUpdatingTimer->expires_from_now(
        std::chrono::seconds(
            kUpdatingTimerPeriodSeconds));
    mUpdatingTimer->async_wait(
        boost::bind(
            &TailManager::update,
            this,
            as::placeholders::error));

    DateTime now = utc_now();

    for(auto lit = mTails.begin(); lit != mTails.end(); ++lit) {
        Tail &tail = **lit;

        for(auto current = tail.end(); current != tail.begin(); ) {
            --current;
            Msg &msg = *current;
            if(!msg.time.is_not_a_date_time()) {
                break;
            }
            msg.time = now;
        }

        for(auto current=tail.begin(); current!=tail.end(); ) {
            Msg &msg = *current;
            if (!msg.time.is_not_a_date_time() and (now - msg.time) <= kCleanDuration()) {
                break;
            }
            current = tail.erase(current);
        }
    }
}

LoggerStream TailManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TailManager::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream TailManager::warning() const
{
    return mLog.warning(logHeader());
}

const string TailManager::logHeader() const
{
    stringstream s;
    s << "[TailManager] ";
    return s.str();
}
