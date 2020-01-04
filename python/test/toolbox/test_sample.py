#!/usr/bin/env python3
import logging, time, math
from toolbox.sample import sample

sample_log = logging.getLogger('test.sample')

def test_sample_attr(function, log = sample_log.getChild('attr')):
    log.info("{0:*^50}".format(" Testing attributes "))
    for attr_name in [
            "_n_call",
            "_updated",
            "_n_measurement",
            "_last_period",
            "_mean_period",
            "_variance_period",
            "_stddev_period",
            "_reset_sampling",
            "_sample__function",
            "_sample__get_time",
            "_sample__last_call",
            "_sample__variance_tmp",
    ]:
        assert hasattr(function, attr_name), \
            "Sampled function should have {} attribute"\
            .format(attr_name)
    log.info("{0:*^50}".format(" DONE "))


def test_sample_attr_init(function, log = sample_log.getChild('attr_init')):
    log.info("{0:*^50}".format(" Testing initial values "))
    for attr_name, expected_init_value in [
            ("_n_call", 0),
            ("_n_measurement", 0),
            ("_last_period", None),
            ("_mean_period", None),
            ("_variance_period", None),
            ("_stddev_period", None),
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


def test_sample_output(function, input_args = range(50), log = sample_log.getChild('output')):
    log.info("{0:*^50}".format(" Testing output determinism "))
    for input_used in input_args:
        output_expected = function._sample__function(input_used)
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


def test_sample_computation(function, input_args = range(50), log = sample_log.getChild('computation')):
    log.info("{0:*^50}".format(" Testing sample expected computations "))
    sleep_time_sec = 50e-3

    for arg in input_args:
        function(arg)
        time.sleep(sleep_time_sec)

    for attr_name, expected_value in [
            ("_n_call", len(input_args)),
            ("_n_measurement", len(input_args) - 1),
    ]:
        log.debug(
            "{}: {} (expecting: {})".format(
                attr_name,
                getattr(function, attr_name),
                expected_value
            )
        )
        assert getattr(function, attr_name) == expected_value, \
            "Sampled function attr {} should be equals to {}"\
            .format(attr_name, expected_value)

    log.debug("_variance_period: {}".format(function._variance_period))
    log.debug("_stddev_period: {}".format(function._stddev_period))
    assert math.isclose(function._variance_period,
                        function._stddev_period*function._stddev_period,
                        rel_tol = 0.05), \
        ("Sampled function computed variance ({}) should be the square of "
         " the stddev ({})")\
        .format(function._variance_period, function._stddev_period)

    log.debug("_mean_period: {}".format(function._mean_period))
    assert math.isclose(function._mean_period,
                        sleep_time_sec,
                        rel_tol = 0.05), \
        ("Sampled function computed period ({}) should be equals to the"
         " sleeping time used ({})")\
        .format(function._mean_period, sleep_time_sec)
    log.info("{0:*^50}".format(" DONE "))


