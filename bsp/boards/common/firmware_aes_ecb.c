/**************************************************************
AES128
Author:   Uli Kretzschmar
MSP430 Systems
Freising
AES software support for encryption and decryption
ECCN 5D002 TSU - Technology / Software Unrestricted
Source: http://is.gd/o9RSPq
**************************************************************/
#include <stdint.h>
#include "opendefs.h"
#include "firmware_aes_ecb.h"

// foreward sbox
const unsigned char sbox[256] = {
    //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, //0
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, //1
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, //2
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, //3
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, //4
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, //5
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, //6
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, //7
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, //8
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, //9
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, //A
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, //B
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, //C
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, //D
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, //E
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }; //F   
// round constant
const unsigned char Rcon[11] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };


// expand the key
void firmware_expandKey(unsigned char *expandedKey,
    unsigned char *key)
{
    unsigned short ii, buf1;
    for (ii = 0; ii<16; ii++)
        expandedKey[ii] = key[ii];
    for (ii = 1; ii<11; ii++){
        buf1 = expandedKey[ii * 16 - 4];
        expandedKey[ii * 16 + 0] = sbox[expandedKey[ii * 16 - 3]] ^ expandedKey[(ii - 1) * 16 + 0] ^ Rcon[ii];
        expandedKey[ii * 16 + 1] = sbox[expandedKey[ii * 16 - 2]] ^ expandedKey[(ii - 1) * 16 + 1];
        expandedKey[ii * 16 + 2] = sbox[expandedKey[ii * 16 - 1]] ^ expandedKey[(ii - 1) * 16 + 2];
        expandedKey[ii * 16 + 3] = sbox[buf1] ^ expandedKey[(ii - 1) * 16 + 3];
        expandedKey[ii * 16 + 4] = expandedKey[(ii - 1) * 16 + 4] ^ expandedKey[ii * 16 + 0];
        expandedKey[ii * 16 + 5] = expandedKey[(ii - 1) * 16 + 5] ^ expandedKey[ii * 16 + 1];
        expandedKey[ii * 16 + 6] = expandedKey[(ii - 1) * 16 + 6] ^ expandedKey[ii * 16 + 2];
        expandedKey[ii * 16 + 7] = expandedKey[(ii - 1) * 16 + 7] ^ expandedKey[ii * 16 + 3];
        expandedKey[ii * 16 + 8] = expandedKey[(ii - 1) * 16 + 8] ^ expandedKey[ii * 16 + 4];
        expandedKey[ii * 16 + 9] = expandedKey[(ii - 1) * 16 + 9] ^ expandedKey[ii * 16 + 5];
        expandedKey[ii * 16 + 10] = expandedKey[(ii - 1) * 16 + 10] ^ expandedKey[ii * 16 + 6];
        expandedKey[ii * 16 + 11] = expandedKey[(ii - 1) * 16 + 11] ^ expandedKey[ii * 16 + 7];
        expandedKey[ii * 16 + 12] = expandedKey[(ii - 1) * 16 + 12] ^ expandedKey[ii * 16 + 8];
        expandedKey[ii * 16 + 13] = expandedKey[(ii - 1) * 16 + 13] ^ expandedKey[ii * 16 + 9];
        expandedKey[ii * 16 + 14] = expandedKey[(ii - 1) * 16 + 14] ^ expandedKey[ii * 16 + 10];
        expandedKey[ii * 16 + 15] = expandedKey[(ii - 1) * 16 + 15] ^ expandedKey[ii * 16 + 11];
    }


}

// multiply by 2 in the galois field
unsigned char galois_mul2(unsigned char value)
{
    if (value >> 7)
    {
        value = value << 1;
        return (value ^ 0x1b);
    }
    else
        return value << 1;
}

