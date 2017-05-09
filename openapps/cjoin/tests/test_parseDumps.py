import ctypes
import cbor
import binascii

import pytest

#============================ defines ===============================

COSE_KEYSET_MAX_NUM_KEYS = 2
SHORT_ADDRESS_LENGTH     = 2
COSE_SYMKEY_LENGTH       = 16
COSE_SYMKEY_KEYID_LENGTH = 1

# from http://stackoverflow.com/questions/24307022/how-to-compare-two-ctypes-objects-for-equality
class CtStruct(ctypes.Structure):
    def __eq__(self, other):
        for field in self._fields_:
            attr_name = field[0]
            a, b = getattr(self, attr_name), getattr(other, attr_name)
            is_array = isinstance(a, ctypes.Array)
            if is_array and a[:] != b[:] or not is_array and a != b:
                return False
        return True

    def __ne__(self, other):
        for field in self._fields_:
            attr_name = field[0]
            a, b = getattr(self, attr_name), getattr(other, attr_name)
            is_array = isinstance(a, ctypes.Array)
            if is_array and a[:] != b[:] or not is_array and a != b:
                return True
        return False

class asn_t(CtStruct):
    _pack_ = 1
    _fields_ = [
        ("byte4", ctypes.c_uint8),
        ("bytes2and3", ctypes.c_uint16),
        ("bytes0and1", ctypes.c_uint16),
    ]

    def print_values(self):
        print(format(self.byte4,'02x'), format(self.bytes2and3, '02x'), format(self.bytes0and1,'02x'))

class short_address_t(CtStruct):
    _fields_ = [
        ("address", ctypes.c_uint8 * SHORT_ADDRESS_LENGTH),
        ("lease_asn", asn_t),
    ]

    def print_values(self):
        print("short address: " + ''.join(format(x, '02x') for x in self.address))
        self.lease_asn.print_values()

class COSE_symmetric_key_t(CtStruct):
    _fields_ = [
        ("kty", ctypes.c_int),  # enum
        ("kid", ctypes.c_uint8 * COSE_SYMKEY_KEYID_LENGTH),
        ("k", ctypes.c_uint8 * COSE_SYMKEY_LENGTH),
    ]

    def print_values(self):
        print "kty: " + str(self.kty)
        print "kid: " + ''.join(format(x, '02x') for x in self.kid)
        print "k: " + ''.join(format(x, '02x') for x in self.k)

class COSE_keyset_t(CtStruct):
    _fields_ = [
        ("key", COSE_symmetric_key_t * COSE_KEYSET_MAX_NUM_KEYS),
    ]
    
    def print_values(self):
        for key in self.key:
            print "================"
            key.print_values()

class join_response_t(CtStruct):
    _fields_ = [
        ("keyset", COSE_keyset_t),
        ("short_address", short_address_t),
        ]
    def print_values(self):
        self.keyset.print_values()
        print "================"
        self.short_address.print_values()

#============================ fixtures ==============================

