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
    for i, single_data in enumerate(data):


        computed_mean = arthstats.update_mean(
            new_data = single_data,
            old_mean = computed_mean,
            num_data = i
        )

        log.debug("{} -> X:{} | Mean: {}".format(i, single_data, computed_mean))

        assert math.isclose(st.mean(data[:(i+1)]), computed_mean, rel_tol = 0.005), \
            ("The recursive mean computed on step n = {} don't match the"
             " arithmetic mean computed with statistics.mean").format(i)


    log.info("{0:*^50}".format(" DONE "))
