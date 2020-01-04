#!/usr/bin/env python3
"""Module containing the sample decorator use to sample function calls

Good tutorial on decorator:
https://realpython.com/primer-on-python-decorators/

Decorators list:
- sample | (opt) time_function | Compute mean period, stddev period, last_call delta T etc..
"""
import time                     # Default time measurement function (time.time)
import threading                # threading.condition
import functools                # functools.update_wrapper
import math                     # math.sqrt

class sample:
    """Compute timing parameters like the mean/stddev period call for a function

    This class is a decorator use to sample the __call__() (mean and
    stddev period call in sec).

    It can also be use to obtain the number of time the function has been
    called (_n_call), the last measurement done on the period (_last_period).

    Ultimately, you can use the _updated variable (threading.Condition) to sync
    up your code when measurement has been done as it will notify you when new
    measurement is used to update the mean/stddev computation.

    Attributs
    ---------
    __function: function
        The function wrapped by the decorator
    __get_time: function
        The function use to get the current time (default to time.time)
    __last_call: float
        The last time we called __get_time()
    __variance_tmp: float
        The temporary variable used to compute the variance
    _updated: threading.Condition
        Condition used to indicate that the function has been called, and
        therefore we computed new values
    _n_call: int
        The number of time we called it
    _n_measurement: int
        The number of measurement done to compute the mean/stddev
    _last_period: float
        The last delta time measured in sec (now - __last_call)
    _mean_period: float
        The mean period computed recursivelly in sec
    _variance_period: float
        The variance period computed recursivelly in sec
    _stddev_period: float
        The stddev period computed recursivelly in sec (_variance_period)^(1/2)
    """
    def __init__(self, _function = None, *, time_function = time.time):
        self.__function = _function
        self.__get_time = time_function
        self._updated = threading.Condition()
        self._reset_sampling()

    @property
    def _variance_period(self):
        with self._updated:
            if self._n_measurement != 0:
                return (self.__variance_tmp / (self._n_measurement))
            else:
                return None

    @property
    def _stddev_period(self):
        with self._updated:
            if self._n_measurement != 0:
                return math.sqrt(self._variance_period)
            else:
                return None

    def _reset_sampling(self):
        with self._updated:
            self.__last_call = None
            self.__variance_tmp = None
            self._n_call = 0
            self._n_measurement = 0
            self._last_period = None
            self._mean_period = None

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
            # toto = sample(time_measurement_function = time.time)(toto)

            functools.update_wrapper(self, args[0])
            self.__function = args[0]

            return self

        else:
            # Here normal function call
            with self._updated:
                now = self.__get_time()

                if self.__last_call is not None:

                    # Measurement Xn
                    self._last_period = now - self.__last_call

                    if self._n_measurement == 0:  # Initialize
                        # Initialization with the first measurement
                        self._mean_period = self._last_period
                        self.__variance_tmp = 0

                    else:       # Iterative computation
                        # Compute Mean ( M_n+1 = M_n + (X_n+1 - M_n)/(n+1))
                        # NOTE: (_n_call) is equals to (_n_measurement + 1)
                        new_mean = \
                            self._mean_period + \
                            ((self._last_period - self._mean_period) / \
                             self._n_call)

                        # Compute the tmp variance V* (false variance)
                        # V*_n+1 = V*_n + (X_n+1 - M_n)x(X_n+1 - M_n+1)
                        # True variance V: V_n+1 = V*/(n)
                        self.__variance_tmp += \
                            ((self._last_period - self._mean_period) *\
                             (self._last_period - new_mean))

                        self._mean_period = new_mean

                    # Notify new measurement
                    self._updated.notify_all()
                    self._n_measurement += 1

                self.__last_call = now
                self._n_call += 1

            return self.__function(*args, **kwargs)
