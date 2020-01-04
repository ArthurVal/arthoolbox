#!/usr/bin/env python3

import logging, math
import toolbox.math.stats as stats

stats_log = logging.getLogger('test.stats')

def test_mean(log = stats_log.getChild('mean')):
    log.info("{0:*^50}".format(" Testing mean computation "))
    assert 5.0 == stats.mean([5]*50),\
        "Mean computed on constant array of 5 should be 5"

    assert 25.0 == stats.mean(range(1, 50)),\
        "Mean computed on array of range(1, 50) should be 25"
    log.info("{0:*^50}".format(" DONE "))


def test_variance(log = stats_log.getChild('variance')):
    pass
