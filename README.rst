============
python-chirp
============

Message-passing for everyone

.. TODO Add real readthedocs

|travis| |rtd| |mpl| |works|

.. |travis|  image:: https://travis-ci.org/concretecloud/python-chirp.svg?branch=master
   :target: https://travis-ci.org/concretecloud/python-chirp
.. |rtd| image:: https://1042.ch/ganwell/docs-master.svg
   :target: https://1042.ch/chirp/
.. |mpl| image:: https://img.shields.io/badge/license-MPL%202.0-blue.svg
   :target: http://mozilla.org/MPL/2.0/
.. |works| image:: https://img.shields.io/badge/hypothesis-works-blue.svg
   :target: http://hypothesis.works

`Read the Docs`_

.. _`Read the Docs`: http://1042.ch/chirp/

BETA-RELEASE: 0.2.0
===================

Features
========

* Fully automatic connection setup

* TLS support

  * Connections to 127.0.0.1 and ::1 aren't encrypted

* Easy message routing

* Robust

  * No message can be lost without an error (in sync mode)

* Very thin API

* Minimal code-base, all additional features will be implemented as modules in
  an upper layer

* Fast

Install
=======

Dependencies
------------

.. code-block:: bash

   Alpine:         apk add libffi-dev libressl-dev libuv-dev
   Debian-based:   apt install libffi-dev libssl-dev libuv1-dev
   Redhat-based:   yum install libffi-devel openssl-devel libuv-devel
   Arch:           pacman -S libffi openssl libuv
   OSX:            brew install libffi openssl libuv

pip
---

If we have wheels for your platform, you don't need to install any dependencies.

.. code-block:: bash

   pip install libchirp

setup.py
--------

.. code-block:: bash

   pip install cffi
   python setup.py install
