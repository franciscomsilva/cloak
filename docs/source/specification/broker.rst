======
Broker
======

Brokers mediate the connection between Cloak clients and
bridges. They are webservers that expose a number of endpoints
to both clients and bridges that are accessible through HTTP
requests.

They basically consist of a simple API which clients will use
to share a RID they are intending to join, alongside with the
RID’s corresponding password. The broker will find an available
bridge and forward that Chatroom ID so they can both join and
initiate a video call.

To avoid the possible block- age of the broker by a censor, in
the near future, we deploy a mechanism called `Domain
Fronting <https://petsymposium.org/2015/papers/03_Fifield.pdf>`__.
By offloading most of the heavy computing to the client side we
achieve two important goals:

#. Decrease the network and computing load on the bridges, making it so that more individuals can be motivated to deploy Cloak’s bridges since it will have a very low impact on their infrastructure and accompanying costs, and
#. By compacting the information shared between client and broker, we highly reduce the cost of deploying a solution with Domain Fronting, which, when used to transfer high amounts of data, can amount to a large monetary cost
