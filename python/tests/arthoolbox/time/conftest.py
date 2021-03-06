#!/usr/bin/env python3
"""conftest.py file use to configurate pytest for the arthoolbox lib
"""
import pytest, time
from . import sample

def add_one(x):
    return x+1

###############################################################################
#                                PYTEST - HOOKS                               #

###############################################################################
#                          TEST - ARTHOOLBOX FIXTURES                         #
@pytest.fixture(
    scope = "module",
    params = [
        [1, 1],
        list(range(50)) ,
    ],
    ids = lambda x: "{}_calls".format(len(x))
)
def input_args(request):
    return request.param

@pytest.fixture(
    scope = "function",
    params = [
        sample(add_one),
        sample(time_function = time.time)(add_one),
        sample(time_function = time.perf_counter)(add_one),
    ],
    ids = [
        "sample_default",
        "sample_time",
        "sample_perf_counter"
    ]
)
def function(request):
    yield request.param
    request.param.last_call = None
    request.param.period.reset()
