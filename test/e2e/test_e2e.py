from __future__ import annotations

import re
import os
import glob
import pytest
import hashlib
import subprocess
import yaml

import h5py
import numpy

import socket
import urllib.error
import urllib.request

from dataclasses import dataclass, field
from typing import Dict, List, Callable, Set


all_test_specs = []
gadgetron_capabilities = None

SPEC_FILENAME_KEY = 'spec_filename'


@dataclass
class Spec():

    @dataclass
    class Job():
        name: str
        datafile: str
        checksum: str
        configurations: List[str]
        args: List[str]

        output_group: str = None

        measurement: str = None
        data_conversion_flag: str = None
        parameter_xml: str = None
        parameter_xsl: str = None

        @staticmethod
        def fromdict(config: Dict[str, str], name: str) -> Spec.Job:
            if not config:
                return None

            datafile = config['data']
            if not datafile:
                raise ValueError("Missing 'data' key in job configuration")

            checksum = config['checksum']
            if not checksum:
                raise ValueError("Missing 'checksum' key in job configuration")

            if 'configuration' in config and 'stream' in config:
                raise ValueError("Cannot have both 'configuration' and 'stream' in a job")
            if 'configuration' in config:
                configurations = [config['configuration']]
                args = [config.get('args', '')]
            elif 'stream' in config:
                configurations = []
                args = []
                for stream in config['stream']:
                    configurations.append(stream['configuration'])
                    args.append(stream.get('args', ''))

            output_group = config.get('output_group', None)

            if 'measurement' in config:
                measurement = str(config['measurement'])

            return Spec.Job(name=name, datafile=datafile, checksum=checksum,
                    configurations=configurations, args=args,
                    output_group=output_group,
                    measurement=measurement,
                    data_conversion_flag=config.get('data_conversion_flag', ''),
                    parameter_xml=config.get('parameter_xml', 'IsmrmrdParameterMap_Siemens.xml'),
                    parameter_xsl=config.get('parameter_xsl', 'IsmrmrdParameterMap_Siemens.xsl')
            )

    @dataclass
    class Validation():
        reference: str
        checksum: str
        reference_image: str
        output_image: str
        scale_comparison_threshold: float
        value_comparison_threshold: float
        disable_image_header_test: bool = False

        @staticmethod
        def fromdict(config: Dict[str, str]) -> Spec.Validation:
            if not config:
                return None
            reference = config['reference']
            if not reference:
                raise ValueError("Missing 'reference' key in job configuration")
            checksum = config['checksum']
            if not checksum:
                raise ValueError("Missing 'checksum' key in job configuration")
            reference_image = config['reference_image']
            if not reference_image:
                raise ValueError("Missing 'reference_image' key in job configuration")
            output_image = config['output_image']
            if not output_image:
                raise ValueError("Missing 'output_image' key in job configuration")
            scale_comparison_threshold = config['scale_comparison_threshold']
            if not scale_comparison_threshold:
                raise ValueError("Missing 'scale_comparison_threshold' key in job configuration")
            value_comparison_threshold = config['value_comparison_threshold']
            if not value_comparison_threshold:
                raise ValueError("Missing 'value_comparison_threshold' key in job configuration")
            disable_image_header_test = config.get('disable_image_header_test', False)

            return Spec.Validation(reference=reference, checksum=checksum,
                    reference_image=reference_image, output_image=output_image,
                    scale_comparison_threshold=scale_comparison_threshold,
                    value_comparison_threshold=value_comparison_threshold,
                    disable_image_header_test=disable_image_header_test)

    filename: str
    name: str
    tags: Set[str] = field(default_factory=set)
    requirements: Dict[str, str] = field(default_factory=dict)

    dependency: Spec.Job = None
    reconstruction: Spec.Job = None
    validations: List[Spec.Validation] = field(default_factory=list)

    def id(self):
        return f"{self.filename}::{self.name}"

    @staticmethod
    def fromfile(filename: str) -> Spec:
        with open(filename, 'r') as file:
            parsed = yaml.safe_load(file)
            spec = Spec(filename=filename, name=parsed['name'])

            tags = parsed.get('tags', None)
            if not tags:
                tags = []
            if not isinstance(tags, list):
                tags = [tags]
            spec.tags = set(tags)

            requirements = parsed.get('requirements', None)
            if not requirements:
                requirements = {}
            if not isinstance(requirements, dict):
                raise ValueError(f"Invalid requirements in {filename}")
            spec.requirements = requirements

            spec.dependency = Spec.Job.fromdict(parsed.get('dependency', None), 'dependency')
            spec.reconstruction = Spec.Job.fromdict(parsed['reconstruction'], 'reconstruction')

            validations = parsed['validation']
            if not isinstance(validations, list):
                validations = [validations]
            spec.validations = [Spec.Validation.fromdict(v) for v in validations]

            return spec


