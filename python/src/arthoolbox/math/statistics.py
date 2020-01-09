#!/usr/bin/env python3
"""Module containing specifics statistics function like recursive computation...

Function list:
- update_mean() | Compute an updated mean with the old mean and new measurement
- update_variance() | Compute a new variance TODO
"""

def update_mean(new_data, old_mean, num_data):
    """Compute a new mean recursively using the old mean and new measurement

    From the arithmetic mean computed using the n measurements (M_n), we compute
    the new mean (M_n+1) adding a new measurement (X_n+1) with the formula:
    M_n+1 = M_n+1 = M_n + (X_n+1 - M_n)/(n+1)


    Parameters
    ----------
    new_data: int or decimal
        The new measurement Xn+1
    old_mean: int or decimal
        The mean Mn computed previously
    num_data:
        The number of data used to compute Mn

    Returns
    -------
    float
        The new mean Mn+1 updated with Xn+1
    """
    return (old_mean + (new_data - old_mean) / (num_data + 1))
