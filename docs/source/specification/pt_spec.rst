=================================
Pluggable Transport Specification
=================================

.. container:: section
      :name: pt-spec

      .. rubric:: 7. Pluggable Transport Specification
         :name: pluggable-transport-specification

      Cloak is designed as a Tor pluggable transport (PT),
      conforming to the PT specification outlined by the Tor
      developers. This approach allows seamless integration into the
      Tor network without requiring modifications to the existing Tor
      protocols. The interaction between Tor and Cloak follows the
      PT API.

      Cloak's lifecycle as a PT:

      #. **Instance Configuration:** Each Tor instance loads a
         configuration file (torrc) containing the path to the
         Cloak executable, command line arguments, and the Tor
         mode of operation (e.g., bridge or client).
      #. **Cloak Subprocess:** Upon launch, Tor starts Cloak as
         a subprocess, passing essential configuration variables to
         it through command line arguments. These variables expose
         Tor configurations, including the bridge remote address.
      #. **Communication with Tor:** Cloak reports its status back
         to Tor by writing success or error messages to the stdout in
         a standardized format. Tor captures and reads these
         messages, establishing communication with Cloak.
      #. **Controlled Shutdown:** Tor performs a controlled shutdown
         of Cloak to ensure proper memory management. It
         accomplishes this by closing the stdin of Cloak. In
         response, Cloak initiates a clean shutdown of all its
         components.

      **WebRTC Video Call Session:** Apart from the initial
      configuration, Cloak is responsible for establishing a
      remote WebRTC video call session connection with the Cloak
      bridge. This connection enables the secure transmission of
      covert Tor data. The local Tor client fully manages Cloak,
      automatically configuring all options and preferences without
      any user interaction.