def pytest_generate_tests(metafunc: pytest.Metafunc) -> None:
    global gadgetron_capabilities
    gadgetron_capabilities = load_gadgetron_capabilities()

    # TODO: Only load if not already loaded
    for spec in load_test_cases():
        all_test_specs.append(spec)

def load_gadgetron_capabilities() -> Dict[str, str]:
    command = ["gadgetron", "--info"]
    res = subprocess.run(command, capture_output=True, text=True)
    if res.returncode != 0:
        pytest.fail(f"Failed to query Gadgetron capabilities... {res.args} return {res.returncode}")

    gadgetron_info = res.stderr

    value_pattern = r"(?:\s*):(?:\s+)(?P<value>.*)?"

    capability_markers = {
        'version': "Version",
        'build': "Git SHA1",
        'memory': "System Memory size",
        'python': "Python Support",
        'julia': "Julia Support",
        'matlab': "Matlab Support",
        'cuda': "CUDA Support"
    }

    plural_capability_markers = {
        'cuda_memory': "Total amount of global GPU memory",
        'cuda': "Number of CUDA capable devices"
    }

    def find_value(marker):
        pattern = re.compile(marker + value_pattern, re.IGNORECASE)
        match = pattern.search(gadgetron_info)

        if not match:
            pytest.fail(f"Failed to parse Gadgetron capability '{marker}' from {gadgetron_info}")

        return match['value']

    def find_plural_values(marker):
        pattern = re.compile(marker + value_pattern, re.IGNORECASE)
        return [match['value'] for match in pattern.finditer(gadgetron_info)]

    capabilities = {key: find_value(marker) for key, marker in capability_markers.items()}
    capabilities.update({key: find_plural_values(marker) for key, marker in plural_capability_markers.items()})

    return capabilities


def load_test_cases() -> List[Dict[str, str]]:
    specs = []
    for filename in glob.glob('cases/*.yml'):
        spec = Spec.fromfile(filename)
        specs.append(spec)
    return sorted(specs, key=lambda s: s.id())


@pytest.fixture
def check_requirements(spec: Spec, ignore_requirements: Set[str], run_tags: Set[str]):
    # print(f"Checking requirements for: {spec.name}")

    # Check tags first
    # The spec's tags must include at least one of the run_tags
    # NOTE: This means the `--tags` option is an OR expression, not an AND expression
    if len(run_tags) > 0 and len(spec.tags & run_tags) == 0:
        print(run_tags)
        pytest.skip("Test missing required tag.")
    if 'skip' in spec.tags:
        pytest.skip("Test was marked as skipped")

    # Then check requirements
    def rules_from_reqs(section):
        class Rule:
            def __init__(self, capability, validator, message):
                self.capability = capability
                self.validator = validator
                self.message = message

            def is_satisfied(self, capabilities):
                value = capabilities.get(self.capability)
                return self.validator(value)
    
        def parse_memory(string):
            pattern = re.compile(r"(?P<value>\d+)(?: MB)?")
            match = pattern.search(string)
            return float(match['value'])

        def is_enabled(value):
            return value in ['YES', 'yes', 'True', 'true', '1']

        def has_more_than(target):
            return lambda value: parse_memory(str(target)) <= parse_memory(value)

        def each(validator):
            return lambda values: all([validator(value) for value in values])

        rules = [
            ('matlab_support', lambda req: Rule('matlab', is_enabled, "MATLAB support required.")),
            ('python_support', lambda req: Rule('python', is_enabled, "Python support required.")),
            ('julia_support', lambda req: Rule('julia', is_enabled, "Julia support required.")),
            ('system_memory', lambda req: Rule('memory', has_more_than(req), "Not enough system memory.")),
            ('gpu_support', lambda req: Rule('cuda', is_enabled, "CUDA support required.")),
            ('gpu_support', lambda req: Rule('cuda', each(has_more_than(req)), "Not enough CUDA devices.")),
            ('gpu_memory', lambda req: Rule('cuda_memory', each(has_more_than(req)), "Not enough graphics memory."))
        ]

        return [(key, rule(section[key])) for key, rule in rules if key in section]

    rules = rules_from_reqs(spec.requirements)
    for _, rule in rules:
        if rule.capability in ignore_requirements:
            continue
        if not rule.is_satisfied(gadgetron_capabilities):
            pytest.skip(rule.message)


