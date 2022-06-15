import picologging
import logging
import pytest

def test_logger_attributes():
    logger = picologging.Logger('test')
    assert logger.name == 'test'
    assert logger.level == logging.NOTSET
    assert logger.parent == None
    assert logger.propagate == True
    assert logger.handlers == []
    assert logger.disabled == False
    assert logger.propagate == True


levels = [
    logging.DEBUG,
    logging.INFO,
    logging.WARNING,
    logging.ERROR,
    logging.CRITICAL,
    logging.NOTSET,
]

@pytest.mark.parametrize("level", levels)
def test_logging_custom_level(level):
    logger = picologging.Logger('test', level)
    assert logger.level == level


def test_set_level():
    logger = picologging.Logger('test')
    logger.setLevel(logging.DEBUG)
    assert logger.level == logging.DEBUG


def test_get_effective_level():
    logger = picologging.Logger('test')
    parent = picologging.Logger('parent', logging.DEBUG)
    logger.parent = parent
    assert logger.getEffectiveLevel() == logging.DEBUG
    assert logger.level == 0
    logger.setLevel(logging.WARNING)
    assert logger.getEffectiveLevel() == logging.WARNING


def test_dodgy_parents():
    logger = picologging.Logger('test')
    parent = "potato"
    logger.parent = parent
    with pytest.raises(TypeError):
        logger.getEffectiveLevel()


def test_add_filter():
    logger = picologging.Logger('test')
    filter = logging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    filter2 = logging.Filter("filter2")
    logger.addFilter(filter2)
    assert logger.filters == [filter, filter2]


def test_remove_filter():
    logger = picologging.Logger('test')
    filter = logging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    logger.removeFilter(filter)
    assert logger.filters == []


def test_no_filter():
    logger = picologging.Logger('test')
    record = picologging.LogRecord('test', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == True


def test_filter_record():
    logger = picologging.Logger('test')
    filter = logging.Filter("hello")
    logger.addFilter(filter)
    record = picologging.LogRecord('hello', logging.INFO, 'test', 1, 'test', (), {})
    record2 = picologging.LogRecord('goodbye', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == True
    assert logger.filter(record2) == False
    logger.removeFilter(filter)
    assert logger.filter(record) == True
    assert logger.filter(record2) == True


def test_filter_callable():
    logger = picologging.Logger('test')
    def filter(record):
        return record.name == 'hello'
    logger.addFilter(filter)
    record = picologging.LogRecord('hello', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == True
    record = picologging.LogRecord('goodbye', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == False
