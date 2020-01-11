#!/usr/bin/env python3
"""Module containing specifics statistics function like recursive computation...

Functions list:
- update_mean()        | Compute an updated mean
- update_variance()    | Compute a new variance
- update_sum_squares() | Compute a new sum of squares of differences from mean

Classes list:
- OnlineVariableStatistics | Store the mean of a variable computed recursively
"""
import threading                # Condition
import collections              # namedtupled

def update_mean(new_data, old_mean, num_data):
    """Compute a new mean recursively using the old mean and new measurement

    From the arithmetic mean computed using the n-1 measurements (M_n-1), we
    compute the new mean (M_n) adding a new measurement (X_n) with the formula:
    M_n = M_n-1 + (X_n - M_n-1)/n

    See: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance

    Parameters
    ----------
    new_data: int or decimal
        The new measurement Xn
    old_mean: int or decimal
        The mean Mn-1 computed previously
    num_data:
        The number of data used to compute Mn

    Returns
    -------
    float
        The new mean Mn updated with Xn
    """
    return (old_mean + (new_data - old_mean) / num_data)

def update_variance(new_data, old_variance, new_mean, old_mean, num_data):
    """Compute a new variance recursively using the old and new mean

    From the previously computed variance V_n-1, the new and old mean M_n and
    M_n-1 and the new measurement X_n, we compute an update value of the new
    variance using the formula:
    V_n = V_n-1 + (((X_n - M_n)*(X_n - M_n-1) - V_n-1) / n)

    See: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance

    Notes
    -----
    This formula may be unstable due to the floating point calculation issues.  

    Parameters
    ----------
    new_data: int or decimal
        The new measurement X_n
    old_variance: int or decimal
        The variance V_n-1 computed previously
    new_mean: int or decimal
        The mean M_n computed on the current step n
    old_mean: int or decimal
        The mean M_n-1 computed previously
    num_data:
        The number of total data n

    Returns
    -------
    float
        The new variance M_n updated with X_n, V_n-1, M_n and M_n-1
    """
    return (
        old_variance +
        (
            ((new_data - old_mean) * (new_data - new_mean) - old_variance)
            / num_data
        )
    )


def update_sum_squares(new_data, old_sum_squares, new_mean, old_mean):
    """Compute the update of sum of squares of differences from the current mean

    From the previously computed sum SUM_n-1, the new and old mean M_n and
    M_n-1 and the new measurement X_n, we compute an update value of the new
    sum of squares differences noted SUM_n using the formula:
    SUM_n = SUM_n-1 +(X_n - M_n)*(X_n - M_n-1)

    See: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance

    This SUM can be use to compute the variance and sample variance:
    Vn = SUM_n/n
    Sn = SUM_n/(n+1)

    This make the variance computation suffer less from floating point
    computation instabilities.

    Parameters
    ----------
    new_data: int or decimal
        The new measurement X_n
    old_sum_squares: int or decimal
        The sum of squares SUM_n-1 computed previously
    new_mean: int or decimal
        The mean M_n computed on the current step n
    old_mean: int or decimal
        The mean M_n-1 computed previously

    Returns
    -------
    float
        The new sum of squares SUM_n updated with X_n, SUM_n-1, M_n and M_n-1
    """
    return (old_sum_squares + ((new_data - old_mean)*(new_data - new_mean)))


class OnlineStatistics(object):
    """Class use to update online the mean and variance of a measurement

    This class, when receiving a new measurement, automatically compute its
    mean and variance/stdev recursively.

    Ultimately, you can use the updated variable (threading.Condition) to sync
    up your code when measurement has been done as it will notify you when new
    measurement is used to update the mean/variance computation.


    Attributs
    ---------
    updated: threading.Condition
        Condition used to indicate that a measurement has been add, and
        therefore we computed new values of mean/variance...
    number_of_measurement: uint
        The number of time this variable has been updated (n)
    measurement: float
        The new measurement at step = n (Xn)
    mean: float
        The mean Mn computed using recurrent equation.
    variance: float
        The variance Vn computed using recurrent equation.
    sampled_variance: float
        The sampled variance Sn computed using recurrent equation.
    stats: collections.namedtuple(Stats, ['n', 'X', 'Mean', 'Var', 'SVar'])
        The concatenated stats in one namedtuple
    """
    CurrentStat = collections.namedtuple(
        'Stats',
        ['n', 'X', 'Mean', 'Var', 'SVar']
    )
    def __init__(self, condition_lock = None):
        self.updated = threading.Condition(condition_lock)
        self.reset()

    def __str__(self):
        return str(self.stats)

    @property
    def number_of_measurement(self):
        return self.__n

    @property
    def measurement(self):
        return self.__measurement

    @measurement.setter
    def measurement(self, new_measure):
        with self.updated:
            self.__n += 1
            self.__measurement = new_measure
            self.__mean, old_mean = (
                update_mean(
                    new_data = self.__measurement,
                    old_mean = self.__mean,
                    num_data = self.__n
                ),
                self.__mean
            )
            self.__sum_squares = update_sum_squares(
                new_data = self.__measurement,
                old_sum_squares = self.__sum_squares,
                new_mean = self.__mean,
                old_mean = old_mean
            )
            self.updated.notify_all()

    @property
    def mean(self):
        return self.__mean if self.__n > 0 else None

    @property
    def variance(self):
        return (self.__sum_squares / self.__n) if self.__n > 0 else None

    @property
    def sampled_variance(self):
        return (self.__sum_squares / (self.__n - 1)) if self.__n > 1 else None

    @property
    def stats(self):
        return OnlineStatistics.CurrentStat(
            n = self.number_of_measurement,
            X = self.measurement,
            Mean = self.mean,
            Var = self.variance,
            SVar = self.sampled_variance,
        )

    def reset(self):
        with self.updated:
            self.__n = 0
            self.__measurement = None
            self.__mean = 0.
            self.__sum_squares = 0.
