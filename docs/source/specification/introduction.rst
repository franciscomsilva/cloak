============
Introduction
============

Purpose
=======

This specification defines the requirements for Cloak, a
pluggable transport for the Tor network designed to securely
bypass Tor traffic censorship imposed by state-level
adversaries using advanced traffic analysis techniques.


Motivation
==========

Cloak was designed to address the challenges posed by
state-level adversaries seeking to block access to the Tor
anonymity network. Implemented as a new Tor pluggable
transport, Cloak offers robust resistance against
sophisticated traffic analysis attacks, safeguarding user
privacy and enabling anonymous communication.

At its core, Cloak establishes covert channels between Tor
clients and Cloak bridges, acting as proxies to the free Tor
network. These covert channels are created by concealing Tor
traffic within the video streams of widely-used web
conferencing services based on WebRTC technology. By leveraging
this approach, Cloak evades detection by
government-controlled ISPs and censors, as it appears as
regular video call traffic. Cloak users can thus freely
access Tor services, such as SecureDrop, without being blocked
or detected, ensuring unrestricted and anonymous communication.

Furthermore, Cloak's bridge distribution infrastructure
plays a crucial role in enhancing practicality and scalability.
It allows Tor users to easily discover and connect to bridges,
mitigating potential limitations of other existing solutions.

Cloak was designed to be a step forward in the battle
against censorship, empowering users to communicate securely
and protect their online privacy. As a research prototype, it
seeks to contribute to the continuous endeavor to fortify
anonymity, promote freedom of expression and paving the way for
an open and unrestricted digital environment."

Notation and Terminology
========================

* **API:** Application Programming Interface
* **DPI:** Deep Packet Inspection
* **EFB:** Encoded Frame Bitstream
* **HTTP:** Hypertext Transfer Protocol
* **ISP:** Internet Service Provider
* **PLI:** Picture Loss Indication
* **PT:** Pluggable Transport
* **RID:** Rendezvous ID
* **RTP:** Real-time Transport Protocol
* **RTCP:** RTP Control Protocol
* **SRTP:** Secure Real-time Transport Protocol
* **WebRTC:** Web Real-Time Communication