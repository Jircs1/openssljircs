/*
 * Copyright 2020-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "internal/nelem.h"

#include <openssl/pkcs12.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include "testutil.h"
#include "helpers/pkcs12.h"

static int default_libctx = 1;

static OSSL_LIB_CTX *testctx = NULL;
static OSSL_PROVIDER *nullprov = NULL;
static OSSL_PROVIDER *deflprov = NULL;
static OSSL_PROVIDER *lgcyprov = NULL;

/* --------------------------------------------------------------------------
 * PKCS12 component test data
 */

static const unsigned char CERT1[] =
{
    0x30, 0x82, 0x01, 0xed, 0x30, 0x82, 0x01, 0x56, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x09, 0x00,
    0x8b, 0x4b, 0x5e, 0x6c, 0x03, 0x28, 0x4e, 0xe6, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x19, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55,
    0x04, 0x03, 0x0c, 0x0e, 0x50, 0x31, 0x32, 0x54, 0x65, 0x73, 0x74, 0x2d, 0x52, 0x6f, 0x6f, 0x74,
    0x2d, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x39, 0x30, 0x39, 0x33, 0x30, 0x30, 0x30, 0x34, 0x36,
    0x35, 0x36, 0x5a, 0x17, 0x0d, 0x32, 0x39, 0x30, 0x39, 0x32, 0x37, 0x30, 0x30, 0x34, 0x36, 0x35,
    0x36, 0x5a, 0x30, 0x1b, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x10, 0x50,
    0x31, 0x32, 0x54, 0x65, 0x73, 0x74, 0x2d, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x2d, 0x31, 0x30,
    0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05,
    0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xbc, 0xdc, 0x6f, 0x8c,
    0x7a, 0x2a, 0x4b, 0xea, 0x66, 0x66, 0x04, 0xa9, 0x05, 0x92, 0x53, 0xd7, 0x13, 0x3c, 0x49, 0xe1,
    0xc8, 0xbb, 0xdf, 0x3d, 0xcb, 0x88, 0x31, 0x07, 0x20, 0x59, 0x93, 0x24, 0x7f, 0x7d, 0xc6, 0x84,
    0x81, 0x16, 0x64, 0x4a, 0x52, 0xa6, 0x30, 0x44, 0xdc, 0x1a, 0x30, 0xde, 0xae, 0x29, 0x18, 0xcf,
    0xc7, 0xf3, 0xcf, 0x0c, 0xb7, 0x8e, 0x2b, 0x1e, 0x21, 0x01, 0x0b, 0xfb, 0xe5, 0xe6, 0xcf, 0x2b,
    0x84, 0xe1, 0x33, 0xf8, 0xba, 0x02, 0xfc, 0x30, 0xfa, 0xc4, 0x33, 0xc7, 0x37, 0xc6, 0x7f, 0x72,
    0x31, 0x92, 0x1d, 0x8f, 0xa0, 0xfb, 0xe5, 0x4a, 0x08, 0x31, 0x78, 0x80, 0x9c, 0x23, 0xb4, 0xe9,
    0x19, 0x56, 0x04, 0xfa, 0x0d, 0x07, 0x04, 0xb7, 0x43, 0xac, 0x4c, 0x49, 0x7c, 0xc2, 0xa1, 0x44,
    0xc1, 0x48, 0x7d, 0x28, 0xe5, 0x23, 0x66, 0x07, 0x22, 0xd5, 0xf0, 0xf1, 0x02, 0x03, 0x01, 0x00,
    0x01, 0xa3, 0x3b, 0x30, 0x39, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16,
    0x80, 0x14, 0xdb, 0xbb, 0xb8, 0x92, 0x4e, 0x24, 0x0b, 0x1b, 0xbb, 0x78, 0x33, 0xf9, 0x01, 0x02,
    0x23, 0x0d, 0x96, 0x18, 0x30, 0x47, 0x30, 0x09, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x04, 0x02, 0x30,
    0x00, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x04, 0x04, 0x03, 0x02, 0x04, 0xf0, 0x30, 0x0d,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x81, 0x81,
    0x00, 0x1c, 0x13, 0xdc, 0x02, 0xf1, 0x44, 0x36, 0x65, 0xa9, 0xbe, 0x30, 0x1c, 0x66, 0x14, 0x20,
    0x86, 0x5a, 0xa8, 0x69, 0x25, 0xf8, 0x1a, 0xb6, 0x9e, 0x5e, 0xe9, 0x89, 0xb8, 0x67, 0x70, 0x19,
    0x87, 0x60, 0xeb, 0x4b, 0x11, 0x71, 0x85, 0xf8, 0xe9, 0xa7, 0x3e, 0x20, 0x42, 0xec, 0x43, 0x25,
    0x01, 0x03, 0xe5, 0x4d, 0x83, 0x22, 0xf5, 0x8e, 0x3a, 0x1a, 0x1b, 0xd4, 0x1c, 0xda, 0x6b, 0x9d,
    0x10, 0x1b, 0xee, 0x67, 0x4e, 0x1f, 0x69, 0xab, 0xbc, 0xaa, 0x62, 0x8e, 0x9e, 0xc6, 0xee, 0xd6,
    0x09, 0xc0, 0xca, 0xe0, 0xaa, 0x9f, 0x07, 0xb2, 0xc2, 0xbb, 0x31, 0x96, 0xa2, 0x04, 0x62, 0xd3,
    0x13, 0x32, 0x29, 0x67, 0x6e, 0xad, 0x2e, 0x0b, 0xea, 0x04, 0x7c, 0x8c, 0x5a, 0x5d, 0xac, 0x14,
    0xaa, 0x61, 0x7f, 0x28, 0x6c, 0x2d, 0x64, 0x2d, 0xc3, 0xaf, 0x77, 0x52, 0x90, 0xb4, 0x37, 0xc0,
    0x30, 
};

