#!/usr/bin/env python3
"""conftest.py file use to configurate pytest for the math pkg
"""
import pytest, random

###############################################################################
#                                PYTEST - HOOKS                               #

###############################################################################
#                          TEST - ARTHOOLBOX FIXTURES                         #
@pytest.fixture(
    scope = "function",
    params = [
        [1],
        list(range(50)),
        [random.uniform(-50, +50) for _ in range(50)]
    ],
    ids = [
        "single_data",
        "range_50",
        "random_50",
    ]
)
def data(request):
    return request.param