def calc_md5(file: Path) -> str:
    md5 = hashlib.new('md5')
    with open(file, 'rb') as f:
        for chunk in iter(lambda: f.read(65536), b''):
            md5.update(chunk)
    return md5.hexdigest()

def is_valid(file: Path, digest: str) -> bool:
    if not os.path.isfile(file):
        return False
    return digest == calc_md5(file) 

def urlretrieve(url: str, filename: str, retries: int = 5) -> str:
    if retries <= 0:
        pytest.fail("Download from {} failed".format(url))
    try:
        with urllib.request.urlopen(url, timeout=60) as connection:                        
            with open(filename,'wb') as f:
                for chunk in iter(lambda : connection.read(1024*1024), b''):
                    f.write(chunk)
            return connection.headers["Content-MD5"]
    except (urllib.error.URLError, ConnectionResetError, socket.timeout) as exc:
        print("Retrying connection for file {}, reason: {}".format(filename, str(exc)))
        return urlretrieve(url, filename, retries=retries-1)

@pytest.fixture
def fetch_test_data(cache_path: Path, data_host_url: str, tmp_path: Path) -> Callable:
    # print(f"Cache path: {cache_path}")

    # TODO: yield nested function then perform cleanup afterwards if not caching

    # TODO: Make it return a Path
    def _fetch_test_data(filename: str, checksum: str) -> str:
        print(f"Fetching test data: {filename}")
        url = f"{data_host_url}{filename}"

        if not cache_path:
            destination = os.path.join(tmp_path, filename)
        else:
            destination = os.path.join(cache_path, filename)

        need_to_fetch = True
        if os.path.exists(destination):
            if not os.path.isfile(destination):
                pytest.fail(f"Destination '{destination}' exists but is not a file")
            
            if not is_valid(destination, checksum):
                print(f"Destination '{destination}' exists file but checksum does not match... Forcing download")
            else:
                need_to_fetch = False

        if need_to_fetch:
            os.makedirs(os.path.dirname(destination), exist_ok=True)
            urlretrieve(url, destination)

        if not is_valid(destination, checksum):
            pytest.fail(f"Downloaded file '{destination}' does not match checksum")

        return destination

    return _fetch_test_data


@pytest.fixture
def convert_data(tmp_path: Path, fetch_test_data: Callable) -> Callable:
    def _convert_data(input_file: str, job: Spec.Job) -> str:

        prefix = os.path.basename(input_file) + '_' + job.name

        # Do siemens_to_ismrmrd conversion if needed
        if job.measurement is not None:
            output_file = os.path.join(tmp_path, prefix + ".h5")
            command = ["siemens_to_ismrmrd", "-X",
                    "-f", input_file, 
                    "-m", job.parameter_xml,
                    "-x", job.parameter_xsl,
                    "-o", output_file,
                    "-z", job.measurement,
                    job.data_conversion_flag]

            with open(os.path.join(tmp_path, os.path.basename(input_file) + "_" + job.name + '.log.out'), 'w') as log_stdout:
                with open(os.path.join(tmp_path, os.path.basename(input_file) + "_" + job.name + '.log.err'), 'w') as log_stderr:
                    result = subprocess.run(command, stdout=log_stdout, stderr=log_stderr, cwd=tmp_path)
                    if result.returncode != 0:
                        pytest.fail(f"siemens_to_ismrmrd failed with return code {result.returncode}")
            input_file = output_file

        # Do ISMRMRD HDF5 -> ISMRMRD Stream -> MRD2 conversion
        output_file = os.path.join(tmp_path, prefix + ".mrd")
        command = f"ismrmrd_hdf5_to_stream -i {input_file} --use-stdout | ismrmrd_to_mrd -o {output_file}"
        command = ['bash', '-c', command]

        result = subprocess.run(command, cwd=tmp_path)
        if result.returncode != 0:
            pytest.fail(f"ismrmrd to mrd conversion failed with return code {result.returncode}")

        return output_file

    return _convert_data

