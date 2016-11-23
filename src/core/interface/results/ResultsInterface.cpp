#include "ResultsInterface.h"

void ResultsInterface::createFifo() {
    if (! FIFOExists()) {
        fs::create_directories(dir());
        mkfifo(FIFOPath().c_str(), 0420);
    }
}

