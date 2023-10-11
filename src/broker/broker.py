from queue import Queue, Empty
from flask import Flask, request
from lib import queries, crypto
from time import time
from uuid import uuid4

app = Flask(__name__)

# Main method for landing page and health check
@app.route('/', methods=['GET'])
def healthCheck():
    return "<h1>TorCloak Broker API</h1>", 200

# Endpoint used by bridges to query for new clients
@app.route('/bridge', methods=['POST'])
def bridgePollClients():
    # Check for  X-Session-ID header
    bridge_id = request.headers.get('X-Session-ID')
    bridge_cName = request.headers.get('cName')
    timeout = request.headers.get('timeout')
    bridge_signature = request.get_data()

    if not bridge_id or not bridge_cName or not bridge_signature:
        return 'One or more elements missing', 400

    # Select the appropriate timeout (default: 10s)
    # TODO: clean hardcoded value
    if timeout:
        deadline = time() + int(timeout)
    else:
        deadline = time() + 10

    # Check if bridge ID exists and get its key if it does
    if not queries.exists_bridge(bridge_id):
        return "Unauthorized", 401
    else:
        pub_key = queries.get_pub_key(bridge_id)

    # Check if siganture is valid
    if not crypto.validate_sig(bridge_cName, bridge_signature, pub_key):
        return 'Invalid Signature', 401

    # At this point the bridge is valid and will wait for an available client
    print(f'{"[BRIDGE]":<9}{"Getting Offer ":>17}. . .')
    while time() <= deadline:
        # We try to get an offer and check if it is still active
        try:
            offer = offers.get_nowait()
            print(f'{"[BRIDGE]":<9}{"Checking ":>17}{str(offer["ID"]):<37}')
            if time() > offer['expires']:
                print(f'{"[BRIDGE]":<9}{"Offer ":>17}{str(offer["ID"]):<37}{"EXPIRED":<7}')
                return f'Offer {str(offer["ID"])} expired', 410
            else:
                # If the offer is still active we answer it and return the chatroom data
                print(f'{"[BRIDGE]":<9}{"Offer ":>17}{str(offer["ID"]):<37}{"SUCCESS":<7}')
                answers[offer['ID']] = bridge_cName
                return [offer['cID'], offer['cPass']], 200
        except Empty:
            continue
    return 'No offers available', 410        

# Endpoint used by clients to post their offers
@app.route('/client', methods=['POST'])
def clientPostOffer():
    cID = request.headers.get('cID')
    cPass = request.headers.get('cPass')
    timeToLive = request.headers.get('ttl')
    timeout = request.headers.get('timeout')

    if not cID or not cPass or not timeToLive:
        return 'One or more elements missing', 400
    
    timeToLive = int(timeToLive)

    # Select the appropriate timeout (default: 20s)
    # TODO: clean hardcoded value 
    if timeout:
        timeout = int(timeout)
        deadline = time() + max(timeout, timeToLive)
    else:
        deadline = time() + max(20, timeToLive)

    # Create an offer with a unique ID and expiration time
    offer_id = uuid4()
    expiration = time() + timeToLive
    offer = {'ID': offer_id,
             'cID': cID,
             'cPass': cPass,
             'expires': expiration
             }

    print(f'{"[CLIENT]":<9}{"Publishing Offer ":>17}{str(offer_id):<37}')
    offers.put(offer)

    # Wait for a validated cName for the chosen amount of time
    print(f'{"[CLIENT]":<9}{"Getting answer ":>17}{str(offer_id):<37}')
    while time() < deadline:
        try:
            answer = answers.pop(offer_id)
            print(f'{"[CLIENT]":<9}{"Offer ":>17}{str(offer_id):<37}{"SUCCESS":<9}')
            return answer, 200
        except KeyError:
            continue
    print(f'{"[CLIENT]":<9}{"Offer ":>17}{str(offer_id):<37}{"TIMED OUT":<9}')
    return f'Offer {offer_id} timed out', 408
        
if __name__ == "__main__":
    # Queue() is thread-safe (not process-safe)
    offers = Queue()
    # Dictionaries are not process-safe and not thread-safe (for GET operations)
    # However, since offer ids are unique and only 1 client thread performs the GET 
    # for that specific id it should be ok...
    answers = dict()
    app.run(debug=False, host='0.0.0.0', port=5005)