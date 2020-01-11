#!/usr/bin/env python3
"""Module containing the sample decorator use to sample function calls

Good tutorial on decorator:
https://realpython.com/primer-on-python-decorators/

Decorators list:
- sample | (opt) time_function | Compute mean period, stddev period, last_call delta T etc..
"""
import time                     # Default time measurement function (time.time)
import functools                # functools.update_wrapper
from arthoolbox.math.statistics import OnlineStatistics

class sample(object):
    """Compute timing parameters like the mean/stddev period call for a function

    This class is a decorator use to sample the __call__() (mean and
    variance period call in sec).

    It can also be use to obtain the number of time the function has been
    called (period.number_of_measurement - 1), the last measurement done on the
    period (period.measurement).



    Attributs
    ---------
    __function: function
        The function wrapped by the decorator
    __get_time: function
        The function use to get the current time (default to time.time)
    last_call: float
        The last __get_time() returned value use to compute measurements
    period: arthoolbox.math.statistics.OnlineStatistics
        OnlineStatistics object use to compute mean and variance online

    """
    def __init__(self, _function = None, *, time_function = time.time):
        if _function:
            functools.update_wrapper(self, _function)
            self.__function = _function

        else:
            self.__function = None

        self.__get_time = time_function
        self.last_call = None
        self.period = OnlineStatistics()


    def __call__(self, *args, **kwargs):
        if not self.__function:
            # function is None, it means that sample has been initialize with
            # an argument therefore the first __call__ contains the function to
            # wrap around
            # i.e.:
            # @sample(time_function = time.time)
            # def toto():
            #   pass
            #
            # is equivalent to:
            # toto = sample(time_function = time.time)(toto)

            functools.update_wrapper(self, args[0])
            self.__function = args[0]

            return self

        else:
            # Here normal function call
            now = self.__get_time()

            if self.last_call is not None:
                # Update with new measurement + Update last_call with now
                self.period.measurement = now - self.last_call

            self.last_call = now

            return self.__function(*args, **kwargs)
