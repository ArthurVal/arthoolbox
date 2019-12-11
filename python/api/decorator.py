#!/usr/bin/env python3
"""Module containing utils decorator used by test scripts

Good tutorial on decorator:
https://realpython.com/primer-on-python-decorators/

Decorators list:
- sample | no args | Compute mean period, stddev period, last_call delta T etc..
"""
import time, threading, functools, math

class sample:
    """Compute timing parameters like the mean/stddev period call for function

    Attributs
    ---------
    __function: function
        The function wrapped by the decorator
    __last_call: float
        The last time we called it (time.perf_counter())
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
    def __init__(self, function):
        functools.update_wrapper(self, function)
        self.__function = function
        self._updated = threading.Condition()
        self._reset_sampling()

    @property
    def _variance_period(self):
        with self._updated:
            return (self.__variance_tmp / (self._n_measurement)) if self._n_measurement != 0 else None

    @property
    def _stddev_period(self):
        with self._updated:
            return math.sqrt(self._variance_period) if self._n_measurement != 0 else None

    def _reset_sampling(self):
        with self._updated:
            self.__last_call = None
            self.__variance_tmp = None
            self._n_call = 0
            self._n_measurement = 0
            self._last_period = None
            self._mean_period = None

    def __call__(self, *args, **kwargs):
        with self._updated:
            now = time.perf_counter()

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