CBORDUMPSANDPARSED = [
    (
        binascii.unhexlify('8281a301040241012050e6bf4287c2d7618d6a9687445ffd33e68142af93'),
        ( 0, # return from parsing (success)
            join_response_t( # parsed object
                COSE_keyset_t( 
                    (COSE_symmetric_key_t * COSE_KEYSET_MAX_NUM_KEYS) (
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x01),
                                k = (ctypes.c_uint8 * 16) (0xe6, 0xbf, 0x42, 0x87, 0xc2, 0xd7, 0x61, 0x8d, 0x6a, 0x96, 0x87, 0x44, 0x5f, 0xfd, 0x33, 0xe6),
                            ),
                            COSE_symmetric_key_t(),
                    )
                ),
                short_address_t( 
                    (ctypes.c_uint8 * SHORT_ADDRESS_LENGTH)(0xaf, 0x93)
                )
            ),
        )
    ),
    (
        binascii.unhexlify('8181a301040241012050e6bf4287c2d7618d6a9687445ffd33e6'),
        ( 0, # return from parsing (success)
            join_response_t( # parsed object
                COSE_keyset_t( 
                    (COSE_symmetric_key_t * COSE_KEYSET_MAX_NUM_KEYS) (
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x01),
                                k = (ctypes.c_uint8 * 16) (0xe6, 0xbf, 0x42, 0x87, 0xc2, 0xd7, 0x61, 0x8d, 0x6a, 0x96, 0x87, 0x44, 0x5f, 0xfd, 0x33, 0xe6),
                            ),
                            COSE_symmetric_key_t(),
                    )
                ),
            ),
        )
    ),
    (
       binascii.unhexlify('8181a60104024101030c04840304090a0550abcdabcdabcdabcdabcdabcdabcdabcd2050e6bf4287c2d7618d6a9687445ffd33e6'), # key ops, alg and base iv present but ignored
        ( 0, # return from parsing (success)
            join_response_t( # parsed object
                COSE_keyset_t( 
                    (COSE_symmetric_key_t * COSE_KEYSET_MAX_NUM_KEYS) (
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x01),
                                k = (ctypes.c_uint8 * 16) (0xe6, 0xbf, 0x42, 0x87, 0xc2, 0xd7, 0x61, 0x8d, 0x6a, 0x96, 0x87, 0x44, 0x5f, 0xfd, 0x33, 0xe6),
                            ),
                            COSE_symmetric_key_t(),
                    )
                ),
            ),
        )
    ),
    (
        binascii.unhexlify('8182a301040241012050e6bf4287c2d7618d6a9687445ffd33e6a301040241022050deadbeefdeadbeefdeadbeefdeadbeef'),
        ( 0, # return from parsing (success)
            join_response_t( # parsed object
                COSE_keyset_t( 
                    (COSE_symmetric_key_t * COSE_KEYSET_MAX_NUM_KEYS) (
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x01),
                                k = (ctypes.c_uint8 * 16) (0xe6, 0xbf, 0x42, 0x87, 0xc2, 0xd7, 0x61, 0x8d, 0x6a, 0x96, 0x87, 0x44, 0x5f, 0xfd, 0x33, 0xe6),
                            ),
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x02),
                                k = (ctypes.c_uint8 * 16) (0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef),
                            ),
                    )
                ),
            ),
        )
    ),
    (
        binascii.unhexlify('8282a301040241012050e6bf4287c2d7618d6a9687445ffd33e6a301040241022050deadbeefdeadbeefdeadbeefdeadbeef8142af93'),
        ( 0, # return from parsing (success)
            join_response_t( # parsed object
                COSE_keyset_t( 
                    (COSE_symmetric_key_t * COSE_KEYSET_MAX_NUM_KEYS) (
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x01),
                                k = (ctypes.c_uint8 * 16) (0xe6, 0xbf, 0x42, 0x87, 0xc2, 0xd7, 0x61, 0x8d, 0x6a, 0x96, 0x87, 0x44, 0x5f, 0xfd, 0x33, 0xe6),
                            ),
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x02),
                                k = (ctypes.c_uint8 * 16) (0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef),
                            ),
                    )
                ),
                short_address_t( 
                    (ctypes.c_uint8 * SHORT_ADDRESS_LENGTH)(0xaf, 0x93)
                )
            ),
        )
    ),
    (
        binascii.unhexlify('8282a301040241012050e6bf4287c2d7618d6a9687445ffd33e6a301040241022050deadbeefdeadbeefdeadbeefdeadbeef8242af9345deadbeefab'),
        ( 0, # return from parsing (success)
            join_response_t( # parsed object
                COSE_keyset_t( 
                    (COSE_symmetric_key_t * COSE_KEYSET_MAX_NUM_KEYS) (
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x01),
                                k = (ctypes.c_uint8 * 16) (0xe6, 0xbf, 0x42, 0x87, 0xc2, 0xd7, 0x61, 0x8d, 0x6a, 0x96, 0x87, 0x44, 0x5f, 0xfd, 0x33, 0xe6),
                            ),
                            COSE_symmetric_key_t(
                                kty = 4,
                                kid = (ctypes.c_uint8 * 1)(0x02),
                                k = (ctypes.c_uint8 * 16) (0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef),
                            ),
                    )
                ),
                short_address_t( 
                    (ctypes.c_uint8 * SHORT_ADDRESS_LENGTH)(0xaf, 0x93),
                    (asn_t)(ctypes.c_uint8(0xde),ctypes.c_uint16(0xadbe),ctypes.c_uint16(0xefab))
                )
            ),
        )
    ),
    (
        binascii.unhexlify('8281a301040241012050e6bf4287c2d7618d6a9687445ffd33e68142'),
        ( 1, # return from parsing (fail)
            join_response_t(), # parsed object (zeroed out)
        )
    ),
    (
        binascii.unhexlify('81a301040241012050e6bf4287c2d7618d6a9687445ffd33e68142af93'),
        ( 1, # return from parsing (fail)
            join_response_t(), # parsed object (zeroed out)
        )
    ),
    (
        binascii.unhexlify('8281a3010402410120508142af93'),
        ( 1, # return from parsing (fail)
            join_response_t(), # parsed object (zeroed out)
        )
    ),
    (
        binascii.unhexlify('8281a3010402410120508142af93'),
        ( 1, # return from parsing (fail)
            join_response_t(), # parsed object (zeroed out)
        )
    ),

]

@pytest.fixture(params=CBORDUMPSANDPARSED)
def cborDumpsAndParsed(request):
    return request.param

#============================ tests =================================

def test_parseCborDumps(cborDumpsAndParsed):

    (input, output) = cborDumpsAndParsed

    test_lib = ctypes.cdll.LoadLibrary('libcbor.so')

    dump = [ord(b) for b in input]
    arr = (ctypes.c_uint8 * len(dump))(*dump)
    plen = ctypes.c_uint8(len(dump))

    result = join_response_t()

    ret = test_lib.cbor_parse_join_response(ctypes.byref(result), ctypes.byref(arr), plen)

    assert ret == output[0]
    assert result == output[1]


def main():
    lib = ctypes.cdll.LoadLibrary('libcbor.so')
    join_response_parsed = join_response_t()

    join_response_serialized = binascii.unhexlify('8281a3010402410120508142af93')

    respPayload     = [ord(b) for b in join_response_serialized]
    arr = (ctypes.c_uint8 * len(respPayload))(*respPayload)
    plen = ctypes.c_uint8(len(respPayload))

    print lib.cbor_parse_join_response(ctypes.byref(join_response_parsed), ctypes.byref(arr), plen)

    #test_jr.print_values()
    join_response_parsed.print_values()


if __name__ == '__main__':
    main()

