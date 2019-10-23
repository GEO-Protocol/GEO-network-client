#include "EventsInterface.h"

EventsInterface::EventsInterface(
    string fifoName,
    bool isBlocked,
    Logger &logger) :
    mFIFOName(fifoName),
    mIsBlocked(isBlocked),
    mLogger(logger)
{
    if (!isFIFOExists()) {
        createFIFO(kPermissionsMask);
    }
    mFIFODescriptor = 0;
    signal(SIGPIPE, SIG_IGN);
}

EventsInterface::~EventsInterface()
{
    if (mFIFODescriptor != 0) {
        close(mFIFODescriptor);
    }
    if (remove(FIFOFilePath().c_str()) != 0) {
        mLogger.warning("events.fifo didn't delete");
    }
}

void EventsInterface::writeEvent(
    Event::Shared event)
{
    if (mFIFODescriptor == 0){
#ifdef LINUX
        if (mIsBlocked) {
            mFIFODescriptor = open(
                FIFOFilePath().c_str(),
                O_WRONLY | O_RSYNC | O_DSYNC | F_SETFL);
        } else {
            mFIFODescriptor = open(
                FIFOFilePath().c_str(),
                O_WRONLY | O_RSYNC | O_DSYNC | F_SETFL | O_NONBLOCK);
        }
#endif

        if (mFIFODescriptor == -1) {
            throw IOError("EventsInterface::writeEvent: Can't open FIFO file.");
        }
    }
    if (write(mFIFODescriptor, event->data().get(), event->dataSize()) != event->dataSize()) {
        close(mFIFODescriptor);

#ifdef LINUX
        if (mIsBlocked) {
            mFIFODescriptor = open(
                FIFOFilePath().c_str(),
                O_WRONLY | O_RSYNC | O_DSYNC | F_SETFL);
        } else {
            mFIFODescriptor = open(
                FIFOFilePath().c_str(),
                O_WRONLY | O_RSYNC | O_DSYNC | F_SETFL | O_NONBLOCK);
        }
#endif

        if (mFIFODescriptor == -1) {
            throw IOError("EventsInterface::writeEvent: Can't open FIFO file.");
        }

        if (write(mFIFODescriptor, event->data().get(), event->dataSize()) != event->dataSize()) {
            throw IOError("EventsInterface::writeEvent: Can't write result to the disk.");
        }
    }
}

const char *EventsInterface::FIFOName() const
{
    return mFIFOName.c_str();
}