@pytest.fixture
def prepare_test_data(fetch_test_data, convert_data):
    def _prepare_test_data(job: Spec.Job):
        input_file = fetch_test_data(job.datafile, job.checksum)

        input_file = convert_data(input_file, job)

        return input_file
            
    return _prepare_test_data


@pytest.fixture
def process_data(prepare_test_data, tmp_path):
    def _process_data(job):
        # print(f"Processing dependency for: {spec.name}")
        input_file = prepare_test_data(job)
        output_file = os.path.join(tmp_path, job.name + ".output.mrd")

        invocations = []
        for config, args in zip(job.configurations, job.args):
            invocations.append(f"gadgetron --from_stream -c {config} {args}")
        invocations[0] += f" --input_path {input_file}"
        # invocations[-1] += f" --output_path {output_file}"

        command = " | ".join(invocations)

        # Add MRD2 -> ISMRMRD Stream -> HDF5 conversion
        command += f" | mrd_to_ismrmrd | ismrmrd_stream_to_hdf5 --use-stdin -o {output_file} -g {job.output_group}"

        command = ['bash', '-c', command]
        print(f"Invoking Gadgetron: {command}")

        with open(os.path.join(tmp_path, job.name + '_gadgetron.log.out'), 'w') as log_stdout:
            with open(os.path.join(tmp_path, job.name + '_gadgetron.log.err'), 'w') as log_stderr:
                result = subprocess.run(command, stdout=log_stdout, stderr=log_stderr, cwd=tmp_path)
                if result.returncode != 0:
                    pytest.fail(f"Gadgetron failed with return code {result.returncode}")

        return output_file

    return _process_data

@pytest.fixture
def validate_output(fetch_test_data):
    def _validate_output(specs: List[Spec.Validation], output_file: str) -> None:
        for spec in specs:
            reference_file = fetch_test_data(spec.reference, spec.checksum)

            validate_data(output_file, spec.output_image, reference_file, spec.reference_image,
                    spec.scale_comparison_threshold, spec.value_comparison_threshold)

            if not spec.disable_image_header_test:
                validate_header(output_file, spec.output_image, reference_file, spec.reference_image)

    return _validate_output

def validate_data(output_file: str, output_group: str, reference_file: str, reference_group: str,
                scale_threshold: float, value_threshold: float):
    try:
        # The errors produced by h5py are not entirely excellent. We spend some code here to clear them up a bit.
        def get_group_data(file, group):
            with h5py.File(file, mode='r') as f:
                try:
                    group = group + '/data'
                    return numpy.squeeze(f[group])
                except KeyError:
                    pytest.fail("Did not find group '{}' in file {}".format(group, file))

        output_data = get_group_data(output_file, output_group)
        reference_data = get_group_data(reference_file, reference_group)
    except OSError as e:
        pytest.fail(str(e))
    except RuntimeError as e:
        pytest.fail(str(e))

    output = output_data[...].flatten().astype('float32')
    reference = reference_data[...].flatten().astype('float32')

    norm_diff = numpy.linalg.norm(output - reference) / numpy.linalg.norm(reference)
    scale = numpy.dot(output, output) / numpy.dot(output, reference)

    if value_threshold < norm_diff:
        pytest.fail("Comparing values, norm diff: {} (threshold: {})".format(norm_diff, value_threshold))

    if value_threshold < abs(1 - scale):
        pytest.fail("Comparing image scales, ratio: {} ({}) (threshold: {})".format(scale, abs(1 - scale),
                                                                                        scale_threshold))

def validate_header(output_file: str, output_group: str, reference_file: str, reference_group: str):
    # TODO: Implement after removing the ISMRMRD/HDF5 stuff
    pass


@pytest.mark.parametrize('spec', all_test_specs, ids=lambda s: s.id())
def test_e2e(spec, check_requirements, process_data, validate_output):
    if spec.dependency is not None:
        process_data(spec.dependency)

    output_file = process_data(spec.reconstruction)

    validate_output(spec.validations, output_file)