static const unsigned char CERT2[] =
{
    0x30, 0x82, 0x01, 0xed, 0x30, 0x82, 0x01, 0x56, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x09, 0x00,
    0x8b, 0x4b, 0x5e, 0x6c, 0x03, 0x28, 0x4e, 0xe7, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x19, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55,
    0x04, 0x03, 0x0c, 0x0e, 0x50, 0x31, 0x32, 0x54, 0x65, 0x73, 0x74, 0x2d, 0x52, 0x6f, 0x6f, 0x74,
    0x2d, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x39, 0x30, 0x39, 0x33, 0x30, 0x30, 0x30, 0x34, 0x36,
    0x35, 0x36, 0x5a, 0x17, 0x0d, 0x32, 0x39, 0x30, 0x39, 0x32, 0x37, 0x30, 0x30, 0x34, 0x36, 0x35,
    0x36, 0x5a, 0x30, 0x1b, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x10, 0x50,
    0x31, 0x32, 0x54, 0x65, 0x73, 0x74, 0x2d, 0x43, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x2d, 0x31, 0x30,
    0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05,
    0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xa8, 0x6e, 0x40, 0x86,
    0x9f, 0x98, 0x59, 0xfb, 0x57, 0xbf, 0xc1, 0x55, 0x12, 0x38, 0xeb, 0xb3, 0x46, 0x34, 0xc9, 0x35,
    0x4d, 0xfd, 0x03, 0xe9, 0x3a, 0x88, 0x9e, 0x97, 0x8f, 0xf4, 0xec, 0x36, 0x7b, 0x3f, 0xba, 0xb8,
    0xa5, 0x96, 0x30, 0x03, 0xc5, 0xc6, 0xd9, 0xa8, 0x4e, 0xbc, 0x23, 0x51, 0xa1, 0x96, 0xd2, 0x03,
    0x98, 0x73, 0xb6, 0x17, 0x9c, 0x77, 0xd4, 0x95, 0x1e, 0x1b, 0xb3, 0x1b, 0xc8, 0x71, 0xd1, 0x2e,
    0x31, 0xc7, 0x6a, 0x75, 0x57, 0x08, 0x7f, 0xba, 0x70, 0x76, 0xf7, 0x67, 0xf4, 0x4e, 0xbe, 0xfc,
    0x70, 0x61, 0x41, 0x07, 0x2b, 0x7c, 0x3c, 0x3b, 0xb3, 0xbc, 0xd5, 0xa8, 0xbd, 0x28, 0xd8, 0x49,
    0xd3, 0xe1, 0x78, 0xc8, 0xc1, 0x42, 0x5e, 0x18, 0x36, 0xa8, 0x41, 0xf7, 0xc8, 0xaa, 0x35, 0xfe,
    0x2d, 0xd1, 0xb4, 0xcc, 0x00, 0x67, 0xae, 0x79, 0xd3, 0x28, 0xd5, 0x5b, 0x02, 0x03, 0x01, 0x00,
    0x01, 0xa3, 0x3b, 0x30, 0x39, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16,
    0x80, 0x14, 0xdb, 0xbb, 0xb8, 0x92, 0x4e, 0x24, 0x0b, 0x1b, 0xbb, 0x78, 0x33, 0xf9, 0x01, 0x02,
    0x23, 0x0d, 0x96, 0x18, 0x30, 0x47, 0x30, 0x09, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x04, 0x02, 0x30,
    0x00, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x04, 0x04, 0x03, 0x02, 0x04, 0xf0, 0x30, 0x0d,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x81, 0x81,
    0x00, 0x3b, 0xa6, 0x73, 0xbe, 0xe0, 0x28, 0xed, 0x1f, 0x29, 0x78, 0x4c, 0xc0, 0x1f, 0xe9, 0x85,
    0xc6, 0x8f, 0xe3, 0x87, 0x7c, 0xd9, 0xe7, 0x0a, 0x37, 0xe8, 0xaa, 0xb5, 0xd2, 0x7f, 0xf8, 0x90,
    0x20, 0x80, 0x35, 0xa7, 0x79, 0x2b, 0x04, 0xa7, 0xbf, 0xe6, 0x7b, 0x58, 0xcb, 0xec, 0x0e, 0x58,
    0xef, 0x2a, 0x70, 0x8a, 0x56, 0x8a, 0xcf, 0x6b, 0x7a, 0x74, 0x0c, 0xf4, 0x15, 0x37, 0x93, 0xcd,
    0xe6, 0xb2, 0xa1, 0x83, 0x09, 0xdb, 0x9e, 0x4f, 0xff, 0x6a, 0x17, 0x4f, 0x33, 0xc9, 0xcc, 0x90,
    0x2a, 0x67, 0xff, 0x16, 0x78, 0xa8, 0x2c, 0x10, 0xe0, 0x52, 0x8c, 0xe6, 0xe9, 0x90, 0x8d, 0xe0,
    0x62, 0x04, 0x9a, 0x0f, 0x44, 0x01, 0x82, 0x14, 0x92, 0x44, 0x25, 0x69, 0x22, 0xb7, 0xb8, 0xc5,
    0x94, 0x4c, 0x4b, 0x1c, 0x9b, 0x92, 0x60, 0x66, 0x90, 0x4e, 0xb9, 0xa8, 0x4c, 0x89, 0xbb, 0x0f,
    0x0b, 
};

