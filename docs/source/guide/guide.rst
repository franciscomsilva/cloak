============================
Start here: setting up Cloak
============================
This guide will help you setup Cloak and use it as a Pluggable Transport in Tor Browser to acess the Tor Network.

.. note::
    Cloak has only been tested with the configuration provided below. Others may work but are not officially supported.


.. contents::


System Requirements 
====================
Our setup is designed to run 2 virtual machines with **8 vCPUs and 8GB of RAM** each. However, **4 vCPUs and 8GB of RAM** per machine should be more than enough. Your host
machine should be able to handle this.


Supported Operating System 
--------------------------
Cloak is currently only supported by 64-bit Linux Distributions, such as Ubuntu or Debian. The minimum required Linux Kernel Version is 4.15.

.. note::
    Specifically, Cloak was developed and tested under Ubuntu Bionic 18.04.5 LTS (4.15.0-112-generic).

Virtualization Requirements
----------------------------

Our setup requires Virtualization to be enabled on the host machine. Specific instructions on how to enable Virtualization can be found in the following links:

**Linux:** `link <https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/5/html/virtualization/sect-virtualization-troubleshooting-enabling_intel_vt_and_amd_v_virtualization_hardware_extensions_in_bios>`_
**Windows:** `link <https://support.microsoft.com/en-us/windows/enable-virtualization-on-windows-11-pcs-c5578302-6e43-4b4b-a449-8ced115f58e1>`_
**MacOS:**  `link <https://kb.parallels.com/en/5653>`_

.. note::
    If you are running Cloak inside a Virtual Machine, you need to ensure Nested Virtualization is enabled. **We have not tested Cloak using this feature**.
    However, you can find instructions on how to enable it here for `VMWare <https://communities.vmware.com/t5/Nested-Virtualization-Documents/Running-Nested-VMs/ta-p/2781466>`_ and `Virtualbox <https://docs.oracle.com/en/virtualization/virtualbox/6.0/admin/nested-virt.html>`_.


Software Requirements
---------------------
Our setup expects a host machine with `Vagrant <https://developer.hashicorp.com/vagrant/docs/installation>`_ and `Virtualbox <https://www.geeksforgeeks.org/how-to-install-virtualbox-in-linux/>`_ installed.


Configuring Vagrant
--------------------
To use your Vagrantfile please install the following plugin:

.. code-block:: console

    vagrant plugin install vagrant-disksize


Starting the Virtual Machines
==============================

**1.** Start by downloading the configuration package from our server using this `link <https://turbina.gsd.inesc-id.pt/resources/cloak_test_setup.tar.gz>`_.


.. note::
    SHA256 Checksum Hash of **cloak_test_setup.tar.gz**: 
    ``9627c2b915a5cd5425900466a286513374f741c02a9debff322fec1bc4b8ca8a``

**2.** Extract the file to your working directory using: 

.. code-block:: console

    tar -xf cloak_test_setup.tar.gz

**3.** You should see two files: ``cloak.box`` and a ``Vagrantfile`` .

**4.** Add our custom box to your local vagrant repository using:

.. code-block:: console

    vagrant box add cloak cloak.box

**5.** After the box has been added, you can now create the machines using ``vagrant up`` (make sure you are in the same directory as the previously downloaded Vagrantfile).


The Vagrantfile comprises provisioning routines which will fetch and configure all the required software. This takes some time, so go and grab a coffee while you wait.

Using the Virtual Machines
==========================

Once the machines are created and booted up, reload them using:

.. code-block:: console

    vagrant reload

Then use the following commands to connect to them:

.. code-block:: console

    vagrant ssh cloakclient
    vagrant ssh cloakbridge

Open your Virtualbox GUI and open each VM's Graphical Environment. This will be necessary for future steps.

Testing Cloak with Tor Browser
==============================

Using the scripts
------------------

On the ``cloakbridge`` machine execute the following commands:

.. code-block:: console

    cd cloak/scripts 
    python bridge.py

You should see an output indicating the start of a Flask API. 

Now, on the ``cloakclient`` machine, do the following commands:

.. code-block:: console

    cd cloak/scripts 
    python client.py

You should see things happening. This will take care of the necessary procedures to execute Cloak.

Log in to meet.jit.si 
----------------------

Due to `changes on Jitsi's side <https://jitsi.org/blog/authentication-on-meet-jit-si/>`_, you need to log in to their service to be able to create a video chat room. This is a one-time configuration as the credentials are saved.

- You should see a Chrome Browser Tab started and a **prompt asking you to Log In**. **Click it** and use your chosen service to login in to the service.

Using Tor Browser with Cloak
-----------------------------

After login, you are now ready to use and test Cloak. Follow these steps:

**1.** Hit ``CTRL+C`` on the ``cloakclient`` running script to reset the configurations. You should see all the windows closing.

**2.** Once again, execute ``python client.py`` on the ``cloakclient`` machine. After a few seconds, you should see a Chrome Window starting up. When the webpage is loaded, the script should automatically click on the ``Join`` button.

**3.** After it clicked to join, if you see the prompt to Log in again, just click on ``Login`` once more. If you did this before, it is normal that this prompt does not show up and you simply join the room. 

**4.** Next, the script will automate the bridge joining the room and starting the video call.

**4.** You can now switch to the Tor Browser window, click on ``Connect`` and confirm you are connected to the Tor Network. You can now browse freely! Enjoy!


.. note::
    The ``Login`` prompt might appear from time to time. You just need to click it to add the Session Token to the Browser's storage.