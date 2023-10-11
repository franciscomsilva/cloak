from lib.database import get_database

# Some queries that might be useful

bridge_info = get_database()

def exists_bridge(id):
    return bridge_info.find_one({"ID": id}) != None

def get_pub_key(id):
    return bridge_info.find_one({"ID": id})["Key"]

def get_location(id):
    return bridge_info.find_one({"ID": id})["Location"]

def find_by_location(location):
    return bridge_info.find({'Location': location})

# Region should be a list of countries
def find_by_region(region):
    return bridge_info.find({'Location': {'$in': region}})