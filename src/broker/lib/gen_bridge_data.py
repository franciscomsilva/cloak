import string
import random
import pickle
from uuid import uuid4
from pycountry import countries
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import serialization, hashes

ID_LENGTH = 25
BRIDGE_NUMBER = 10
COUNTRY_LIST = []

def create_bridge_list(n, l=ID_LENGTH):
    bridge_list = []
    with open('bridges.txt','w') as ofile:
        for i in range(n):
            session_id = str(uuid4())
            chat_name = ''.join(random.choices(string.ascii_letters + string.digits, k=l))
            location = random.choice(list(countries)).name
            private_key = rsa.generate_private_key(
                public_exponent=65537,
                key_size=2048,
            )
            public_key = private_key.public_key()

            serialized_priv_key = private_key.private_bytes(encoding=serialization.Encoding.PEM,
                                                            format=serialization.PrivateFormat.PKCS8,
                                                            encryption_algorithm=serialization.NoEncryption())
            
            serialized_pub_key = public_key.public_bytes(encoding=serialization.Encoding.PEM,
                                                         format=serialization.PublicFormat.PKCS1)
            
            cName_signature = private_key.sign(
                bytes(chat_name, encoding='UTF-8'),
                padding.PSS(
                mgf=padding.MGF1(hashes.SHA256()),
                salt_length=padding.PSS.MAX_LENGTH
                ),
                hashes.SHA256()
            )

            bridge_list.append({
                'ID': session_id,
                'Location': location,
                'Key': serialized_pub_key
                })
            
            ofile.write('BRIDGE_{}:\nID: {}\ncNAME: {}\nLOCATION: {}\nPUB_KEY:\n{}PRIV_KEY:\n{}SIGNED cNAME:\n{}\n\n'.format(
                i+1,
                session_id,
                chat_name,
                location,
                serialized_pub_key.decode(),
                serialized_priv_key.decode(),
                cName_signature
                ))
            
        ofile.close()

    with open('bridges.pickle','wb') as ofile:
        pickle.dump(bridge_list,ofile)
        ofile.close()

    return

if __name__ == '__main__':
    create_bridge_list(BRIDGE_NUMBER)
