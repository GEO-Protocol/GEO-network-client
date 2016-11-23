#include "Command.h"

Command::Command(const string &identifier):
    mIdentifier(identifier){}

const string& Command::identifier() const {
    return mIdentifier;
}