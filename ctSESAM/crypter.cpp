/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QDebug>
#include <string>
#include "3rdparty/cryptopp562/sha.h"
#include "3rdparty/cryptopp562/ccm.h"
#include "3rdparty/cryptopp562/misc.h"
#include "securebytearray.h"
#include "pbkdf2.h"
#include "crypter.h"
#include "util.h"
#include "global.h"


const int Crypter::SaltSize = 32;
const int Crypter::AESKeySize = 256 / 8;
const int Crypter::DomainIterations = 32768;
const int Crypter::KGKIterations = 1024;
const int Crypter::KGKSize = 64;
const int Crypter::AESBlockSize = CryptoPP::AES::BLOCKSIZE;
const int Crypter::EEKSize = Crypter::SaltSize + Crypter::AESBlockSize + Crypter::KGKSize;

/*!
 * \brief Crypter::encode
 *
 * This function encrypts a block of data with the given encryption key.
 *
 * The encryption key itself is encrypted with the given master key.
 *
 * \param key An AES key generated from the user's master password.
 * \param IV AES initialization vector. A randomly generated byte sequence of `CryptoPP::AES::BLOCKSIZE` length.
 * \param KGK Key generation key. A randomly generated byte sequence of `Crypter::KGKSize` length.
 * \param salt A 32 byte salt.
 * \param data The data to be encrypted.
 * \param compress If `true`, data will be compressed before encryption.
 * \param errCode Will be set to a code that tells the caller what might have gone wrong while encryption.
 * \param errMsg `errCode` as a readable message.
 * \return Block of binary data with the following structure:
 *
 * Bytes   | Description
 * ------- | ---------------------------------------------------------------------------
 *       1 | Format flag (must be 0x01)
 *      32 | Salt (randomly generated)
 *     112 | Encrypted data: 32 bytes randomly generated salt, 16 bytes bytes randomly IV, 64 bytes key generation key.
 *       n | Encrypted data
 *
 */
QByteArray Crypter::encode(const SecureByteArray &key,
                           const SecureByteArray &IV,
                           const QByteArray &salt,
                           const SecureByteArray &KGK,
                           const QByteArray &data,
                           bool compress,
                           int *errCode, QString *errMsg)
{
  Q_ASSERT_X(KGK.size() == KGKSize, "Crypter::encode()", "KGK.size() must be KGKSize");

  if (errCode != nullptr)
    *errCode = NoCryptError;

  SecureByteArray KGK2;
  const QByteArray salt2(randomBytes(SaltSize));
  KGK2.append(salt2);
  const QByteArray IV2(randomBytes(AESBlockSize));
  KGK2.append(IV2);
  KGK2.append(KGK);
  Q_ASSERT_X(KGK2.size() == EEKSize, "Crypter::encode()", "KGK2.size()  must equal EEKSize");

  QByteArray EEK = encrypt(key, IV, KGK2, CryptoPP::StreamTransformationFilter::NO_PADDING);
  Q_ASSERT_X(EEK.size() == EEKSize, "Crypter::encode()", "EEK.size() must equal EEKSize");

  const SecureByteArray &blobKey = Crypter::makeKeyFromPassword(KGK, salt2);
  Q_ASSERT_X(blobKey.size() == AESKeySize, "Crypter::encode()", "blobKey.size() must equal AESKeySize");
  QByteArray _baPlain = compress ? qCompress(data, 9) : data;
  QByteArray baCipher = encrypt(blobKey, IV2, _baPlain, CryptoPP::StreamTransformationFilter::PKCS_PADDING);

  QByteArray result;
  result.append(AES256EncryptedMasterkeyFormat);
  result.append(salt);
  result.append(EEK);
  result.append(baCipher);
  return result;
}

/*!
 * \brief Crypter::decode
 * \param key An AES key generated from the user's master password.
 * \param IV AES initialization vector. A randomly generated byte sequence of `CryptoPP::AES::BLOCKSIZE` length.
 * \param KGK Key generation key. A randomly generated byte sequence of `Crypter::AESKeySize` length.
 * \param salt A 32 byte salt.
 * \param baCipher The data to be decrypted.
 * \param compress If `true`, data will be compressed before encryption.
 * \param errCode Will be set to a code that tells the caller what might have gone wrong while encryption.
 * \param errMsg `errCode` as a readable message.
 * \return
 */
