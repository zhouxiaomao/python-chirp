============
python-chirp
============

Message-passing for everyone

|travis| |rtd| |mpl| |works|

.. |travis|  image:: https://travis-ci.org/concretecloud/python-chirp.svg?branch=master
   :target: https://travis-ci.org/concretecloud/python-chirp
.. |rtd| image:: https://1042.ch/ganwell/docs-master.svg
   :target: https://docs.adfinis-sygroup.ch/public/python-chirp
.. |mpl| image:: https://img.shields.io/badge/license-MPL%202.0-blue.svg
   :target: http://mozilla.org/MPL/2.0/
.. |works| image:: https://img.shields.io/badge/hypothesis-works-blue.svg
   :target: http://hypothesis.works

`Read the Docs`_

.. _`Read the Docs`: https://docs.adfinis-sygroup.ch/public/python-chirp

.. raw:: html

    <img alt="chirp"
    src="https://raw.githubusercontent.com/concretecloud/chirp/master/doc/_static/chirp.png"
    width="33%">

Supported by |adsy|

.. |adsy| image:: https://1042.ch/ganwell/adsy-logo.svg
   :target: https://adfinis-sygroup.ch/

BETA-RELEASE: 1.1.2
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

   Alpine:       apk add python3-dev libffi-dev libressl-dev libuv-dev
   Debian-based: apt install python3-dev libffi-dev libssl-dev libuv1-dev
   Redhat-based: yum install python3-devel libffi-devel openssl-devel libuv-devel
   Arch:         pacman -S libffi openssl libuv
   OSX:          brew install libffi openssl libuv

As long as libchirp is beta we only provide wheels for Windows: building on
Windows is very complex.

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


Changes
=======

2018-03-20 - 1.1.1b0
---------------------

* Update to libchirp.c 1.1.0-beta

2018-03-21 - 1.1.2b0
--------------------

* Update to libchirp.c 1.1.1-beta

  * Fixes deadlock in send

  * Fixes bug when disabling signals

* Disable signals

License
=======

libchirp is subject to the terms of the Mozilla Public License, v. 2.0. Creating
a "Larger Work" under the GNU (Lesser) General Public License is explicitly
allowed. Contributors to libchirp must agree to the Mozilla Public License, v.
2.0.

Contributing
============

Please open an issue first. Contributions of missing features are very welcome, but
we want to keep to scope of libchirp minimal, so additional features should
probably be implemented in an upper layer.

Most valuable contributions:

* Use it and report issues

* Fix documentation

* Promote libchirp

* Contribute any kind of tests or fuzzing (if possible hypothesis_ based)

* Make bindings for your favorite language

* Make packages for your favorite distribution

* A uvloop_ based implementation, see also issue_

.. _uvloop: https://github.com/MagicStack/uvloop

.. _issue: https://github.com/MagicStack/uvloop/issues/52

.. _hypothesis: https://hypothesis.readthedocs.io/en/latest/
