/* $Header: /cvs/host/zzip/blowfish/blowfish.h,v 1.5 2003/10/06 18:24:25 neal Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 2001 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#ifndef __BLOWFISH_H__
#define __BLOWFISH_H__

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @file blowfish.h
 *
 * @brief Functions to do Blowfish encryption operations.
 */

/*
 * Author     :  Paul Kocher
 * E-mail     :  pck@netcom.com
 * Date       :  1997
 * Description:  C implementation of the Blowfish algorithm.
 */

#define MAXKEYBYTES 56          /**< 448 bits */

/**
 * @brief An area to hold the current encryption context.
 */
typedef struct {
  unsigned long P[16 + 2];
  unsigned long S[4][256];
} BlowfishContext;

/**
 * @brief Initialize a blowfish context with an encryption key.
 * @param context A blob that is filled in with a new context.
 * @param key The encryption key to be used.
 * @param keyLen The length of the encryption key.
 */
void Blowfish_Init(BlowfishContext *context, unsigned char *key, int keyLen);

/**
 * @brief incrimental encryption of a unit of data
 * @param context The current encryption state
 * @param xl left input
 * @param xr right input
 */
void Blowfish_Encrypt(BlowfishContext *context, unsigned long *xl, unsigned long *xr);

/**
 * @brief incrimental decryption of a unit of data
 * @param context The current decryption state
 * @param xl left input
 * @param xr right input
 */
void Blowfish_Decrypt(BlowfishContext *context, unsigned long *xl, unsigned long *xr);

/**
 * @brief Encrypt a whole data block.
 * @param context The current encryption state
 * @param buffer location of data to encrypt
 * @param length how much data to encrypt
 */
void Blowfish_Encrypt_Buffer(BlowfishContext *context, unsigned long *buffer, unsigned long length);

 /**
 * @brief Decrypt a whole data block.
 * @param context The current decryption state
 * @param buffer location of data to decrypt
 * @param length how much data to decrypt
 */
void Blowfish_Decrypt_Buffer(BlowfishContext *context, unsigned long *buffer, unsigned long length);

/**
 * @brief Verify the blowfish context.
 * @param context This verifies the blowfish against test vectors
 * to make sure the implementation is correct. It doesn't test the
 * context (in fact it writes over the context).
 * @return
 *    - 0 = OK
 *    - -1= bad
 */
int Blowfish_Test(BlowfishContext *context);       /* 0=ok, -1=bad */

#ifdef __cplusplus
}
#endif

#endif /* __BLOWFISH_H__ */

/*
 * $Log: blowfish.h,v $
 * Revision 1.5  2003/10/06 18:24:25  neal
 * add or change doxygen comments for documentation.
 *
 * Revision 1.4  2003/09/23 17:22:00  neal
 * added doxygen tags and moved/added comments for documentation.
 *
 * Revision 1.3  2003/09/05 01:00:29  radford
 * o Remove mixture of carriage returns.
 *
 * Revision 1.2  2001/12/12 20:44:43  david
 * - Added Blowfish_Encrypt_Buffer() and Blowfish_Decrypt_Buffer(). These can encrypt/decrypt
 *   more than 8 bytes.
 * - ctx -> context.
 *
 * Revision 1.1.1.1  2001/10/15 18:25:13  david
 * Initial versions of the loader and rom-loader. These may not work yet.
 *
 */
