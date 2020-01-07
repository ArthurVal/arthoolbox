#!/usr/bin/env python3

"""Module containing function used for statistical computations on discrete sets

Function list:
- mean
- variance
- stddev
"""
import math

def mean(values):
    """Compute the arithmetical mean of an input array of values

    Attributs
    ---------
    values : iterable(numbers)
        The input list of values we need to compute the mean of

    Returns
    -------
    float
        The mean computed
    """
    return (math.fsum(values) / len(values))


def variance(values, mean_computed = None):
    """Compute the variance of an input array of values

    Attributs
    ---------
    values : list(numbers)
        The input list of values we need to compute the mean of
    mean_computed : float or None
        The mean computed, None to compute it with mean() function

    Returns
    -------
    float
        The variance computed
    """
    if not mean_computed:
        mean_computed = mean(values)

    return (math.fsum(map(lambda x: math.pow(x, 2), values)) / len(values)) - math.pow(mean_computed, 2)


def stddev(values, mean_computed = None):
    """Compute the standard deviation of an input array of values

    Attributs
    ---------
    values : list(numbers)
        The input list of values we need to compute the mean of
    mean_computed : float or None
        The mean computed, None to compute it with mean() function

    Returns
    -------
    float
        The stddev computed
    """
    return math.sqrt(variance(values, mean_computed))
