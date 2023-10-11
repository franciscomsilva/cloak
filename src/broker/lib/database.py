import argparse
import pickle
from pymongo import MongoClient


def get_database():
    # Provide the mongodb atlas url to connect python to mongodb using pymongo
    CONNECTION_STRING = "127.0.0.1"

    # Create a connection using MongoClient. You can import MongoClient or use pymongo.MongoClient
    client = MongoClient(CONNECTION_STRING)
    database = client["cloak"]
    collection = database["bridge_info"]

    # Create the database for our example
    return collection

def populate(data):
    # Get the bridge collection
    bridges_info = get_database()

    # Populate the collection
    bridges_info.insert_many(data)
    return

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=str, help='The pickle file with the bridge data')

    args = parser.parse_args()
    path = args.file

    f = open(path,'rb')
    bridges = pickle.load(f)
    f.close()

    # Populate the database with a List of bridges 
    populate(bridges)
