#include "xorcipher.h"


QString XorCipher::encrypt(QString sourceString, ushort key)
{
    QString encryptedString;
    encryptedString.reserve(sourceString.size());
    for(int i = 0; i< sourceString.size(); ++i)
    {
        encryptedString+=sourceString.at(i).unicode()^key%255;
    }
    return encryptedString;
}

QString XorCipher::decrypt(QString encryptedString, ushort key)
{
    QString dencryptedString;
    dencryptedString.reserve(encryptedString.size());
    for(int i = 0; i< encryptedString.size(); ++i)
    {
        dencryptedString+=encryptedString.at(i).unicode()^key%255;
    }
    return dencryptedString;
}