static const unsigned char KEY1[] =
{
    0x30, 0x82, 0x02, 0x5d, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xbc, 0xdc, 0x6f, 0x8c, 0x7a,
    0x2a, 0x4b, 0xea, 0x66, 0x66, 0x04, 0xa9, 0x05, 0x92, 0x53, 0xd7, 0x13, 0x3c, 0x49, 0xe1, 0xc8,
    0xbb, 0xdf, 0x3d, 0xcb, 0x88, 0x31, 0x07, 0x20, 0x59, 0x93, 0x24, 0x7f, 0x7d, 0xc6, 0x84, 0x81,
    0x16, 0x64, 0x4a, 0x52, 0xa6, 0x30, 0x44, 0xdc, 0x1a, 0x30, 0xde, 0xae, 0x29, 0x18, 0xcf, 0xc7,
    0xf3, 0xcf, 0x0c, 0xb7, 0x8e, 0x2b, 0x1e, 0x21, 0x01, 0x0b, 0xfb, 0xe5, 0xe6, 0xcf, 0x2b, 0x84,
    0xe1, 0x33, 0xf8, 0xba, 0x02, 0xfc, 0x30, 0xfa, 0xc4, 0x33, 0xc7, 0x37, 0xc6, 0x7f, 0x72, 0x31,
    0x92, 0x1d, 0x8f, 0xa0, 0xfb, 0xe5, 0x4a, 0x08, 0x31, 0x78, 0x80, 0x9c, 0x23, 0xb4, 0xe9, 0x19,
    0x56, 0x04, 0xfa, 0x0d, 0x07, 0x04, 0xb7, 0x43, 0xac, 0x4c, 0x49, 0x7c, 0xc2, 0xa1, 0x44, 0xc1,
    0x48, 0x7d, 0x28, 0xe5, 0x23, 0x66, 0x07, 0x22, 0xd5, 0xf0, 0xf1, 0x02, 0x03, 0x01, 0x00, 0x01,
    0x02, 0x81, 0x81, 0x00, 0xa5, 0x6d, 0xf9, 0x8f, 0xf5, 0x5a, 0xa3, 0x50, 0xd9, 0x0d, 0x37, 0xbb,
    0xce, 0x13, 0x94, 0xb8, 0xea, 0x32, 0x7f, 0x0c, 0xf5, 0x46, 0x0b, 0x90, 0x17, 0x7e, 0x5e, 0x63,
    0xbd, 0xa4, 0x78, 0xcd, 0x19, 0x97, 0xd4, 0x92, 0x30, 0x78, 0xaa, 0xb4, 0xa7, 0x9c, 0xc6, 0xdf,
    0x2a, 0x65, 0x0e, 0xb5, 0x9f, 0x9c, 0x84, 0x0d, 0x4d, 0x3a, 0x74, 0xfc, 0xd0, 0xb4, 0x09, 0x74,
    0xc4, 0xb8, 0x24, 0x03, 0xa8, 0xf0, 0xf8, 0x0d, 0x5c, 0x8e, 0xdf, 0x4b, 0xe1, 0x0a, 0x8f, 0x4f,
    0xd5, 0xc7, 0x9b, 0x54, 0x55, 0x8f, 0x00, 0x5c, 0xea, 0x4c, 0x73, 0xf9, 0x1b, 0xbf, 0xb8, 0x93,
    0x33, 0x20, 0xce, 0x45, 0xd9, 0x03, 0x02, 0xb2, 0x36, 0xc5, 0x0a, 0x30, 0x50, 0x78, 0x80, 0x66,
    0x00, 0x22, 0x38, 0x86, 0xcf, 0x63, 0x4a, 0x5c, 0xbf, 0x2b, 0xd9, 0x6e, 0xe6, 0xf0, 0x39, 0xad,
    0x12, 0x25, 0x41, 0xb9, 0x02, 0x41, 0x00, 0xf3, 0x7c, 0x07, 0x99, 0x64, 0x3a, 0x28, 0x8c, 0x8d,
    0x05, 0xfe, 0x32, 0xb5, 0x4c, 0x8c, 0x6d, 0xde, 0x3d, 0x16, 0x08, 0xa0, 0x01, 0x61, 0x4f, 0x8e,
    0xa0, 0xf7, 0x26, 0x26, 0xb5, 0x8e, 0xc0, 0x7a, 0xce, 0x86, 0x34, 0xde, 0xb8, 0xef, 0x86, 0x01,
    0xbe, 0x24, 0xaa, 0x9b, 0x36, 0x93, 0x72, 0x9b, 0xf9, 0xc6, 0xcb, 0x76, 0x84, 0x67, 0x06, 0x06,
    0x30, 0x50, 0xdf, 0x42, 0x17, 0xe0, 0xa7, 0x02, 0x41, 0x00, 0xc6, 0x91, 0xa0, 0x41, 0x34, 0x11,
    0x67, 0x4b, 0x08, 0x0f, 0xda, 0xa7, 0x99, 0xec, 0x58, 0x11, 0xa5, 0x82, 0xdb, 0x50, 0xfe, 0x77,
    0xe2, 0xd1, 0x53, 0x9c, 0x7d, 0xe8, 0xbf, 0xe7, 0x7c, 0xa9, 0x01, 0xb1, 0x87, 0xc3, 0x52, 0x79,
    0x9e, 0x2c, 0xa7, 0x6f, 0x02, 0x37, 0x32, 0xef, 0x24, 0x31, 0x21, 0x0b, 0x86, 0x05, 0x32, 0x4a,
    0x2e, 0x0b, 0x65, 0x05, 0xd3, 0xd6, 0x30, 0xb2, 0xfc, 0xa7, 0x02, 0x41, 0x00, 0xc2, 0xed, 0x31,
    0xdc, 0x40, 0x9c, 0x3a, 0xe8, 0x42, 0xe2, 0x60, 0x5e, 0x52, 0x3c, 0xc5, 0x54, 0x14, 0x0e, 0x8d,
    0x7c, 0x3c, 0x34, 0xbe, 0xa6, 0x05, 0x86, 0xa2, 0x36, 0x5d, 0xd9, 0x0e, 0x3e, 0xd4, 0x52, 0x50,
    0xa9, 0x35, 0x01, 0x93, 0x68, 0x92, 0x2e, 0x9a, 0x86, 0x27, 0x1a, 0xab, 0x32, 0x9e, 0xe2, 0x79,
    0x9f, 0x5b, 0xf3, 0xa5, 0xd2, 0xf1, 0xd3, 0x6e, 0x7b, 0x3e, 0x1b, 0x85, 0x93, 0x02, 0x40, 0x68,
    0xb8, 0xb6, 0x7e, 0x8c, 0xba, 0x3c, 0xf2, 0x8a, 0x2e, 0xea, 0x4f, 0x07, 0xd3, 0x68, 0x62, 0xee,
    0x1a, 0x04, 0x16, 0x44, 0x0d, 0xef, 0xf6, 0x1b, 0x95, 0x65, 0xa5, 0xd1, 0x47, 0x81, 0x2c, 0x14,
    0xb3, 0x8e, 0xf9, 0x08, 0xcf, 0x11, 0x07, 0x55, 0xca, 0x2a, 0xad, 0xf7, 0xd3, 0xbd, 0x0f, 0x97,
    0xf0, 0xde, 0xde, 0x70, 0xb6, 0x44, 0x70, 0x47, 0xf7, 0xf9, 0xcf, 0x75, 0x61, 0x7f, 0xf3, 0x02,
    0x40, 0x38, 0x4a, 0x67, 0xaf, 0xae, 0xb6, 0xb2, 0x6a, 0x00, 0x25, 0x5a, 0xa4, 0x65, 0x20, 0xb1,
    0x13, 0xbd, 0x83, 0xff, 0xb4, 0xbc, 0xf4, 0xdd, 0xa1, 0xbb, 0x1c, 0x96, 0x37, 0x35, 0xf4, 0xbf,
    0xed, 0x4c, 0xed, 0x92, 0xe8, 0xac, 0xc9, 0xc1, 0xa5, 0xa3, 0x23, 0x66, 0x40, 0x8a, 0xa1, 0xe6,
    0xe3, 0x95, 0xfe, 0xc4, 0x53, 0xf5, 0x7d, 0x6e, 0xca, 0x45, 0x42, 0xe4, 0xc2, 0x9f, 0xe5, 0x1e,
    0xb5, 
};


