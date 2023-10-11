============
Design Goals
============

The primary objective of Cloak is to securely bypass Tor
traffic censorship imposed by adversaries using advanced
traffic analysis techniques. Such an adversary is assumed to
actively attempt to detect streams that make use of Cloak to
bypass its censorship mechanisms and actively disrupt and tear
down those streams. The design of Cloak is driven by the
following sub-goals:

#. **Unobservability:** Cloak aims to ensure that censors cannot distinguish between regular WebRTC videocall streams and streams carrying covert data. By concealing the true nature of the traffic, Cloak thwarts attempts by adversaries to detect and disrupt its usage.
#. **Unblockability:** To maintain a robust and effective solution, Cloak is engineered to cause significant collateral damage to a country's social and economic status should a censor attempt to block the carrier WebRTC application on which Cloak relies. This feature makes blocking the system economically and politically unfavorable for censors.
#. **Video-Carrier Independence:** Cloak's encoding strategy is designed to be flexible and adaptable, allowing it to work seamlessly with any WebRTC-based carrier application. This approach ensures that the system is not tied to a specific video carrier, enhancing its versatility and potential for widespread adoption.
#. **Reasonable Performance:** While prioritizing censorship resistance, Cloak also emphasizes achieving reasonable performance levels. The system aims to support most typical Internet tasks, including exchanging emails, uploading files, and streaming standard to high-resolution videos, without compromising user experience.
#. **Uphold Tor's Anonymity Properties:** As Cloak utilizes the Tor traffic to bypass censorship, it is of utmost importance that the system upholds the security and privacy features of the Tor Network. Maintaining the anonymity of users and safeguarding the integrity of the Tor infrastructure remain critical aspects of Cloak's design.

By focusing on these design goals, Cloak offers a powerful
solution to circumvent censorship and uphold the principles of
online privacy and freedom.
