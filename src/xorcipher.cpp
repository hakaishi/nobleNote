/* nobleNote, a note taking application
 * Copyright (C) 2020 Christian Metscher <hakaishi@web.de>,
                      Fabian Deuchler <Taiko000@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * nobleNote is licensed under the MIT, see `http://copyfree.org/licenses/mit/license.txt'.
 */

#include "xorcipher.h"


QString XorCipher::encrypt(QString sourceString, ushort key)
{
    QString encryptedString;
    encryptedString.reserve(sourceString.size());
    for(int i = 0; i< sourceString.size(); ++i)
    {
        encryptedString+=QChar::fromLatin1(sourceString.at(i).unicode()^key%255);
    }
    return encryptedString;
}

QString XorCipher::decrypt(QString encryptedString, ushort key)
{
    QString dencryptedString;
    dencryptedString.reserve(encryptedString.size());
    for(int i = 0; i< encryptedString.size(); ++i)
    {
        dencryptedString+=QChar::fromLatin1(encryptedString.at(i).unicode()^key%255);
    }
    return dencryptedString;
}
