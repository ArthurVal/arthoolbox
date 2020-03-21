"""Module containing the lazydict class definition

Class list:
- _LazyDict           | Dictionnary with special value initialized on access
- _LazyDict.LazyValue | The special constructor used to lazy build values
"""

class _LazyDict:
    """Dictionnary that can initialize content lazily with the LazyValue class
    """
    class LazyValue(object):
        """Class use to construct value by giving it a function and its args
        """
        def __init__(self, func, *args, **kwargs):
            if callable(func):
                self.__func = func
                self.__args = args

            else:
                self.__func = lambda *x, **y: x + tuple(y.items())
                self.__args = (func, ) + args

            self.__kwargs = kwargs

        def construct(self):
            return self.__func(*self.__args, **self.__kwargs)

        def __repr__(self):
            return "{}{}".format(
                repr(self.__func.__name__),
                self.__args + tuple(self.__kwargs.items())
            )


    def __init__(self, **kwargs):
        self.__dict = kwargs


    def __repr__(self):
        return "{{{}}}".format(
            ", ".join(
                [
                    "{key}: {value}".format(
                        key = repr(k),
                        value = repr(v)
                        if not isinstance(v, _LazyDict.LazyValue)
                        else "<Not Instanciated: {}>".format(repr(v))

                    )
                    for k,v in self.__dict.items()
                ]
            )
        )



    def __setitem__(self, key, value):
        self.__dict[key] = value


    def __getitem__(self, key):
        if key not in self.__dict:
            raise KeyError("{} not contained within dict".format(key))

        elif isinstance(self.__dict[key], _LazyDict.LazyValue):
            self.__dict[key] = self.__dict[key].construct()

        return self.__dict[key]


    def __delitem__(self, key):
        del self.__dict[key]


    def __contains__(self, key):
        return key in self.__dict


    def __len__(self):
        return len(self.__dict)


    def __iter__(self):
        return iter(self.__dict)
