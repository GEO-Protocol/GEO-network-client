#include <boost/uuid/uuid.hpp>
#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager() {

}

TrustLinesManager::~TrustLinesManager() {
    for (auto const& iterator : mTrustLines) {
        delete iterator.second;
    }
    mTrustLines.clear();
}

uint8_t *TrustLinesManager::serializeBytesFromTrustLineStruct(TrustLine *trustLine) {
    //Виділення 97 байт пам’яті під масив(буфер), в якому буде зберігатись сереалізований екземпляр стр-ри.
    uint8_t *buffer = (uint8_t *) malloc(BUCKET_SIZE);
    //Занулення байтів в буфері,
    memset(buffer, 0, BUCKET_SIZE);
    //Сереалізація значення лінії довіри до користувача в байти і запис в буфер.
    memcpy(buffer, trustAmountDataToBytes(trustLine->getIncomingTrustAmount()).data(), TRUST_AMOUNT_PART_SIZE);
    //Сереалізація значення лінії довіри від користувача в байти і запис в буфер.
    memcpy(buffer + TRUST_AMOUNT_PART_SIZE, trustAmountDataToBytes(trustLine->getOutgoingTrustAmount()).data(), TRUST_AMOUNT_PART_SIZE);
    //Сереалізація значення балансу користувача в байти і запис в буфер. Останній елемент масиву - знак балансу.
    memcpy(buffer + TRUST_AMOUNT_PART_SIZE * 2, balanceToBytes(trustLine->getBalance()).data(), BALANCE_PART_SIZE + SIGN_BYTE_PART_SIZE);
    return buffer;
}

vector<uint8_t> TrustLinesManager::trustAmountDataToBytes(trust_amount amount) {
    vector<uint8_t> byteSet;
    //Конвертація cpp_int шаблону в байти, результат записується у вектор.
    export_bits(static_cast<boost::multiprecision::checked_uint256_t>(amount), back_inserter(byteSet), 8);
    //Отримання розміру вектора.
    size_t filledPlace = byteSet.size();
    //Якщо значення типу cpp_int не використовує весь виділений для нього простір в пам’яті,
    //після конвертації в байти, у вектор будуть записана лише та к-сть байтів, яка являє собою значення, нульові байти відкидаються.
    //Тому, для того, щоб зберегти шаблон по якому буде визначатись зміщення, у вектор додаються нульові байти, до тих пір,
    //поки весь простір, який виділений під тип даних не буде використаний.
    for (unsigned long i = 0; i < TRUST_AMOUNT_PART_SIZE - filledPlace; ++i) {
        byteSet.push_back(0);
    }
    return byteSet;
}

vector<uint8_t> TrustLinesManager::balanceToBytes(balance_value balance) {
    vector<uint8_t> byteSet;
    //Конвертація cpp_int шаблону в байти, результат записується у вектор.
    export_bits(static_cast<boost::multiprecision::int256_t>(balance), back_inserter(byteSet), 8, true);
    //Отримання розміру вектора.
    size_t filledPlace = byteSet.size();
    //Якщо значення типу cpp_int не використовує весь виділений для нього простір в пам’яті,
    //після конвертації в байти, у вектор будуть записана лише та к-сть байтів, яка являє собою значення, нульові байти відкидаються.
    //Тому, для того, щоб зберегти шаблон по якому буде визначатись зміщення, у вектор додаються нульові байти, до тих пір,
    //поки весь простір, який виділений під тип даних не буде використаний.
    for (unsigned long i = 0; i < BALANCE_PART_SIZE - filledPlace; ++i) {
        byteSet.push_back(0);
    }
    //Перевірка знаку:
    //якщо число негативне -
    if (balance.sign() == -1) {
        //в контрольний байт пишеться 1.
        byteSet.push_back(1);
    } else {
        //інакше, якщо значення позитивне - в контрольний байт пишеться 0
        byteSet.push_back(0);
    }
    return byteSet;
}

void TrustLinesManager::deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID) {
    //Десереалізаця байтового масиву в екземпляр структури.
    TrustLine trustLine(contractorUUID,
                        parseTrustAmountData(buffer),
                        parseTrustAmountData(buffer + TRUST_AMOUNT_PART_SIZE),
                        parseBalanceData(buffer + TRUST_AMOUNT_PART_SIZE * 2));
    //Додання готового екземпляру в динамічну пам’ять.
    mTrustLines.insert(pair<boost::uuids::uuid, TrustLine *>(contractorUUID, &trustLine));
}

