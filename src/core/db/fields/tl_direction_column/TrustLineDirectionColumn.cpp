#include "TrustLineDirectionColumn.h"

db::fields::TrustLineDirectionColumn::TrustLineDirectionColumn(const char *filename, const char *path)
    : AbstractFileDescriptorHandler(path, filename) {

}

void db::fields::TrustLineDirectionColumn::set(const db::AbstractRecordsHandler::RecordNumber recN,
                                               const db::fields::TrustLineDirectionColumn::Direction direction) {

}

bool db::fields::TrustLineDirectionColumn::remove(const db::AbstractRecordsHandler::RecordNumber recN) {
    return false;
}

const db::fields::TrustLineDirectionColumn::Direction
db::fields::TrustLineDirectionColumn::direction(const db::AbstractRecordsHandler::RecordNumber recN) {
    return Incoming;
}
