//
// Created by minyor on 14.03.19.
//

#include "TailManager.h"

TailManager::TailManager(
    Logger &logger):
    mLog(logger) {
}

TailManager::~TailManager() {
}

TailManager::Tail &TailManager::getTail(Message::MessageType type) {
    return mTails[type];
}