QByteArray Crypter::decode(const SecureByteArray &masterPassword,
                           QByteArray baCipher,
                           bool uncompress,
                           __out SecureByteArray &KGK,
                           __out int *errCode,
                           __out QString *errMsg)
{
  if (errCode != nullptr)
    *errCode = NoCryptError;
  FormatFlags formatFlag = static_cast<FormatFlags>(baCipher.at(0));
  Q_ASSERT_X(formatFlag == AES256EncryptedMasterkeyFormat, "Crypter::decode()", "formatFlag must equal AES256EncryptedMasterkeyFormat");
  baCipher.remove(0, 1);

  const QByteArray &salt = QByteArray(baCipher.constData(), SaltSize);
  baCipher.remove(0, SaltSize);

  SecureByteArray EEK;
  EEK = QByteArray(baCipher.constData(), EEKSize);
  Q_ASSERT_X(EEK.size() == EEKSize, "Crypter::decode()", "EEK.size() must equal EEKSize");
  baCipher.remove(0, EEKSize);

  SecureByteArray key;
  SecureByteArray IV;
  Crypter::makeKeyAndIVFromPassword(masterPassword, salt, key, IV);

  QByteArray baKGK = decrypt(key, IV, EEK, CryptoPP::StreamTransformationFilter::NO_PADDING);
  Q_ASSERT_X(baKGK.size() == EEKSize, "Crypter::decode()", "baKGK.size() must equal EEKSize");

  QByteArray salt2(baKGK.constData(), SaltSize);
  baKGK.remove(0, SaltSize);
  QByteArray IV2(baKGK.constData(), AESBlockSize);
  baKGK.remove(0, AESBlockSize);
  KGK = SecureByteArray(baKGK.constData(), KGKSize);

  const SecureByteArray &blobKey = Crypter::makeKeyFromPassword(KGK, salt2);
  QByteArray plain = decrypt(blobKey, IV2, baCipher, CryptoPP::StreamTransformationFilter::PKCS_PADDING);

  return uncompress ? qUncompress(plain) : plain;
}


QByteArray Crypter::encrypt(const SecureByteArray &key, const SecureByteArray &IV, const QByteArray &baPlain, CryptoPP::StreamTransformationFilter::BlockPaddingScheme padding)
{
  std::string sPlain(baPlain.constData(), baPlain.size());
  std::string sCipher;
  CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
  enc.SetKeyWithIV(reinterpret_cast<const byte*>(key.constData()), key.size(), reinterpret_cast<const byte*>(IV.constData()));
  CryptoPP::ArraySource s(
        sPlain,
        true,
        new CryptoPP::StreamTransformationFilter(
          enc,
          new CryptoPP::StringSink(sCipher),
          padding
          )
        );
  Q_UNUSED(s); // just to please the compiler
  return QByteArray(sCipher.c_str(), sCipher.length());
}


QByteArray Crypter::decrypt(const SecureByteArray &key, const SecureByteArray &IV, const QByteArray &baCipher, CryptoPP::StreamTransformationFilter::BlockPaddingScheme padding)
{
  std::string sCipher(baCipher.constData(), baCipher.size());
  std::string sPlain;
  CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
  dec.SetKeyWithIV(reinterpret_cast<const byte*>(key.constData()), key.size(), reinterpret_cast<const byte*>(IV.constData()));
  CryptoPP::ArraySource s(
        sCipher,
        true,
        new CryptoPP::StreamTransformationFilter(
          dec,
          new CryptoPP::StringSink(sPlain),
          padding
          )
        );
  Q_UNUSED(s); // just to please the compiler
  return QByteArray(sPlain.c_str(), sPlain.length());
}


QByteArray Crypter::randomBytes(int size)
{
  QByteArray buf(size, static_cast<char>(0));
  for (int i = 0; i < buf.size(); ++i)
    buf[i] = static_cast<char>(gRandomDevice());
  return buf;
}


SecureByteArray Crypter::makeKeyFromPassword(const SecureByteArray &masterPassword, const QByteArray &salt)
{
  PBKDF2 cryptPassword;
  cryptPassword.setSalt(salt);
  cryptPassword.setIterations(KGKIterations);
  cryptPassword.generate(masterPassword, QCryptographicHash::Sha256);
  return cryptPassword.derivedKey(Crypter::AESKeySize);
}


void Crypter::makeKeyAndIVFromPassword(const SecureByteArray &masterPassword, const QByteArray &salt, SecureByteArray &key, SecureByteArray &IV)
{
  PBKDF2 cryptPassword;
  cryptPassword.setIterations(DomainIterations);
  cryptPassword.setSalt(salt);
  cryptPassword.generate(masterPassword, QCryptographicHash::Sha384);
  const SecureByteArray &hash = cryptPassword.derivedKey();
  key = hash.mid(0, AESKeySize);
  IV = hash.mid(AESKeySize, AESBlockSize);
}
