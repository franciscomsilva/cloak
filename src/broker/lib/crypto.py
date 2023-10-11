from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.exceptions import InvalidSignature

def validate_sig(msg, sig, key_bytes):
    pub_key = serialization.load_pem_public_key(
        key_bytes,
    )

    try:
        pub_key.verify(
            sig,
            bytes(msg,encoding='ASCII'),
            padding.PSS(
            mgf=padding.MGF1(hashes.SHA256()),
            salt_length=padding.PSS.MAX_LENGTH
            ),
            hashes.SHA256()
        )
        return True
    
    except InvalidSignature:
        return False
    
if __name__ == '__main__':
    print(validate_sig('kxIAiwkiDiaLloEKl47PKQDtP',
                       b'\x96\xfa9\nc]\xce(`+\x07_\x95\x0b.\xb5\xec\\j\xe1\xab\xa1\xa6\x1c*\xcb\x00\x07\xe7\x8dJh\xea\xc1\x9e-\xc3\x04\xbb\x1e\xe4\xbd\xbb\xc3\xfd\xaa\xf7\xb2\xa8-\x16#\xe7\x9e\x99C\x98\xe3\xb4\xc6d\x12\xad8<\xef\xb4DL\xbb\x86\xa0\xb3f\xe1\xe9\xfd\xadvj@q\xcb\xa1\x9db\x81P\x8c\xe3\xacQ\xb7\xbby\xbd\x18"\x0en,\xef\xd4m\x88|#\xa2=\xe1"\xd8l\x85\t\t+\x9aF\xf3~\x9c\xb0H\x93\x97\xa9 \xff?\x8doof8\xe0\xd6=\x07\x85\xb6\x01\xe2\xdc6\x8f\x14\xb6U8\x1a\xf3\x83\xd2q\xadk\x9dMJc8\xba\xfe\xb2z\xed\n\xd9\xfd\xba\x7fM\xf1\x83\xfa\xa1\x08~.B\xc1\xeb\xee\xd1\x84\x87\xc2_S\xbd1\xadYQ\xca1w8e>}\xdb\xdb\xafY\xb0C3_\x13\xd5O\xcc\xdd\'\xc1Eh\xf9\x02\xf44\xf8\xcf\xb8\xc7i\n\xf4\xff\xd1\xe0V\xb6\xfc\xdc\xce02\x83\xb7\x1a\xa0a\x94"\xb2\x96q\x7f\xa9\x1dkQ\x90',
                       b'-----BEGIN RSA PUBLIC KEY-----\nMIIBCgKCAQEApOdXW10Wmsj8jsfb+TPheMViqGlbFaSOxzxtTdRZ1BCa/Y/msP82\n/XT+4+DFUgEE+ZmpVC68HMFrjKf1vNtGlI8/qX2zZK4OCM6/ffHB+3gyqsFmI6lO\nTaEltYSK0f8BbUyC2wA0BAgWOyI2Zw7vZC0shxAj1nL5JgqcU6kkNruowsTwa4kx\n7hHpUFgIiSmlx+jSPFNMY2hB9XBar/Aex++gCbg/hE7IAaH0/L3tJdy9SQZWaWMz\n6uF5tCaJBoEmJR/ohpFMdaVnE+BOAhO2LeAU/OjHSQY/24FYbo6JVmZG3Z4ckIlS\nwVRApFWQzIAFb4rNk/Iuu3tE+WMv9j04owIDAQAB\n-----END RSA PUBLIC KEY-----\n',
                       ))