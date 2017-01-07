#include "TrustLineDirectionColumn.h"


namespace db {
namespace fields {


TrustLineDirectionColumn::TrustLineDirectionColumn(
    const fs::path &path):
    AbstractFileDescriptorHandler(path / fs::path("data.bin")) {

    open();
}


void TrustLineDirectionColumn::set(
    const db::AbstractRecordsHandler::RecordNumber recN,
    const TrustLineDirection direction,
    const bool sync) {

    SerializedTrustLineDirection serializableDirection(direction);
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1 &&
        fwrite(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::set: can't write record.");
    }

    if (sync) {
        syncLowLevelOSBuffers();
    }
}

void TrustLineDirectionColumn::remove(
    const db::AbstractRecordsHandler::RecordNumber recN,
    const bool sync) {

    const auto removedRecordValue = kRemovedRecordValue;
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fwrite(&removedRecordValue, sizeof(removedRecordValue), 1, mFileDescriptor) != 1 &&
        fwrite(&removedRecordValue, sizeof(removedRecordValue), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::set: can't remove record.");
    }

    if (sync) {
        syncLowLevelOSBuffers();
    }
}

/*!
 * @param recN - number of the record that should be returned.
 * @return direction of the trust line;
 *
 * Throws
 */
const TrustLineDirection TrustLineDirectionColumn::direction(
    const db::AbstractRecordsHandler::RecordNumber recN) const {

    SerializedTrustLineDirection serializableDirection;
    fseek(mFileDescriptor, recordOffset(recN), SEEK_SET);
    if (fread(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1 &&
        fread(&serializableDirection, sizeof(serializableDirection), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::set: can't read record.");
    }

    if (serializableDirection == kRemovedRecordValue) {
        throw NotFoundError(
            "TrustLineDirectionColumn::direction: no record associated with recN");
    } else {
        return TrustLineDirection(serializableDirection);
    }

}



TrustLineDirectionColumn::FileHeader::FileHeader():
    version(1){}


void TrustLineDirectionColumn::open() {
    AbstractFileDescriptorHandler::open();

    if (fileSize() == 0) {
        // Init default header.
        FileHeader initialHeader; // will be initialised to the defaults by the constructor.
        updateFileHeader(&initialHeader);

    } else {
        // Load data from current header.
        auto fileHeader = loadFileHeader();

        // Checking of received file version.
        // Files with unexpected file version should be rejected.
        if (fileHeader.version != 1) {
            throw ValueError(
                "TrustLineDirectionColumn::open: "
                    "unexpected file version occurred.");
        }
    }
}

TrustLineDirectionColumn::FileHeader TrustLineDirectionColumn::loadFileHeader() const {
    FileHeader header;
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fread(&header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::loadFileHeader: "
                "can't load file header.");
    }
    return header;
}
void TrustLineDirectionColumn::updateFileHeader(
    const TrustLineDirectionColumn::FileHeader *header) const {

    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fwrite(header, sizeof(FileHeader), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TrustLineDirectionColumn::updateFileHeader: "
                "can't write header to the disk.");
    }
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "TrustLineDirectionColumn::updateFileHeader: "
                "can't sync buffers with the disk.");

    }
}

const TrustLineDirectionColumn::IndexRecordOffset
TrustLineDirectionColumn::recordOffset(const AbstractRecordsHandler::RecordNumber recN) const {
    return (recN * sizeof(SerializedTrustLineDirection)) + sizeof(FileHeader);
}


}
}