static const unsigned char KEY2[] =
{
    0x30, 0x82, 0x02, 0x5c, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xa8, 0x6e, 0x40, 0x86, 0x9f,
    0x98, 0x59, 0xfb, 0x57, 0xbf, 0xc1, 0x55, 0x12, 0x38, 0xeb, 0xb3, 0x46, 0x34, 0xc9, 0x35, 0x4d,
    0xfd, 0x03, 0xe9, 0x3a, 0x88, 0x9e, 0x97, 0x8f, 0xf4, 0xec, 0x36, 0x7b, 0x3f, 0xba, 0xb8, 0xa5,
    0x96, 0x30, 0x03, 0xc5, 0xc6, 0xd9, 0xa8, 0x4e, 0xbc, 0x23, 0x51, 0xa1, 0x96, 0xd2, 0x03, 0x98,
    0x73, 0xb6, 0x17, 0x9c, 0x77, 0xd4, 0x95, 0x1e, 0x1b, 0xb3, 0x1b, 0xc8, 0x71, 0xd1, 0x2e, 0x31,
    0xc7, 0x6a, 0x75, 0x57, 0x08, 0x7f, 0xba, 0x70, 0x76, 0xf7, 0x67, 0xf4, 0x4e, 0xbe, 0xfc, 0x70,
    0x61, 0x41, 0x07, 0x2b, 0x7c, 0x3c, 0x3b, 0xb3, 0xbc, 0xd5, 0xa8, 0xbd, 0x28, 0xd8, 0x49, 0xd3,
    0xe1, 0x78, 0xc8, 0xc1, 0x42, 0x5e, 0x18, 0x36, 0xa8, 0x41, 0xf7, 0xc8, 0xaa, 0x35, 0xfe, 0x2d,
    0xd1, 0xb4, 0xcc, 0x00, 0x67, 0xae, 0x79, 0xd3, 0x28, 0xd5, 0x5b, 0x02, 0x03, 0x01, 0x00, 0x01,
    0x02, 0x81, 0x81, 0x00, 0xa6, 0x00, 0x83, 0xf8, 0x2b, 0x33, 0xac, 0xfb, 0xdb, 0xf0, 0x52, 0x4b,
    0xd6, 0x39, 0xe3, 0x94, 0x3d, 0x8d, 0xa9, 0x01, 0xb0, 0x6b, 0xbe, 0x7f, 0x10, 0x01, 0xb6, 0xcd,
    0x0a, 0x45, 0x0a, 0xca, 0x67, 0x8e, 0xd8, 0x29, 0x44, 0x8a, 0x51, 0xa8, 0x66, 0x35, 0x26, 0x30,
    0x8b, 0xe9, 0x41, 0xa6, 0x22, 0xec, 0xd2, 0xf0, 0x58, 0x41, 0x33, 0x26, 0xf2, 0x3f, 0xe8, 0x75,
    0x4f, 0xc7, 0x5d, 0x2e, 0x5a, 0xa8, 0x7a, 0xd2, 0xbf, 0x59, 0xa0, 0x86, 0x79, 0x0b, 0x92, 0x6c,
    0x95, 0x5d, 0x87, 0x63, 0x5c, 0xd6, 0x1a, 0xc0, 0xf6, 0x7a, 0x15, 0x8d, 0xc7, 0x3c, 0xb6, 0x9e,
    0xa6, 0x58, 0x46, 0x9b, 0xbf, 0x3e, 0x28, 0x8c, 0xdf, 0x1a, 0x87, 0xaa, 0x7e, 0xf5, 0xf2, 0xcb,
    0x5e, 0x84, 0x2d, 0xf6, 0x82, 0x7e, 0x89, 0x4e, 0xf5, 0xe6, 0x3c, 0x92, 0x80, 0x1e, 0x98, 0x1c,
    0x6a, 0x7b, 0x57, 0x01, 0x02, 0x41, 0x00, 0xdd, 0x60, 0x95, 0xd7, 0xa1, 0x9d, 0x0c, 0xa1, 0x84,
    0xc5, 0x39, 0xca, 0x67, 0x4c, 0x1c, 0x06, 0x71, 0x5b, 0x5c, 0x2d, 0x8d, 0xce, 0xcd, 0xe2, 0x79,
    0xc8, 0x33, 0xbe, 0x50, 0x37, 0x60, 0x9f, 0x3b, 0xb9, 0x59, 0x55, 0x22, 0x1f, 0xa5, 0x4b, 0x1d,
    0xca, 0x38, 0xa0, 0xab, 0x87, 0x9c, 0x86, 0x0e, 0xdb, 0x1c, 0x4f, 0x4f, 0x07, 0xed, 0x18, 0x3f,
    0x05, 0x3c, 0xec, 0x78, 0x11, 0xf6, 0x99, 0x02, 0x41, 0x00, 0xc2, 0xc5, 0xcf, 0xbe, 0x95, 0x91,
    0xeb, 0xcf, 0x47, 0xf3, 0x33, 0x32, 0xc7, 0x7e, 0x93, 0x56, 0xf7, 0xd8, 0xf9, 0xd4, 0xb6, 0xd6,
    0x20, 0xac, 0xba, 0x8a, 0x20, 0x19, 0x14, 0xab, 0xc5, 0x5d, 0xb2, 0x08, 0xcc, 0x77, 0x7c, 0x65,
    0xa8, 0xdb, 0x66, 0x97, 0x36, 0x44, 0x2c, 0x63, 0xc0, 0x6a, 0x7e, 0xb0, 0x0b, 0x5c, 0x90, 0x12,
    0x50, 0xb4, 0x36, 0x60, 0xc3, 0x1f, 0x22, 0x0c, 0xc8, 0x13, 0x02, 0x40, 0x33, 0xc8, 0x7e, 0x04,
    0x7c, 0x97, 0x61, 0xf6, 0xfe, 0x39, 0xac, 0x34, 0xfe, 0x48, 0xbd, 0x5d, 0x7c, 0x72, 0xa4, 0x73,
    0x3b, 0x72, 0x9e, 0x92, 0x55, 0x6e, 0x51, 0x3c, 0x39, 0x43, 0x5a, 0xe4, 0xa4, 0x71, 0xcc, 0xc5,
    0xaf, 0x3f, 0xbb, 0xc8, 0x80, 0x65, 0x67, 0x2d, 0x9e, 0x32, 0x10, 0x99, 0x03, 0x2c, 0x99, 0xc8,
    0xab, 0x71, 0xed, 0x31, 0xf8, 0xbb, 0xde, 0xee, 0x69, 0x7f, 0xba, 0x31, 0x02, 0x40, 0x7e, 0xbc,
    0x60, 0x55, 0x4e, 0xd5, 0xc8, 0x6e, 0xf4, 0x0e, 0x57, 0xbe, 0x2e, 0xf9, 0x39, 0xbe, 0x59, 0x3f,
    0xa2, 0x30, 0xbb, 0x57, 0xd1, 0xa3, 0x13, 0x2e, 0x55, 0x7c, 0x7c, 0x6a, 0xd8, 0xde, 0x02, 0xbe,
    0x9e, 0xed, 0x10, 0xd0, 0xc5, 0x73, 0x1d, 0xea, 0x3e, 0xb1, 0x55, 0x81, 0x02, 0xef, 0x48, 0xc8,
    0x1c, 0x5c, 0x7a, 0x92, 0xb0, 0x58, 0xd3, 0x19, 0x5b, 0x5d, 0xa2, 0xb6, 0x56, 0x69, 0x02, 0x40,
    0x1e, 0x00, 0x6a, 0x9f, 0xba, 0xee, 0x46, 0x5a, 0xc5, 0xb5, 0x9f, 0x91, 0x33, 0xdd, 0xc9, 0x96,
    0x75, 0xb7, 0x87, 0xcf, 0x18, 0x1c, 0xb7, 0xb9, 0x3f, 0x04, 0x10, 0xb8, 0x75, 0xa9, 0xb8, 0xa0,
    0x31, 0x35, 0x03, 0x30, 0x89, 0xc8, 0x37, 0x68, 0x20, 0x30, 0x99, 0x39, 0x96, 0xd6, 0x2b, 0x3d,
    0x5e, 0x45, 0x84, 0xf7, 0xd2, 0x61, 0x50, 0xc9, 0x50, 0xba, 0x8d, 0x08, 0xaa, 0xd0, 0x08, 0x1e,
};


