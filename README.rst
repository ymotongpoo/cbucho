`cbucho` is a package for C/API exercises. Yes, we love bucho!

Setup
-----

This module is C porting of `"bucho" <https://bitbucket.org/ae35/bucho>`_, one of the most famous Python modules.

This module requires 2 libraries:

* `libcurl <http://curl.haxx.se/libcurl/>`_
* `libxml2 <http://xmlsoft.org/index.html>`_

After installing libraries above, just run setup.py::

  $ python setup.py build
  $ python setup.py install

History
-------

0.0.2 (2011-05-08)
~~~~~~~~~~~~~~~~~~

- latest_status(), all_status() is implemented


0.0.1 (2011-05-05)
~~~~~~~~~~~~~~~~~~

- first release
- show() is implemented