trust_amount TrustLinesManager::parseTrustAmountData(uint8_t *buffer) {
    trust_amount amount;
    vector<uint8_t> bytesVector(TRUST_AMOUNT_PART_SIZE);
    vector<uint8_t> notZeroBytesVector;
    //Копіювання даних з буфера у вектор байтів. У векторі попередньо зарезервоване місце під к-сть елементів.
    copy(buffer, buffer + TRUST_AMOUNT_PART_SIZE, bytesVector.begin());
    //Отримання не нульових байтів і копіювання їх у вектор не нуьлових байтів
    for (unsigned long i = 0; i < bytesVector.size(); ++i) {
        if (bytesVector.at(i) != 0) {
            notZeroBytesVector.push_back(bytesVector.at(i));
        }
    }
    //Якщо значення не рівне нулю, тоді у векторі не нульових байтів будуть якісь дані -
    if (notZeroBytesVector.size() > 0) {
        //конвертація байтів в cpp_int шаблон з вектору не нульових байтів.
        import_bits(amount, notZeroBytesVector.begin(), notZeroBytesVector.end());
    } else if (notZeroBytesVector.size() == 0) {
        //конвертація байтів в cpp_int шаблон з вектору байтів.
        import_bits(amount, bytesVector.begin(), bytesVector.end());
    }
    return amount;
}

balance_value TrustLinesManager::parseBalanceData(uint8_t *buffer) {
    balance_value balance;
    vector<uint8_t> bytesVector(BALANCE_PART_SIZE);
    vector<uint8_t> notZeroBytesVector;
    //Отримання байту зі знаком значення.
    uint8_t sign = buffer[BALANCE_PART_SIZE + SIGN_BYTE_PART_SIZE - 1];
    //Копіювання даних з буфера у вектор байтів. У векторі попередньо зарезервоване місце під к-сть елементів.
    copy(buffer, buffer + BALANCE_PART_SIZE, bytesVector.begin());
    //Отримання не нульових байтів і копіювання їх у вектор не нуьлових байтів
    for (unsigned long i = 0; i < bytesVector.size(); ++i) {
        if (bytesVector.at(i) != 0) {
            notZeroBytesVector.push_back(bytesVector.at(i));
        }
    }
    //Якщо значення не рівне нулю, тоді у векторі не нульових байтів будуть якісь дані.
    if (notZeroBytesVector.size() > 0) {
        //Конвертація байтів в cpp_int шаблон з вектору не нульових байтів.
        import_bits(balance, notZeroBytesVector.begin(), notZeroBytesVector.end(), 8, true);
    } else if (notZeroBytesVector.size() == 0) {
        //Конвертація байтів в cpp_int шаблон з вектору байтів.
        import_bits(balance, bytesVector.begin(), bytesVector.end(), 8, true);
    }
    //Якщо байт знаку рівний одиниці -
    if (sign == 1) {
        //реверс значення у негативне.
        balance = balance * -1;
    }
    return balance;
}

bool TrustLinesManager::saveTrustLine(TrustLine *trustLine) {
    uint8_t *trustLineData = serializeBytesFromTrustLineStruct(trustLine);
    //TODO: write bytes data in file with StorageManager, check if operation proceed success and write trust line in ram
    //Contractor's uuid is present in trust line instance, but she's not serialized in bytes array
    /*
     if ((pStorageManager->write(trustLineData)){
         free(trustLineData);
         return mTrustLines.insert(pair<uuids::uuid, TrustLine *>(trustLine->getContractorNodeUUID(), trustLine)).second;
     } else {
         free(trustLineData);
         return false;
     }
     */
    free(trustLineData);
    if (isTrustLineExist(trustLine->getContractorNodeUUID())) {
        map<uuids::uuid, TrustLine *>::iterator it = mTrustLines.find(trustLine->getContractorNodeUUID());
        if (it != mTrustLines.end()){
            it->second = trustLine;
            return true;
        } else {
            return false;
        }
    } else {
        return mTrustLines.insert(pair<uuids::uuid, TrustLine *>(trustLine->getContractorNodeUUID(), trustLine)).second;
    }
}

bool TrustLinesManager::removeTrustLine(const uuids::uuid contractorUUID) {
    //TODO: remove data from storage manager, check if operation proceed success and remove trust line from ram
    //TODO: something like in saveTrustLine() method
    if (isTrustLineExist(contractorUUID)) {
        delete mTrustLines.at(contractorUUID);
        mTrustLines.erase(contractorUUID);
        return true;
    }else{
        return false;
    }
}

