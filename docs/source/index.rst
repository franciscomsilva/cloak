.. Cloak Pluggable Transport documentation master file, created by
   sphinx-quickstart on Mon Sep 18 11:37:37 2023.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Cloak: Tor Pluggable Transport
=====================================================
Cloak was designed to address the challenges posed by state-level adversaries seeking to block access to the Tor anonymity network. 
Implemented as a new Tor pluggable transport, Cloak offers robust resistance against sophisticated traffic analysis attacks, safeguarding 
user privacy and enabling anonymous communication.

.. toctree::
   :caption: Getting Started
   :maxdepth: 1

   guide/guide

.. toctree::
   :caption: Formal Specification
   :maxdepth: 1

   specification/introduction
   specification/design_goals
   specification/threat_model
   specification/architecture
   specification/system_features
   specification/broker
   specification/pt_spec
   specification/socks_proto

.. toctree::
   :caption: Development
   :maxdepth: 1
   :includehidden:

   
   documentation/documentation