static const PKCS12_ATTR ATTRS1[] = {
    { "friendlyName", "george" },
    { "localKeyID", "1234567890" },
    { "1.2.3.4.5", "MyCustomAttribute" },
    { NULL, NULL }
};

static const PKCS12_ATTR ATTRS2[] = {
    { "friendlyName", "janet" },
    { "localKeyID", "987654321" },
    { "1.2.3.5.8.13", "AnotherCustomAttribute" },
    { NULL, NULL }
};

static const PKCS12_ATTR ATTRS3[] = {
    { "friendlyName", "wildduk" },
    { "localKeyID", "1122334455" },
    { "oracle-jdk-trustedkeyusage", "anyExtendedKeyUsage" },
    { NULL, NULL }
};

static const PKCS12_ATTR ATTRS4[] = {
    { "friendlyName", "wildduk" },
    { "localKeyID", "1122334455" },
    { NULL, NULL }
};

static const PKCS12_ENC enc_default = {
#ifndef OPENSSL_NO_DES
    NID_pbe_WithSHA1And3_Key_TripleDES_CBC,
#else
    NID_aes_128_cbc,
#endif
    "Password1",
    1000
};

static const PKCS12_ENC mac_default = {
    NID_sha1,
    "Password1",
    1000
};

static const int enc_nids_all[] = {
    /* NOTE: To use PBES2 we pass the desired cipher NID instead of NID_pbes2 */
    NID_aes_128_cbc,
    NID_aes_256_cbc,
#ifndef OPENSSL_NO_DES
    NID_des_ede3_cbc,
    NID_des_cbc,
#endif
#ifndef OPENSSL_NO_RC5
    NID_rc5_cbc,
#endif
#ifndef OPENSSL_NO_RC4
    NID_rc4,
#endif
#ifndef OPENSSL_NO_RC2
    NID_rc2_cbc,
#endif

#ifndef OPENSSL_NO_MD2
# ifndef OPENSSL_NO_DES
    NID_pbeWithMD2AndDES_CBC,
# endif
# ifndef OPENSSL_NO_RC2
    NID_pbeWithMD2AndRC2_CBC,
# endif
#endif

#ifndef OPENSSL_NO_MD5
# ifndef OPENSSL_NO_DES
    NID_pbeWithMD5AndDES_CBC,
# endif
# ifndef OPENSSL_NO_RC2
    NID_pbeWithMD5AndRC2_CBC,
# endif
#endif
#ifndef OPENSSL_NO_DES
    NID_pbeWithSHA1AndDES_CBC,
#endif
#ifndef OPENSSL_NO_RC2
    NID_pbe_WithSHA1And128BitRC2_CBC,
    NID_pbe_WithSHA1And40BitRC2_CBC,
    NID_pbeWithSHA1AndRC2_CBC,
#endif
#ifndef OPENSSL_NO_RC4
    NID_pbe_WithSHA1And128BitRC4,
    NID_pbe_WithSHA1And40BitRC4,
#endif
#ifndef OPENSSL_NO_DES
    NID_pbe_WithSHA1And2_Key_TripleDES_CBC,
    NID_pbe_WithSHA1And3_Key_TripleDES_CBC,
#endif
};

static const int enc_nids_no_legacy[] = {
    /* NOTE: To use PBES2 we pass the desired cipher NID instead of NID_pbes2 */
    NID_aes_128_cbc,
    NID_aes_256_cbc,
#ifndef OPENSSL_NO_DES
    NID_des_ede3_cbc,
    NID_pbe_WithSHA1And2_Key_TripleDES_CBC,
    NID_pbe_WithSHA1And3_Key_TripleDES_CBC,
#endif
};

static const int mac_nids[] = {
    NID_sha1,
    NID_md5,
    NID_sha256,
    NID_sha512,
    NID_sha3_256,
    NID_sha3_512
};

static const int iters[] = {
    1,
    1000
};

static const char *passwords[] = {
    "Password1",
    "",
};

/* --------------------------------------------------------------------------
 * Local functions
 */ 

static int get_custom_oid(void)
{
    static int sec_nid = -1;

    if (sec_nid != -1)
        return sec_nid;
    if (!TEST_true(OBJ_create("1.3.5.7.9", "CustomSecretOID", "My custom secret OID")))
        return -1;
    return sec_nid = OBJ_txt2nid("CustomSecretOID");
}


/* --------------------------------------------------------------------------
 * PKCS12 format tests
 */

