#!/usr/bin/env python3
import statistics as st, math
from . import log as parent_log

import arthoolbox.math.statistics as arthstats

module_log = parent_log.getChild('statistics')

def test_update_mean(data):
    log = module_log.getChild('update_mean')
    log.info("{0:*^50}".format(" Testing update_mean function "))

    log.debug("Use the update_mean function on {} data".format(len(data)))
    computed_mean = 0
    for i, single_data in enumerate(data, start = 1):

        computed_mean = arthstats.update_mean(
            new_data = single_data,
            old_mean = computed_mean,
            num_data = i
        )

        log.debug("{} -> X:{} | Mean: {}".format(i, single_data, computed_mean))

        assert math.isclose(st.mean(data[:(i)]), computed_mean, rel_tol = 0.005), \
            ("The recursive mean computed on step n = {} doesn't match the"
             " arithmetic mean computed with statistics.mean").format(i)


    log.info("{0:*^50}".format(" DONE "))


def test_update_variance(data):
    log = module_log.getChild('update_variance')
    log.info("{0:*^50}".format(" Testing update_variance function "))

    log.debug("Use the update_variance function on {} data".format(len(data)))
    i_mean, i_1_mean, computed_variance = 0.0, data[0], 0.0
    for i, single_data in enumerate(data[1:], start = 2):

        i_mean = arthstats.update_mean(
            new_data = single_data,
            old_mean = i_1_mean,
            num_data = i
        )

        computed_variance, i_1_mean = (
            arthstats.update_variance(
                new_data = single_data,
                old_variance = computed_variance,
                new_mean = i_mean,
                old_mean = i_1_mean,
                num_data = i
            ),
            i_mean
        )

        log.debug(
            ("{} -> X:{} | Mean: {} | Variance: {}")
            .format(i, single_data, i_mean, computed_variance)
        )

        assert math.isclose(
            st.pvariance(data[:(i)], mu = i_mean),
            computed_variance,
            rel_tol = 0.005), \
            ("The recursive variance computed on step n = {} doesn't match the"
             " arithmetic variance computed with statistics.pvariance")\
             .format(i)

    log.info("{0:*^50}".format(" DONE "))


def test_update_sum_squares(data):
    log = module_log.getChild('update_sum_squares')
    log.info("{0:*^50}".format(" Testing update_sum_squares function "))

    log.debug(
        "Use the update_sum_squares function on {} data".format(len(data))
    )
    i_mean, i_1_mean, computed_sum_squares = 0.0, data[0], 0.0
    for i, single_data in enumerate(data[1:], start = 2):

        i_mean = arthstats.update_mean(
            new_data = single_data,
            old_mean = i_1_mean,
            num_data = i
        )

        computed_sum_squares, i_1_mean = (
            arthstats.update_sum_squares(
                new_data = single_data,
                old_sum_squares = computed_sum_squares,
                new_mean = i_mean,
                old_mean = i_1_mean,
            ),
            i_mean
        )

        computed_variance, computed_sample_variance = \
            (computed_sum_squares / i), (computed_sum_squares / (i-1))

        log.debug(
            ("{} -> X:{} | Mean: {} | sum_squares: {} | variance: {} | "
             "sample variance: {}")
            .format(
                i,
                single_data,
                i_mean,
                computed_sum_squares,
                computed_variance,
                computed_sample_variance,
            )
        )

        assert math.isclose(
            st.pvariance(data[:(i)], mu = i_mean),
            computed_variance,
            rel_tol = 0.005), \
            ("The recursive sum_squares computed on step n = {} doesn't match "
             "the arithmetic variance computed with statistics.pvariance")\
             .format(i)

        assert math.isclose(
            st.variance(data[:(i)], xbar = i_mean),
            computed_sample_variance,
            rel_tol = 0.005), \
            ("The recursive sampled variance computed on step n = {} doesn't "
             "match the arithmetic variance computed with statistics.variance")\
             .format(i)

    log.info("{0:*^50}".format(" DONE "))
