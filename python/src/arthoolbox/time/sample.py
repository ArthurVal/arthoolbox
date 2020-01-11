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

def sample(_func = None, *, time_function = time.time):
    """Compute timing parameters like the mean/stddev period call for a function

    This class is a decorator use to sample the decorated function. It can be
    use to obtain the last period measured in sec (func.period.measurement)
    between 2 function call, the mean, the variance and the sampled variance of
    the period between function calls with func.period.mean,
    func.period.variance and func.period.sampled_variance.

    Additionnaly, you can synchronise your code with the func.period.updated
    condition variable (threading.Condition) in order to be notified when new
    period statistic is computed.

    Attributs
    ---------
    last_call: float
        The last __get_time() returned value use to compute measurements
    period: arthoolbox.math.statistics.OnlineStatistics
        OnlineStatistics object use to compute mean and variance online

    Parameters
    ----------
    _func: Callable obj
        The function to wrap around the sampling
    time_function: Callable obj
        Function use to measure time point (Default time.time)
    """
    def decorator_sample(func):
        """Decorator use to store the func and create the wrapper based on func
        """
        @functools.wraps(func)
        def wrapper_sample(*args, **kwargs):
            """Function returns to wrap func
            """
            # Function call -> Time measurements
            now = time_function()

            if wrapper_sample.last_call is not None:
                # Register the new measurement
                wrapper_sample.period.measurement = \
                    now - wrapper_sample.last_call

            wrapper_sample.last_call = now

            return func(*args, **kwargs)

        # Initialize the wrapper function with attribut (function are objects)
        wrapper_sample.last_call = None
        wrapper_sample.period = OnlineStatistics()

        return wrapper_sample


    if _func is None:
        # sample called without unamed arguments -> First argument is the
        # function:
        #
        # function = sample(function)
        #
        # OR
        #
        # @sample
        # def function():
        #   ...
        return decorator_sample

    else:
        # sample called with time_function:
        #
        # function = sample(time_function = toto)(function)
        #
        # OR
        #
        # @sample(time_function = toto)
        # def function():
        #   ...
        return decorator_sample(_func)


# class sample(object):
#     """Compute timing parameters like the mean/stddev period call for a function

#     This class is a decorator use to sample the __call__() (mean and
#     variance period call in sec).

#     It can also be use to obtain the number of time the function has been
#     called (period.number_of_measurement - 1), the last measurement done on the
#     period (period.measurement).



#     Attributs
#     ---------
#     __function: function
#         The function wrapped by the decorator
#     __get_time: function
#         The function use to get the current time (default to time.time)
#     last_call: float
#         The last __get_time() returned value use to compute measurements
#     period: arthoolbox.math.statistics.OnlineStatistics
#         OnlineStatistics object use to compute mean and variance online
#     """
#     def __init__(self, _function = None, *, time_function = time.time):
#         if _function:
#             functools.update_wrapper(self, _function)
#             self.__function = _function

#         else:
#             self.__function = None

#         self.__get_time = time_function
#         self.last_call = None
#         self.period = OnlineStatistics()


#     def __call__(self, *args, **kwargs):
#         if not self.__function:
#             # function is None, it means that sample has been initialize with
#             # an argument therefore the first __call__ contains the function to
#             # wrap around
#             # i.e.:
#             # @sample(time_function = time.time)
#             # def toto():
#             #   pass
#             #
#             # is equivalent to:
#             # toto = sample(time_function = time.time)(toto)

#             functools.update_wrapper(self, args[0])
#             self.__function = args[0]

#             return self

#         else:
#             # Here normal function call
#             now = self.__get_time()

#             if self.last_call is not None:
#                 # Update with new measurement + Update last_call with now
#                 self.period.measurement = now - self.last_call

#             self.last_call = now

#             return self.__function(*args, **kwargs)
