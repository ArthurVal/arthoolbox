#!/usr/bin/env python
"""Given an input range [a, b, c, ...], sort each element from the lowest to the
highest and output the result through stdout. The input range is selected either
from the argument list (element are separated by white spaces) OR from stdin
(has the priority over normal arguments).

Examples:
- Classic sort from arguments:
$ ./sort_elements.py 5 1 2 3 545 2 2 1
1
1
2
2
2
3
5
545

- Sort using unix pipe (ignoring the arguments):
$ ls | ./sort_elements.py 5 1 2 3 545 2 2 1
list_duplicates.py
list_duplicates_example.sh
sort_elements.py

## Selecting keys

By default, each elements (strings) are lexicographically compared with each
other but optional arguments can be used to select (-kf / --key-filter) a 'KEY',
located inside each elements, in order to perform the sorting.

Notes:
- '--kf/--key-filter' use the default python regex syntax, see
  "https://docs.python.org/3.12/library/re.html" for more details;
- When the regex filter doesn't match anything, the element will be ignored;
- Ignored elements are outputed through stderr when logging lvl is set to DEBUG;

Examples:
- Sort the elements using the LAST character of each elements
$ ls | ./sort_elements.py -kf '.*(.)$'
list_duplicates_example.sh
list_duplicates.py
sort_elements.py

- Sort elements starting with a 'T' (Foo is ignored)
$ ./sort_elements.py -kf '(T.*)$' Toto Tata Foo
Tata
Toto

- Sort elements starting with a 'T' (Foo is ignored and logged)
$ ./sort_elements.py -kf '(T.*)$' Toto Tata Foo -v DEBUG
[2023-01-13 16:31:06,211][DEBUG   ]: Key Convert: str
[2023-01-13 16:31:06,211][DEBUG   ]: Key filter : (T.*)$
[2023-01-13 16:31:06,211][INFO    ]: Processing elements from ARGS (3 elements): ...
[2023-01-13 16:31:06,211][DEBUG   ]: Ignoring: 'Foo'
Tata
Toto
[2023-01-13 16:31:06,211][INFO    ]: Processing elements from ARGS (3 elements): SUCCEED

## Converting keys

Once a key is selected from the input string element, the key is compared using
str comparison. Yet you can use the '-kt/--key-type' argument to perform a
conversion from the key string toward an other type (currently mainly
arithmetic types, like integer or floating points).

Examples:
- Not using key-type (string comparison)
$ ./sort_elements.py 1.123 2.414 1.2314 1e5
1.123
1.2314
1e5
2.414

- Using key-type = float
$ ./sort_elements.py 1.123 2.414 1.2314 1e5 -kt float
1.123
1.2314
2.414
1e5
"""
import argparse
import logging

import sys
import select

import re
import functools

def __is_using_stdin():
    """Indicates if stdin is active (i.e. using a pipe)."""
    return bool(select.select([sys.stdin], [], [], 0)[0])

def __all_stdin_elements():
    """Generator iterating over all lines from stdin until EOF."""
    try:
        while True:
            yield input()
    except EOFError:
        pass

def __regex_arg(pattern):
    """Return a REGEX engine if it contains at least ONE matching group."""
    regex = re.compile(pattern)

    if regex.groups < 1:
        raise argparse.ArgumentTypeError(
            "The regex pattern requires at least ONE matching group (pattern"
            " contained inside parenthesis)."
        )

    return regex

def when_output(predicate, handle):
    """Decorates f, calling handle(args) when predicate(f(args)) is TRUE."""
    def decorator_when_output(func):
        @functools.wraps(func)
        def wrapper_when_output(*args, **kwargs):
            output = func(*args, **kwargs)
            if predicate(output):
                handle(*args, **kwargs)
            return output

        return wrapper_when_output

    return decorator_when_output

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description = "Sort (stable) items from a range in ascending order.",
        epilog = __doc__,
        formatter_class = argparse.RawTextHelpFormatter
    )

    parser.add_argument(
        'elements',
        nargs='*',
        help = "Elements to sort (overwrited by STDIN if piped)."
    )

    parser.add_argument(
        '-r', '--reverse',
        action='store_true',
        help = "Sort in reverse order."
    )

    parser.add_argument(
        '-kf', '--key-filter',
        default = "(.*)",
        type = __regex_arg,
        help =
        "Regex containing AT LEAST one matching group (default: '(.*)').\n"
        "The matching group will be used as KEY when sorting elements.\n"
        "Example:\n"
        '-kf "^(.).*$" : Use the FIRST character of each element to sort them.\n'
        'WARNING: None matching element will be ignore from the sorted restult.'
    )

    convert_map = {
        'int'   : lambda str: int(str, 10),
        'hex'   : lambda str: int(str, 16),
        'oct'   : lambda str: int(str, 8),
        'float' : float,
        'str'   : str,
    }

    parser.add_argument(
        '-kt', '--key-type',
        default = 'str',
        choices = list(convert_map.keys()),
        help =
        "Expected key type (after filtering) used to perform the comparison "
        "when sorting (default: str)."
    )

    parser.add_argument(
        '-v', '--verbose',
        choices = ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
        default = 'WARNING',
        help = 'Logging verbose level (stderr) (default: WARNING).',
    )

    args = parser.parse_args()

    logging.basicConfig(
        level = args.verbose,
        format = '[%(asctime)s][%(levelname)-8s]: %(message)s',
        # By default, logs are going through stderr, this ways it doesn't
        # disturb the script output

        # stream=sys.stdout,
    )

    log = logging.getLogger(__name__)

    log.debug("Key Convert: {}".format(args.key_type))
    log.debug("Key Filter : {}".format(args.key_filter.pattern))

    log_header_str = "Processing elements from {}"

    # Using the stdin (UNIX pipe...) takes priority over the standards arguments
    # pass through the script
    if __is_using_stdin():
        log_header_str = log_header_str.format("STDIN")
        args.elements = __all_stdin_elements()
    else:
        log_header_str = log_header_str.format(
            "ARGS ({} elements)".format(len(args.elements))
        )

    log.info("{}: ...".format(log_header_str))

    get_key_match = args.key_filter.fullmatch

    # When the logging level is DEBUG or less, we decorate the regex.fullmatch()
    # function in order to log elements that doesn't match the regex, for DEBUG
    # purposes.
    if log.getEffectiveLevel() <= logging.DEBUG:
        get_key_match = when_output(
            lambda match: match is None,
            lambda element: log.debug("Ignoring: '{}'".format(element))
        )(get_key_match)

    convert_key = convert_map[args.key_type]

    for elem in sorted(
            # get_key_match returns regex matches that are 'None' when failing
            # to find the key pattern -> filter(None, ...) removes them
            filter(None, map(get_key_match, args.elements)),
            reverse = args.reverse,

            # Sorting is done by converting the key found (.group(1)) and
            # comparing it with each others.
            key = lambda match: convert_key(match.group(1))
    ):
        # Elem are VALID (i.e. != None) regex.matches object returned by
        # get_key_match:
        # .group(0) corresponds to the complete element that matches the pattern
        # .group(1) is the matched key used to sort
        print(elem.group(0))

    log.info("{}: SUCCEED".format(log_header_str))
