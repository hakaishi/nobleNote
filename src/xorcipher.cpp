#include "xorcipher.h"


QString XorCipher::encrypt(QString sourceString, int key)
{
    QString encryptedString;
    encryptedString.reserve(sourceString.size());
    for(size_t i = 0; i< sourceString.size(); ++i)
    {
        encryptedString+=sourceString.at(i).unicode()^key%255;
    }
    return encryptedString;
}

QString XorCipher::decrypt(QString encryptedString, int key)
{
    QString dencryptedString;
    dencryptedString.reserve(encryptedString.size());
    for(size_t i = 0; i< encryptedString.size(); ++i)
    {
        dencryptedString+=encryptedString.at(i).unicode()^key%255;
    }
    return dencryptedString;
}
