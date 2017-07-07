#ifndef ABSTRACTMIGRATION_H
#define ABSTRACTMIGRATION_H

#include "../IOTransaction.h"


class AbstractMigration {
public:
    virtual ~AbstractMigration(){};

    virtual void apply(
        IOTransaction::Shared ioTransaction) = 0;
};

#endif // ABSTRACTMIGRATION_H
