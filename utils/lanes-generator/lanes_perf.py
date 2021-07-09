from contextlib import contextmanager
import functools
import time


def timed(f):
    @functools.wraps(f)
    def wrapped(*args, **kwargs):
        with timer(f'{f.__name__} call'):
            return f(*args, **kwargs)
    return wrapped


@contextmanager
def timer(what):
    t0 = time.perf_counter()
    try:
        yield
    finally:
        t1 = time.perf_counter()
        print(f'[{t1-t0:.3f}s] {what}')
