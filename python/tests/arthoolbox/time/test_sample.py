#!/usr/bin/env python3
import time, math
from . import log as parent_log, sample

module_log = parent_log.getChild('sample')

def test_sample_attr(function, log = module_log.getChild('attr')):
    log.info("{0:*^50}".format(" Testing attributes "))
    for attr_name in [
            "last_call",
            "period",
            "__wrapped__",
    ]:
        assert hasattr(function, attr_name), \
            "Sampled function should have {} attribute"\
            .format(attr_name)
    log.info("{0:*^50}".format(" DONE "))


def test_sample_attr_init(function, log = module_log.getChild('attr_init')):
    log.info("{0:*^50}".format(" Testing initial values "))
    for attr_name, expected_init_value in [
            ("last_call", None),
    ]:
        log.debug(
            "{}: {} (expecting: {})".format(
                attr_name,
                getattr(function, attr_name),
                expected_init_value
            )
        )
        assert getattr(function, attr_name) == expected_init_value, \
            "Sampled function attr {} should be equals to {} initially"\
            .format(attr_name, expected_init_value)
    log.info("{0:*^50}".format(" DONE "))


def test_sample_output(function, input_args, log = module_log.getChild('output')):
    log.info("{0:*^50}".format(" Testing output determinism "))
    for input_used in input_args:
        output_expected = function.__wrapped__(input_used)
        output_obtained = function(input_used)
        log.debug(
            "sampled f({}) = {} (expecting: {})"
            .format(input_used, output_obtained, output_expected)
        )

        assert output_obtained == output_expected,\
            ("Sampled function output should be the same when sampled and"
             " when not (input used: {} | ouput expected: {} | "
             "ouput obtained: {})").format(
                 input_used,
                 output_expected,
                 output_obtained,
             )
    log.info("{0:*^50}".format(" DONE "))


def test_sample_computation(function, input_args, log = module_log.getChild('computation')):
    log.info("{0:*^50}".format(" Testing sample expected computations "))
    sleep_time_sec = 50e-3

    for arg in input_args:
        function(arg)
        time.sleep(sleep_time_sec)

    log.debug(
        "period.number_of_measurement: {} (expecting: {})".format(
            function.period.number_of_measurement,
            (len(input_args) - 1)
        )
    )
    assert function.period.number_of_measurement == (len(input_args) - 1), \
        "Sampled function number_of_measurement {} should be equals to {}"\
        .format(function.period.number_of_measurement, len(input_args) - 1)

    log.debug("mean: {}".format(function.period.mean))
    log.debug("variance: {}".format(function.period.variance))
    log.debug("sampled_variance: {}".format(function.period.sampled_variance))

    assert math.isclose(function.period.mean,
                        sleep_time_sec,
                        rel_tol = 0.05), \
        ("Sampled function computed period ({}) should be equals to the"
         " sleeping time used ({})")\
        .format(function.period.mean, sleep_time_sec)

    log.info("{0:*^50}".format(" DONE "))



 # TODO:

 # - ?
