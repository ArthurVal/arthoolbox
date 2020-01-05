#!/usr/bin/env python3

from . import log as parent_log, logging
import math
import toolbox.math.stats.discrete as stats

module_log = parent_log.getChild('discrete')

def test_mean(log = module_log.getChild('mean')):
    log.info("{0:*^50}".format(" Testing discrete mean computation "))
    assert 5.0 == stats.mean([5]*50),\
        "Mean computed on constant array of 5 should be 5"

    assert 25.0 == stats.mean(range(1, 50)),\
        "Mean computed on array of range(1, 50) should be 25"
    log.info("{0:*^50}".format(" DONE "))


def test_variance(log = module_log.getChild('variance')):
    log.info("{0:*^50}".format(" Testing discrete variance computation "))

    log.debug("Generating a gaussian function (for fun)")

    # Gaussian parameters
    gaussian_esp = 50.00
    log.debug("Expected value used: {}".format(gaussian_esp))
    gaussian_stddev = 5.00
    log.debug("Stddev used: {}".format(gaussian_stddev))

    # pre computation constants
    gaussian_exp_cst = - 1 /(2 * math.pow(gaussian_stddev, 2))
    gaussian_cst = 1 / (gaussian_stddev * math.sqrt(2 * math.pi))

    # gaussian function
    gaussian_function = lambda x: \
        (gaussian_cst * math.exp(math.pow((x - gaussian_esp), 2) * gaussian_exp_cst))

    # Input used
    # --Params

    gaussian_x_step = 1
    gaussian_x_n_points = 1e6

    gaussian_x_min = -int(gaussian_x_n_points/2)
    gaussian_x_max = int(gaussian_x_n_points/2)

    gaussian_x = list(map(lambda x: x*gaussian_x_step ,range(gaussian_x_min, gaussian_x_max, 1)))

    log.debug(
        "Input: {} points ([{} ; {}]) with step of {}"
        .format(
            len(gaussian_x),
            gaussian_x[0],
            gaussian_x[-1],
            gaussian_x_step
        )
    )


    # Output gaussian function
    gaussian_y = list(map(gaussian_function, gaussian_x))


    # Weigthed gaussian
    gaussian_w = list(map(lambda x, y: (x*y), gaussian_x, gaussian_y))

    arithmetic_mean = stats.mean(gaussian_y)
    stddev_computed = stats.stddev(gaussian_y, arithmetic_mean)

    log.debug("Mean x: {}".format(stats.mean(gaussian_x)))
    log.debug("Mean y: {}".format(stats.mean(gaussian_y)))
    log.debug("Mean w: {}".format(stats.mean(gaussian_w)))
    log.debug("esperance: {}".format(math.fsum(gaussian_w) / math.fsum(gaussian_y)))

    log.debug("Sum y: {}".format(math.fsum(gaussian_y)))

    log.debug("mean computed: {}".format(arithmetic_mean))
    log.debug("stddev computed: {}".format(stddev_computed))


    # import matplotlib.pyplot as plt
    # import numpy as np
    # plt.plot(np.array(gaussian_x), np.array(gaussian_y))
    # plt.xlabel('x')
    # plt.ylabel('Gaussian')
    # plt.title("Gaussian function with Esp = {} & Stddev = {}"
    #           .format(gaussian_esp, gaussian_stddev))
    # plt.legend()
    # plt.show()
    # weigthed_mean = math.fsum(list(map(lambda x, y: (x*y), gaussian_x, gaussian_y))) / math.fsum(gaussian_y)
    # log.debug("Expected value computed: {}".format(weigthed_mean))


    log.info("{0:*^50}".format(" DONE "))