bool TrustLinesManager::isTrustLineExist(const uuids::uuid contractorUUID) {
    return mTrustLines.count(contractorUUID) > 0;
}

bool TrustLinesManager::open(const uuids::uuid contractorUUID, const trust_amount amount) {
    //Якщо лінія довіри існує
    if (isTrustLineExist(contractorUUID)) {
        //Отримання вказівник на екземпляр
        TrustLine *trustLine = mTrustLines.at(contractorUUID);
        //Встановлення розміру вихідної лінії довіри
        trustLine->setOutgoingTrustAmount(amount);
        //Збереження лінії довіри
        return saveTrustLine(trustLine);
    } else {
        //Інакше, створення нового екзмепляру лінії довіри
        //Встановлення розміру вихідної лінії довіри
        TrustLine *trustLine = new TrustLine(contractorUUID, 0, amount, 0);
        //Збереження лінії довіри
        return saveTrustLine(trustLine);
    }
}

int TrustLinesManager::close(const uuids::uuid contractorUUID) {
    //Якщо лінія довіри по відношенню до контрагента існує
    if (isTrustLineExist(contractorUUID)) {
        TrustLine *trustLine = mTrustLines.at(contractorUUID);
        //Якщо баланс менший або рівний нулю (лінія довіри від користувача не використана)
        if (trustLine->getBalance() <= ZERO_INT256_VALUE) {
            //Якщо лінія довіри до користувача рівна нулю
            if (trustLine->getIncomingTrustAmount() == ZERO_CHECKED_INT256_VALUE) {
                //Вилучення лінії довіри цілком
                trustLine = nullptr;
                //Якщо при вилученні відбулась помилка - буде повернуто значення -2
                return removeTrustLine(contractorUUID) ? 1 : -2;
            } else {
                //Інакше, якщо лінія довіри до користувача існує, редагуємо значення вихідної лінії довіри,
                //присвоївши йому нуль.
                trustLine->setOutgoingTrustAmount(0);
                return saveTrustLine(trustLine) ? 1 : -2;
            }
        } else {
            //Інакше повернення значення, яке вказує, що лінію довіри від користувача до контрагента закрити не можливо
            return -1;
        }
    } else {
        //Інакше повернення значення, яке вказує, що лінія довіри по відношенню до контрагента не існує
        return 0;
    }
}

bool TrustLinesManager::accept(const uuids::uuid contractorUUID, const trust_amount amount) {
    //Якщо лінія довіри існує
    if (isTrustLineExist(contractorUUID)) {
        //Отримання вказівник на екземпляр
        TrustLine *trustLine = mTrustLines.at(contractorUUID);
        //Встановлення розміру вхідної лінії довіри
        trustLine->setIncomingTrustAmount(amount);
        //Збереження лінії довіри
        return saveTrustLine(trustLine);
    } else {
        //Інакше, створення нового екзмепляру лінії довіри
        //Встановлення розміру вихідної лінії довіри
        TrustLine *trustLine = new TrustLine(contractorUUID, amount, 0, 0);
        //Збереження лінії довіри
        return saveTrustLine(trustLine);
    }
}

int TrustLinesManager::reject(const uuids::uuid contractorUUID) {
    //Якщо лінія довіри по відношенню до контрагента існує
    if (isTrustLineExist(contractorUUID)) {
        TrustLine *trustLine = mTrustLines.at(contractorUUID);
        //Якщо баланс більший або рівний нулю (лінія довіри від контрагента не використана)
        if (trustLine->getBalance() >= ZERO_INT256_VALUE) {
            //Якщо лінія довіри до контрагента рівна нулю
            if (trustLine->getOutgoingTrustAmount() == ZERO_CHECKED_INT256_VALUE) {
                //Вилучення лінії довіри цілком
                trustLine = nullptr;
                //Якщо при вилученні відбулась помилка - буде повернуто значення -2
                return removeTrustLine(contractorUUID) ? 1 : -2;
            } else {
                //Інакше, якщо лінія довіри до контрагента існує, редагуємо значення вхідної лінії довіри,
                //присвоївши йому нуль.
                trustLine->setIncomingTrustAmount(0);
                return saveTrustLine(trustLine) ? 1 : -2;
            }
        } else {
            //Інакше повернення значення, яке вказує, що лінію довіри від контрагента до користувача відхилити не можливо
            return -1;
        }
    } else {
        //Інакше повернення значення, яке вказує, що лінія довіри по відношенню до контрагента не існує
        return 0;
    }
}