static int test_single_cert_no_attrs(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("1cert.p12");

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_certbag(pb, CERT1, sizeof(CERT1), NULL);

        end_contentinfo(pb);

    end_pkcs12(pb);

    /* Read/decode */
    start_check_pkcs12(pb);

        start_check_contentinfo(pb);

            check_certbag(pb, CERT1, sizeof(CERT1), NULL);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_single_key(PKCS12_ENC *enc)
{
    char fname[80];
    PKCS12_BUILDER *pb;

    sprintf(fname, "1key_ciph-%s_iter-%d.p12", OBJ_nid2sn(enc->nid), enc->iter);

    pb = new_pkcs12_builder(fname);

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_keybag(pb, KEY1, sizeof(KEY1), NULL, enc);

        end_contentinfo(pb);

    end_pkcs12(pb);

    /* Read/decode */
    start_check_pkcs12(pb);

        start_check_contentinfo(pb);

            check_keybag(pb, KEY1, sizeof(KEY1), NULL, enc);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_single_key_enc_alg(int z)
{
    PKCS12_ENC enc;

    if (lgcyprov == NULL)
        enc.nid = enc_nids_no_legacy[z];
    else
        enc.nid = enc_nids_all[z];
    enc.pass = enc_default.pass;
    enc.iter = enc_default.iter;

    return test_single_key(&enc);
}

static int test_single_key_enc_pass(int z)
{
    PKCS12_ENC enc;

    enc.nid = enc_default.nid;
    enc.pass = passwords[z];
    enc.iter = enc_default.iter;

    return test_single_key(&enc);
}

static int test_single_key_enc_iter(int z)
{
    PKCS12_ENC enc;

    enc.nid = enc_default.nid;
    enc.pass = enc_default.pass;
    enc.iter = iters[z];

    return test_single_key(&enc);
}

static int test_single_key_with_attrs(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("1keyattrs.p12");

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);

        end_contentinfo(pb);

    end_pkcs12(pb);

    /* Read/decode */
    start_check_pkcs12(pb);

        start_check_contentinfo(pb);

            check_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_single_cert_mac(PKCS12_ENC *mac)
{
    char fname[80];
    PKCS12_BUILDER *pb;

    sprintf(fname, "1cert_mac-%s_iter-%d.p12", OBJ_nid2sn(mac->nid), mac->iter);

    pb = new_pkcs12_builder(fname);

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_certbag(pb, CERT1, sizeof(CERT1), NULL);

        end_contentinfo(pb);

    end_pkcs12_with_mac(pb, mac);

    /* Read/decode */
    start_check_pkcs12_with_mac(pb, mac);

        start_check_contentinfo(pb);

            check_certbag(pb, CERT1, sizeof(CERT1), NULL);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_single_cert_mac_alg(int z)
{
    PKCS12_ENC mac;

    mac.nid = mac_nids[z];
    mac.pass = mac_default.pass;
    mac.iter = mac_default.iter;

    return test_single_cert_mac(&mac);
}

static int test_single_cert_mac_pass(int z)
{
    PKCS12_ENC mac;

    mac.nid = mac_default.nid;
    mac.pass = passwords[z];
    mac.iter = mac_default.iter;

    return test_single_cert_mac(&mac);
}

static int test_single_cert_mac_iter(int z)
{
    PKCS12_ENC mac;

    mac.nid = mac_default.nid;
    mac.pass = mac_default.pass;
    mac.iter = iters[z];

    return test_single_cert_mac(&mac);
}

static int test_cert_key_with_attrs_and_mac(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("1cert1key.p12");

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_certbag(pb, CERT1, sizeof(CERT1), ATTRS1);
            add_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);

        end_contentinfo(pb);

    end_pkcs12_with_mac(pb, &mac_default);

    /* Read/decode */
    start_check_pkcs12_with_mac(pb, &mac_default);

        start_check_contentinfo(pb);

            check_certbag(pb, CERT1, sizeof(CERT1), ATTRS1);
            check_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_cert_key_encrypted_content(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("1cert1key_enc.p12");

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_certbag(pb, CERT1, sizeof(CERT1), ATTRS1);
            add_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);

        end_contentinfo_encrypted(pb, &enc_default);

    end_pkcs12_with_mac(pb, &mac_default);

    /* Read/decode */
    start_check_pkcs12_with_mac(pb, &mac_default);

        start_check_contentinfo_encrypted(pb, &enc_default);

            check_certbag(pb, CERT1, sizeof(CERT1), ATTRS1);
            check_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_single_secret_encrypted_content(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("1secret.p12");
    int custom_nid = get_custom_oid();

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_secretbag(pb, custom_nid, "VerySecretMessage", ATTRS1);

        end_contentinfo_encrypted(pb, &enc_default);

    end_pkcs12_with_mac(pb, &mac_default);

    /* Read/decode */
    start_check_pkcs12_with_mac(pb, &mac_default);

        start_check_contentinfo_encrypted(pb, &enc_default);

            check_secretbag(pb, custom_nid, "VerySecretMessage", ATTRS1);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_single_secret(PKCS12_ENC *enc)
{
    int custom_nid;
    char fname[80];
    PKCS12_BUILDER *pb;

    sprintf(fname, "1secret_ciph-%s_iter-%d.p12", OBJ_nid2sn(enc->nid), enc->iter);
    pb = new_pkcs12_builder(fname);
    custom_nid = get_custom_oid();

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_secretbag(pb, custom_nid, "VerySecretMessage", ATTRS1);

        end_contentinfo_encrypted(pb, enc);

    end_pkcs12_with_mac(pb, &mac_default);

    /* Read/decode */
    start_check_pkcs12_with_mac(pb, &mac_default);

        start_check_contentinfo_encrypted(pb, enc);

            check_secretbag(pb, custom_nid, "VerySecretMessage", ATTRS1);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_single_secret_enc_alg(int z)
{
    PKCS12_ENC enc;

    if (lgcyprov == NULL)
        enc.nid = enc_nids_no_legacy[z];
    else
        enc.nid = enc_nids_all[z];
    enc.pass = enc_default.pass;
    enc.iter = enc_default.iter;

    return test_single_secret(&enc);
}

static int test_multiple_contents(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("multi_contents.p12");
    int custom_nid = get_custom_oid();

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_certbag(pb, CERT1, sizeof(CERT1), ATTRS1);
            add_certbag(pb, CERT2, sizeof(CERT2), ATTRS2);
            add_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);
            add_keybag(pb, KEY2, sizeof(KEY2), ATTRS2, &enc_default);

        end_contentinfo(pb);

        start_contentinfo(pb);

            add_secretbag(pb, custom_nid, "VeryVerySecretMessage", ATTRS1);

        end_contentinfo_encrypted(pb, &enc_default);

    end_pkcs12_with_mac(pb, &mac_default);

    /* Read/decode */
    start_check_pkcs12_with_mac(pb, &mac_default);

        start_check_contentinfo(pb);

            check_certbag(pb, CERT1, sizeof(CERT1), ATTRS1);
            check_certbag(pb, CERT2, sizeof(CERT2), ATTRS2);
            check_keybag(pb, KEY1, sizeof(KEY1), ATTRS1, &enc_default);
            check_keybag(pb, KEY2, sizeof(KEY2), ATTRS2, &enc_default);

        end_check_contentinfo(pb);

        start_check_contentinfo_encrypted(pb, &enc_default);

            check_secretbag(pb, custom_nid, "VeryVerySecretMessage", ATTRS1);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_jdk_trusted_attr(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("jdk_trusted.p12");

    /* Generate/encode */
    start_pkcs12(pb);

        start_contentinfo(pb);

            add_certbag(pb, CERT1, sizeof(CERT1), ATTRS3);

        end_contentinfo(pb);

    end_pkcs12_with_mac(pb, &mac_default);

    /* Read/decode */
    start_check_pkcs12_with_mac(pb, &mac_default);

        start_check_contentinfo(pb);

            check_certbag(pb, CERT1, sizeof(CERT1), ATTRS3);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);
}

static int test_set0_attrs(void)
{
    PKCS12_BUILDER *pb = new_pkcs12_builder("attrs.p12");
    PKCS12_SAFEBAG *bag = NULL;
    STACK_OF(X509_ATTRIBUTE) *attrs = NULL;
    X509_ATTRIBUTE *attr = NULL;

    start_pkcs12(pb);

        start_contentinfo(pb);

            /* Add cert and attrs (name/locakkey only) */
            add_certbag(pb, CERT1, sizeof(CERT1), ATTRS4);

            bag = sk_PKCS12_SAFEBAG_value(pb->bags, 0);
            attrs = (STACK_OF(X509_ATTRIBUTE)*)PKCS12_SAFEBAG_get0_attrs(bag);

            /* Create new attr, add to list and confirm return attrs is not NULL */
            attr = X509_ATTRIBUTE_create(NID_oracle_jdk_trustedkeyusage, V_ASN1_OBJECT, OBJ_txt2obj("anyExtendedKeyUsage", 0));
            X509at_add1_attr(&attrs, attr);
            PKCS12_SAFEBAG_set0_attrs(bag, attrs);
            attrs = (STACK_OF(X509_ATTRIBUTE)*)PKCS12_SAFEBAG_get0_attrs(bag);
            X509_ATTRIBUTE_free(attr);
            if(!TEST_ptr(attrs)) {
                goto err;
            }

        end_contentinfo(pb);

    end_pkcs12(pb);

    /* Read/decode */
    start_check_pkcs12(pb);

        start_check_contentinfo(pb);

            /* Use existing check functionality to confirm cert bag attrs identical to ATTRS3 */
            check_certbag(pb, CERT1, sizeof(CERT1), ATTRS3);

        end_check_contentinfo(pb);

    end_check_pkcs12(pb);

    return end_pkcs12_builder(pb);

err:
    return 0;
}

#ifndef OPENSSL_NO_DES
static int pkcs12_create_test(void)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    PKCS12 *p12 = NULL;
    const unsigned char *p;

    static const unsigned char rsa_key[] = {
        0x30, 0x82, 0x02, 0x5d, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xbb,
        0x24, 0x7a, 0x09, 0x7e, 0x0e, 0xb2, 0x37, 0x32, 0xcc, 0x39, 0x67, 0xad,
        0xf1, 0x9e, 0x3d, 0x6b, 0x82, 0x83, 0xd1, 0xd0, 0xac, 0xa4, 0xc0, 0x18,
        0xbe, 0x8d, 0x98, 0x00, 0xc0, 0x7b, 0xff, 0x07, 0x44, 0xc9, 0xca, 0x1c,
        0xba, 0x36, 0xe1, 0x27, 0x69, 0xff, 0xb1, 0xe3, 0x8d, 0x8b, 0xee, 0x57,
        0xa9, 0x3a, 0xaa, 0x16, 0x43, 0x39, 0x54, 0x19, 0x7c, 0xae, 0x69, 0x24,
        0x14, 0xf6, 0x64, 0xff, 0xbc, 0x74, 0xc6, 0x67, 0x6c, 0x4c, 0xf1, 0x02,
        0x49, 0x69, 0xc7, 0x2b, 0xe1, 0xe1, 0xa1, 0xa3, 0x43, 0x14, 0xf4, 0x77,
        0x8f, 0xc8, 0xd0, 0x85, 0x5a, 0x35, 0x95, 0xac, 0x62, 0xa9, 0xc1, 0x21,
        0x00, 0x77, 0xa0, 0x8b, 0x97, 0x30, 0xb4, 0x5a, 0x2c, 0xb8, 0x90, 0x2f,
        0x48, 0xa0, 0x05, 0x28, 0x4b, 0xf2, 0x0f, 0x8d, 0xec, 0x8b, 0x4d, 0x03,
        0x42, 0x75, 0xd6, 0xad, 0x81, 0xc0, 0x11, 0x02, 0x03, 0x01, 0x00, 0x01,
        0x02, 0x81, 0x80, 0x00, 0xfc, 0xb9, 0x4a, 0x26, 0x07, 0x89, 0x51, 0x2b,
        0x53, 0x72, 0x91, 0xe0, 0x18, 0x3e, 0xa6, 0x5e, 0x31, 0xef, 0x9c, 0x0c,
        0x16, 0x24, 0x42, 0xd0, 0x28, 0x33, 0xf9, 0xfa, 0xd0, 0x3c, 0x54, 0x04,
        0x06, 0xc0, 0x15, 0xf5, 0x1b, 0x9a, 0xb3, 0x24, 0x31, 0xab, 0x3c, 0x6b,
        0x47, 0x43, 0xb0, 0xd2, 0xa9, 0xdc, 0x05, 0xe1, 0x81, 0x59, 0xb6, 0x04,
        0xe9, 0x66, 0x61, 0xaa, 0xd7, 0x0b, 0x00, 0x8f, 0x3d, 0xe5, 0xbf, 0xa2,
        0xf8, 0x5e, 0x25, 0x6c, 0x1e, 0x22, 0x0f, 0xb4, 0xfd, 0x41, 0xe2, 0x03,
        0x31, 0x5f, 0xda, 0x20, 0xc5, 0xc0, 0xf3, 0x55, 0x0e, 0xe1, 0xc9, 0xec,
        0xd7, 0x3e, 0x2a, 0x0c, 0x01, 0xca, 0x7b, 0x22, 0xcb, 0xac, 0xf4, 0x2b,
        0x27, 0xf0, 0x78, 0x5f, 0xb5, 0xc2, 0xf9, 0xe8, 0x14, 0x5a, 0x6e, 0x7e,
        0x86, 0xbd, 0x6a, 0x9b, 0x20, 0x0c, 0xba, 0xcc, 0x97, 0x20, 0x11, 0x02,
        0x41, 0x00, 0xc9, 0x59, 0x9f, 0x29, 0x8a, 0x5b, 0x9f, 0xe3, 0x2a, 0xd8,
        0x7e, 0xc2, 0x40, 0x9f, 0xa8, 0x45, 0xe5, 0x3e, 0x11, 0x8d, 0x3c, 0xed,
        0x6e, 0xab, 0xce, 0xd0, 0x65, 0x46, 0xd8, 0xc7, 0x07, 0x63, 0xb5, 0x23,
        0x34, 0xf4, 0x9f, 0x7e, 0x1c, 0xc7, 0xc7, 0xf9, 0x65, 0xd1, 0xf4, 0x04,
        0x42, 0x38, 0xbe, 0x3a, 0x0c, 0x9d, 0x08, 0x25, 0xfc, 0xa3, 0x71, 0xd9,
        0xae, 0x0c, 0x39, 0x61, 0xf4, 0x89, 0x02, 0x41, 0x00, 0xed, 0xef, 0xab,
        0xa9, 0xd5, 0x39, 0x9c, 0xee, 0x59, 0x1b, 0xff, 0xcf, 0x48, 0x44, 0x1b,
        0xb6, 0x32, 0xe7, 0x46, 0x24, 0xf3, 0x04, 0x7f, 0xde, 0x95, 0x08, 0x6d,
        0x75, 0x9e, 0x67, 0x17, 0xba, 0x5c, 0xa4, 0xd4, 0xe2, 0xe2, 0x4d, 0x77,
        0xce, 0xeb, 0x66, 0x29, 0xc5, 0x96, 0xe0, 0x62, 0xbb, 0xe5, 0xac, 0xdc,
        0x44, 0x62, 0x54, 0x86, 0xed, 0x64, 0x0c, 0xce, 0xd0, 0x60, 0x03, 0x9d,
        0x49, 0x02, 0x40, 0x54, 0xd9, 0x18, 0x72, 0x27, 0xe4, 0xbe, 0x76, 0xbb,
        0x1a, 0x6a, 0x28, 0x2f, 0x95, 0x58, 0x12, 0xc4, 0x2c, 0xa8, 0xb6, 0xcc,
        0xe2, 0xfd, 0x0d, 0x17, 0x64, 0xc8, 0x18, 0xd7, 0xc6, 0xdf, 0x3d, 0x4c,
        0x1a, 0x9e, 0xf9, 0x2a, 0xb0, 0xb9, 0x2e, 0x12, 0xfd, 0xec, 0xc3, 0x51,
        0xc1, 0xed, 0xa9, 0xfd, 0xb7, 0x76, 0x93, 0x41, 0xd8, 0xc8, 0x22, 0x94,
        0x1a, 0x77, 0xf6, 0x9c, 0xc3, 0xc3, 0x89, 0x02, 0x41, 0x00, 0x8e, 0xf9,
        0xa7, 0x08, 0xad, 0xb5, 0x2a, 0x04, 0xdb, 0x8d, 0x04, 0xa1, 0xb5, 0x06,
        0x20, 0x34, 0xd2, 0xcf, 0xc0, 0x89, 0xb1, 0x72, 0x31, 0xb8, 0x39, 0x8b,
        0xcf, 0xe2, 0x8e, 0xa5, 0xda, 0x4f, 0x45, 0x1e, 0x53, 0x42, 0x66, 0xc4,
        0x30, 0x4b, 0x29, 0x8e, 0xc1, 0x69, 0x17, 0x29, 0x8c, 0x8a, 0xe6, 0x0f,
        0x82, 0x68, 0xa1, 0x41, 0xb3, 0xb6, 0x70, 0x99, 0x75, 0xa9, 0x27, 0x18,
        0xe4, 0xe9, 0x02, 0x41, 0x00, 0x89, 0xea, 0x6e, 0x6d, 0x70, 0xdf, 0x25,
        0x5f, 0x18, 0x3f, 0x48, 0xda, 0x63, 0x10, 0x8b, 0xfe, 0xa8, 0x0c, 0x94,
        0x0f, 0xde, 0x97, 0x56, 0x53, 0x89, 0x94, 0xe2, 0x1e, 0x2c, 0x74, 0x3c,
        0x91, 0x81, 0x34, 0x0b, 0xa6, 0x40, 0xf8, 0xcb, 0x2a, 0x60, 0x8c, 0xe0,
        0x02, 0xb7, 0x89, 0x93, 0xcf, 0x18, 0x9f, 0x49, 0x54, 0xfd, 0x7d, 0x3f,
        0x9a, 0xef, 0xd4, 0xa4, 0x4f, 0xc1, 0x45, 0x99, 0x91,
    };

    p = rsa_key;
    if (!TEST_ptr(pkey = d2i_PrivateKey_ex(EVP_PKEY_RSA, NULL, &p,
                                           sizeof(rsa_key), NULL, NULL)))
        goto err;
    if (!TEST_int_eq(ERR_peek_error(), 0))
        goto err;
    p12 = PKCS12_create(NULL, NULL, pkey, NULL, NULL,
                        NID_pbe_WithSHA1And3_Key_TripleDES_CBC,
                        NID_pbe_WithSHA1And3_Key_TripleDES_CBC, 2, 1, 0);
    if (!TEST_ptr(p12))
        goto err;

    if (!TEST_int_eq(ERR_peek_error(), 0))
        goto err;
    ret = 1;
err:
    PKCS12_free(p12);
    EVP_PKEY_free(pkey);
    return ret;
}
#endif

typedef enum OPTION_choice {
    OPT_ERR = -1,
    OPT_EOF = 0,
    OPT_WRITE,
    OPT_LEGACY,
    OPT_CONTEXT,
    OPT_TEST_ENUM
} OPTION_CHOICE;

const OPTIONS *test_get_options(void)
{
    static const OPTIONS options[] = {
        OPT_TEST_OPTIONS_DEFAULT_USAGE,
        { "write",   OPT_WRITE,   '-', "Write PKCS12 objects to file" },
        { "legacy",  OPT_LEGACY,  '-', "Test the legacy APIs" },
        { "context", OPT_CONTEXT, '-', "Explicitly use a non-default library context" },
        { NULL }
    };
    return options;
}

int setup_tests(void)
{
    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_WRITE:
            PKCS12_helper_set_write_files(1);
            break;
        case OPT_LEGACY:
            PKCS12_helper_set_legacy(1);
            break;
        case OPT_CONTEXT:
            default_libctx = 0;
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    if (!default_libctx) {
        testctx = OSSL_LIB_CTX_new();
        if (!TEST_ptr(testctx))
            return 0;
        nullprov = OSSL_PROVIDER_load(NULL, "null");
        if (!TEST_ptr(nullprov))
            return 0;
    }

    deflprov = OSSL_PROVIDER_load(testctx, "default");
    if (!TEST_ptr(deflprov))
        return 0;
    lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");

    PKCS12_helper_set_libctx(testctx);

    /*
     * Verify that the default and fips providers in the default libctx are not
     * available if we are using a standalone context
     */
    if (!default_libctx) {
        if (!TEST_false(OSSL_PROVIDER_available(NULL, "default"))
                || !TEST_false(OSSL_PROVIDER_available(NULL, "fips")))
            return 0;
    }

    ADD_TEST(test_single_cert_no_attrs);
    if (lgcyprov == NULL) {
        ADD_ALL_TESTS(test_single_key_enc_alg, OSSL_NELEM(enc_nids_no_legacy));
        ADD_ALL_TESTS(test_single_secret_enc_alg, OSSL_NELEM(enc_nids_no_legacy));
    } else {
        ADD_ALL_TESTS(test_single_key_enc_alg, OSSL_NELEM(enc_nids_all));
        ADD_ALL_TESTS(test_single_secret_enc_alg, OSSL_NELEM(enc_nids_all));
    }
#ifndef OPENSSL_NO_DES
    if (default_libctx)
        ADD_TEST(pkcs12_create_test);
#endif
    ADD_ALL_TESTS(test_single_key_enc_pass, OSSL_NELEM(passwords));
    ADD_ALL_TESTS(test_single_key_enc_iter, OSSL_NELEM(iters));
    ADD_TEST(test_single_key_with_attrs);
    ADD_ALL_TESTS(test_single_cert_mac_alg, OSSL_NELEM(mac_nids));
    ADD_ALL_TESTS(test_single_cert_mac_pass, OSSL_NELEM(passwords));
    ADD_ALL_TESTS(test_single_cert_mac_iter, OSSL_NELEM(iters));
    ADD_TEST(test_cert_key_with_attrs_and_mac);
    ADD_TEST(test_cert_key_encrypted_content);
    ADD_TEST(test_single_secret_encrypted_content);
    ADD_TEST(test_multiple_contents);
    ADD_TEST(test_jdk_trusted_attr);
    ADD_TEST(test_set0_attrs);
    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(nullprov);
    OSSL_PROVIDER_unload(deflprov);
    OSSL_PROVIDER_unload(lgcyprov);
    OSSL_LIB_CTX_free(testctx);
}
