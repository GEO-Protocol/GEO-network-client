#include "EventsInterface.h"

EventsInterface::EventsInterface(
    Logger &logger) :
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
        mFIFODescriptor = open(
            FIFOFilePath().c_str(),
            O_WRONLY | O_RSYNC | O_DSYNC | F_SETFL | O_NONBLOCK);
#endif

        if (mFIFODescriptor == -1) {
            throw IOError("EventsInterface::writeEvent: Can't open FIFO file.");
        }
    }
    sleep(2);
    if (write(mFIFODescriptor, event->data().get(), event->dataSize()) != event->dataSize()) {
        close(mFIFODescriptor);

#ifdef LINUX
        mFIFODescriptor = open(
            FIFOFilePath().c_str(),
            O_WRONLY | O_RSYNC | O_DSYNC | F_SETFL | O_NONBLOCK);
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
    return kFIFOName;
}
