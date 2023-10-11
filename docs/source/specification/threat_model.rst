============
Threat Model
============

In the context of Cloak, the goal of the adversary is to
detect and block the usage of the system, without jeopardizing
legitimate WebRTC connections that can be vital to the
country’s economy. We assume that the adversary is a
state-level censor, able to observe, store, interfere with, and
analyze all the network traffic of the Internet infrastructure
originated from Cloak endpoints, if within the censor’s
jurisdiction. The adversary is also able to block generalized
access to remote Internet services it deems sensitive, such as
the Tor Network. The censor is considered to have advanced
tools based on Deep Packet Inspection (DPI) and statistical
traffic analysis to detect and block these services.

However, several attacks are out of scope. We assume that the
censor does not have control and access over the used WebRTC
gateways. In other words, the censor cannot observe the video
streams and evaluate whether the video streams contain covert
data or not. Secondly, we deem the adversary to be
computationally bounded and unable to decrypt any encrypted
traffic for services it does not control, such as Tor Traffic.
The adversary’s control is also limited to the network: it has
no control over the software installed on end-user computers
and does not have the power to deploy rogue software on these
machines, with the purpose of monitoring systems on network
edges. Thus, Cloak’s users and bridges are assumed to be
executing trusted software. Also, as mentioned earlier, the
adversary will only seek to rapidly disrupt and tear down
traffic which is suspected of carrying covert channels, and it
will refrain from blocking the carrier application altogether,
avoiding the blockage of an important and highly used service
by the population and damaging its economy. Lastly, we also
assume the censor has no control over the Tor network and its
infrastructure, and so, cannot easily control or observe any of
the traffic after it enters or exits the Tor Network.