// straight foreward aes encryption implementation
//   first the group of operations
//     - addroundkey
//     - subbytes
//     - shiftrows
//     - mixcolums
//   is executed 9 times, after this addroundkey to finish the 9th round, 
//   after that the 10th round without mixcolums
//   no further subfunctions to save cycles for function calls
//   no structuring with "for (....)" to save cycles
void firmware_aes_encr(unsigned char *state, unsigned char *expandedKey)
{
    unsigned char buf1, buf2, buf3, round;


    for (round = 0; round < 9; round++){
        // addroundkey, sbox and shiftrows
        // row 0
        state[0] = sbox[(state[0] ^ expandedKey[(round * 16)])];
        state[4] = sbox[(state[4] ^ expandedKey[(round * 16) + 4])];
        state[8] = sbox[(state[8] ^ expandedKey[(round * 16) + 8])];
        state[12] = sbox[(state[12] ^ expandedKey[(round * 16) + 12])];
        // row 1
        buf1 = state[1] ^ expandedKey[(round * 16) + 1];
        state[1] = sbox[(state[5] ^ expandedKey[(round * 16) + 5])];
        state[5] = sbox[(state[9] ^ expandedKey[(round * 16) + 9])];
        state[9] = sbox[(state[13] ^ expandedKey[(round * 16) + 13])];
        state[13] = sbox[buf1];
        // row 2
        buf1 = state[2] ^ expandedKey[(round * 16) + 2];
        buf2 = state[6] ^ expandedKey[(round * 16) + 6];
        state[2] = sbox[(state[10] ^ expandedKey[(round * 16) + 10])];
        state[6] = sbox[(state[14] ^ expandedKey[(round * 16) + 14])];
        state[10] = sbox[buf1];
        state[14] = sbox[buf2];
        // row 3
        buf1 = state[15] ^ expandedKey[(round * 16) + 15];
        state[15] = sbox[(state[11] ^ expandedKey[(round * 16) + 11])];
        state[11] = sbox[(state[7] ^ expandedKey[(round * 16) + 7])];
        state[7] = sbox[(state[3] ^ expandedKey[(round * 16) + 3])];
        state[3] = sbox[buf1];

        // mixcolums //////////
        // col1
        buf1 = state[0] ^ state[1] ^ state[2] ^ state[3];
        buf2 = state[0];
        buf3 = state[0] ^ state[1]; buf3 = galois_mul2(buf3); state[0] = state[0] ^ buf3 ^ buf1;
        buf3 = state[1] ^ state[2]; buf3 = galois_mul2(buf3); state[1] = state[1] ^ buf3 ^ buf1;
        buf3 = state[2] ^ state[3]; buf3 = galois_mul2(buf3); state[2] = state[2] ^ buf3 ^ buf1;
        buf3 = state[3] ^ buf2;     buf3 = galois_mul2(buf3); state[3] = state[3] ^ buf3 ^ buf1;
        // col2
        buf1 = state[4] ^ state[5] ^ state[6] ^ state[7];
        buf2 = state[4];
        buf3 = state[4] ^ state[5]; buf3 = galois_mul2(buf3); state[4] = state[4] ^ buf3 ^ buf1;
        buf3 = state[5] ^ state[6]; buf3 = galois_mul2(buf3); state[5] = state[5] ^ buf3 ^ buf1;
        buf3 = state[6] ^ state[7]; buf3 = galois_mul2(buf3); state[6] = state[6] ^ buf3 ^ buf1;
        buf3 = state[7] ^ buf2;     buf3 = galois_mul2(buf3); state[7] = state[7] ^ buf3 ^ buf1;
        // col3
        buf1 = state[8] ^ state[9] ^ state[10] ^ state[11];
        buf2 = state[8];
        buf3 = state[8] ^ state[9];   buf3 = galois_mul2(buf3); state[8] = state[8] ^ buf3 ^ buf1;
        buf3 = state[9] ^ state[10];  buf3 = galois_mul2(buf3); state[9] = state[9] ^ buf3 ^ buf1;
        buf3 = state[10] ^ state[11]; buf3 = galois_mul2(buf3); state[10] = state[10] ^ buf3 ^ buf1;
        buf3 = state[11] ^ buf2;      buf3 = galois_mul2(buf3); state[11] = state[11] ^ buf3 ^ buf1;
        // col4
        buf1 = state[12] ^ state[13] ^ state[14] ^ state[15];
        buf2 = state[12];
        buf3 = state[12] ^ state[13]; buf3 = galois_mul2(buf3); state[12] = state[12] ^ buf3 ^ buf1;
        buf3 = state[13] ^ state[14]; buf3 = galois_mul2(buf3); state[13] = state[13] ^ buf3 ^ buf1;
        buf3 = state[14] ^ state[15]; buf3 = galois_mul2(buf3); state[14] = state[14] ^ buf3 ^ buf1;
        buf3 = state[15] ^ buf2;      buf3 = galois_mul2(buf3); state[15] = state[15] ^ buf3 ^ buf1;

    }
    // 10th round without mixcols
    state[0] = sbox[(state[0] ^ expandedKey[(round * 16)])];
    state[4] = sbox[(state[4] ^ expandedKey[(round * 16) + 4])];
    state[8] = sbox[(state[8] ^ expandedKey[(round * 16) + 8])];
    state[12] = sbox[(state[12] ^ expandedKey[(round * 16) + 12])];
    // row 1
    buf1 = state[1] ^ expandedKey[(round * 16) + 1];
    state[1] = sbox[(state[5] ^ expandedKey[(round * 16) + 5])];
    state[5] = sbox[(state[9] ^ expandedKey[(round * 16) + 9])];
    state[9] = sbox[(state[13] ^ expandedKey[(round * 16) + 13])];
    state[13] = sbox[buf1];
    // row 2
    buf1 = state[2] ^ expandedKey[(round * 16) + 2];
    buf2 = state[6] ^ expandedKey[(round * 16) + 6];
    state[2] = sbox[(state[10] ^ expandedKey[(round * 16) + 10])];
    state[6] = sbox[(state[14] ^ expandedKey[(round * 16) + 14])];
    state[10] = sbox[buf1];
    state[14] = sbox[buf2];
    // row 3
    buf1 = state[15] ^ expandedKey[(round * 16) + 15];
    state[15] = sbox[(state[11] ^ expandedKey[(round * 16) + 11])];
    state[11] = sbox[(state[7] ^ expandedKey[(round * 16) + 7])];
    state[7] = sbox[(state[3] ^ expandedKey[(round * 16) + 3])];
    state[3] = sbox[buf1];
    // last addroundkey
    state[0] ^= expandedKey[160];
    state[1] ^= expandedKey[161];
    state[2] ^= expandedKey[162];
    state[3] ^= expandedKey[163];
    state[4] ^= expandedKey[164];
    state[5] ^= expandedKey[165];
    state[6] ^= expandedKey[166];
    state[7] ^= expandedKey[167];
    state[8] ^= expandedKey[168];
    state[9] ^= expandedKey[169];
    state[10] ^= expandedKey[170];
    state[11] ^= expandedKey[171];
    state[12] ^= expandedKey[172];
    state[13] ^= expandedKey[173];
    state[14] ^= expandedKey[174];
    state[15] ^= expandedKey[175];
}

/**
\brief Basic AES encryption of a single 16-octet block.
\param[in,out] buffer Single block plaintext. Will be overwritten by ciphertext.
\param[in] key Buffer containing the secret key (16 octets).

\returns E_SUCCESS when the encryption was successful. 
*/
owerror_t firmware_aes_ecb_enc(uint8_t buffer[16], uint8_t key[16])
{
    uint8_t expandedKey[176];

    firmware_expandKey(expandedKey, key);       // expand the key into 176 bytes
    firmware_aes_encr(buffer, expandedKey);

    return E_SUCCESS;
}

