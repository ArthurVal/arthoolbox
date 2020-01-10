#!/usr/bin/env python3
"""Module containing specifics statistics function like recursive computation...

Function list:
- update_mean()     | Compute an updated mean
- update_variance() | Compute a new variance
"""

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
