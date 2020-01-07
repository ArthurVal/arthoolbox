#!/usr/bin/env python3
"""conftest.py file use to configurate pytest by adding hooks/params...
"""
import pytest

###############################################################################
#                                PYTEST - HOOKS                               #
def pytest_configure(config):
    """
    Allows plugins and conftest files to perform initial configuration.

    This hook is called for every plugin and initial conftest file
    after command line options have been parsed.

    After that, the hook is called for other conftest files as they are
    imported.

    .. note::
        This hook is incompatible with ``hookwrapper=True``.

    :arg _pytest.config.Config config: pytest config object
    """
    import logging.config, yaml

    with open('tests/logging.yaml', 'rt') as log_yml_file:
        config = yaml.safe_load(log_yml_file.read())
        logging.config.dictConfig(config)


def pytest_sessionstart(session):
    """ called after the ``Session`` object has been created and before performing collection
    and entering the run test loop.

    :param _pytest.main.Session session: the pytest session object
    """

def pytest_collection(session):
    """Perform the collection protocol for the given session.

    Stops at first non-None result, see :ref:`firstresult`.

    :param _pytest.main.Session session: the pytest session object
    """

def pytest_collection_modifyitems(session, config, items):
    """ called after collection has been performed, may filter or re-order
    the items in-place.

    :param _pytest.main.Session session: the pytest session object
    :param _pytest.config.Config config: pytest config object
    :param List[_pytest.nodes.Item] items: list of item objects
    """

def pytest_collection_finish(session):
    """ called after collection has been performed and modified.

    :param _pytest.main.Session session: the pytest session object
    """

def pytest_runtestloop(session):
    """ called for performing the main runtest loop
    (after collection finished).

    Stops at first non-None result, see :ref:`firstresult`

    :param _pytest.main.Session session: the pytest session object
    """

def pytest_runtest_protocol(item, nextitem):
    """ implements the runtest_setup/call/teardown protocol for
    the given test item, including capturing exceptions and calling
    reporting hooks.

    :arg item: test item for which the runtest protocol is performed.

    :arg nextitem: the scheduled-to-be-next test item (or None if this
                   is the end my friend).  This argument is passed on to
                   :py:func:`pytest_runtest_teardown`.

    :return boolean: True if no further hook implementations should be invoked.


    Stops at first non-None result, see :ref:`firstresult` """

def pytest_runtest_logstart(nodeid, location):
    """ signal the start of running a single test item.

    This hook will be called **before** :func:`pytest_runtest_setup`, :func:`pytest_runtest_call` and
    :func:`pytest_runtest_teardown` hooks.

    :param str nodeid: full id of the item
    :param location: a triple of ``(filename, linenum, testname)``
    """

def pytest_runtest_setup(item):
    """ called before ``pytest_runtest_call(item)``. """

def pytest_runtest_call(item):
    """ called to execute the test ``item``. """

def pytest_pyfunc_call(pyfuncitem):
    """ call underlying test function.

    Stops at first non-None result, see :ref:`firstresult` """

def pytest_runtest_teardown(item, nextitem):
    """ called after ``pytest_runtest_call``.

    :arg nextitem: the scheduled-to-be-next test item (None if no further
                   test item is scheduled).  This argument can be used to
                   perform exact teardowns, i.e. calling just enough finalizers
                   so that nextitem only needs to call setup-functions.
    """

def pytest_runtest_logfinish(nodeid, location):
    """ signal the complete finish of running a single test item.

    This hook will be called **after** :func:`pytest_runtest_setup`, :func:`pytest_runtest_call` and
    :func:`pytest_runtest_teardown` hooks.

    :param str nodeid: full id of the item
    :param location: a triple of ``(filename, linenum, testname)``
    """

def pytest_runtest_makereport(item, call):
    """ return a :py:class:`_pytest.runner.TestReport` object
    for the given :py:class:`pytest.Item <_pytest.main.Item>` and
    :py:class:`_pytest.runner.CallInfo`.

    Stops at first non-None result, see :ref:`firstresult`
    """

def pytest_unconfigure(config):
    """ called before test process is exited.

    :param _pytest.config.Config config: pytest config object
    """

def pytest_sessionfinish(session, exitstatus):
    """ called after whole test run finished, right before returning the exit status to the system.

    :param _pytest.main.Session session: the pytest session object
    :param int exitstatus: the status which pytest will return to the system
    """


###############################################################################
#                            TEST - GLOBAL FIXTURES                           #

