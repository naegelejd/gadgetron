#!/usr/bin/python3

import pytest
import os
import glob
import shutil

from pathlib import Path
from typing import List, Set

# def pytest_exception_interact(node, call, report):
#     if report.failed and node.config.getoption('--echo-log-on-failure'):
#         if 'tmp_path' in node.funcargs:
#             tmp_path = node.funcargs['tmp_path']
#             for log in glob.glob(os.path.join(tmp_path, '*.log*')):
#                 with open(log, 'r') as logfile:
#                     logdata = logfile.read()                
#                     report.sections.append((log, logdata))       

# def pytest_runtest_teardown(item, nextitem):
#     if item.config.getoption('--save-results'):
#         output_path=item.config.getoption('--save-results')
#         output_path = os.path.join(os.path.abspath(output_path), item.callspec.id)

#         if 'tmp_path' in item.funcargs:
#             tmp_path = item.funcargs['tmp_path']

#             shutil.copytree(tmp_path, output_path, dirs_exist_ok=True)
    

def pytest_addoption(parser):
    parser.addoption(
        '--data-host', action='store', default='http://gadgetrondata.blob.core.windows.net/gadgetrontestdata/', 
        help='Host from which to download the data.'
    )
    parser.addoption(
        '--cache-disable', action='store_true', default=False, 
        help='Disables local caching of input files.'
    )
    parser.addoption(
        '--cache-path', action='store', default="", 
        help='Location for storing cached data files.'
    )
    parser.addoption(
        '--ignore-requirements', action='store', default="",
        help="Run tests regardless of whether Gadgetron has the specified capabilities (comma-separated)."
    )
    parser.addoption(
        '--tags', action='store', default="", 
        help='Only run tests with the specified tags (comma-separated).'
    )
    parser.addoption(
        '--echo-log-on-failure', action='store_true', default=False, 
        help='capture test logs on a failed test.'
    )
    parser.addoption(
        '--save-results', action='store', default="",
        help='Save Gadgetron output and client logs to specified folder'
    )


@pytest.fixture
def data_host_url(request) -> str:
    return request.config.getoption('--data-host')

@pytest.fixture
def cache_disable(request) -> bool:
    return request.config.getoption('--cache-disable')

@pytest.fixture
def cache_path(request, cache_disable) -> Path:
    if cache_disable:
        return None

    base = request.config.getoption('--cache-path')
    if base == "":
        current_dir = Path(os.path.dirname(__file__))
        # base = os.path.join(current_dir, "data")
        return current_dir / "data"
    else:
        # base = Path(base)
        return Path(base)

@pytest.fixture
def ignore_requirements(request) -> Set[str]:
    return parse_comma_separated_set(request.config.getoption('--ignore-requirements'))

@pytest.fixture
def run_tags(request) -> Set[str]:
    return parse_comma_separated_set(request.config.getoption('--tags'))


def parse_comma_separated_set(arg: str) -> Set[str]:
    if arg == '':
        return set()
    return set(arg.split(','))