import ismrmrd
import numpy as np

class IsmrmrdCondition:
    KSPACE_ENCODE_STEP_1 = 1
    KSPACE_ENCODE_STEP_2 = 2
    AVERAGE = 3
    SLICE = 4
    CONTRAST = 5
    PHASE = 6
    REPETITION = 7
    SET = 8
    SEGMENT = 9
    USER_0 = 10
    USER_1 = 11
    USER_2 = 12
    USER_3 = 13
    USER_4 = 14
    USER_5 = 15
    USER_6 = 16
    USER_7 = 17
    NONE = 18

class IsmrmrdAcquisitionData(object):
    def __init__(self, head=ismrmrd.AcquisitionHeader(),
        data=np.empty((0), dtype='complex64'), traj=None):
        self.head = head
        self.data = data
        self.traj = traj

class IsmrmrdAcquisitionBucketStats(object):
    def __init__(self):
        self.kspace_encode_step_1 = set()
        self.kspace_encode_step_2 = set()
        self.slice = set()
        self.phase = set()
        self.contrast = set()
        self.repetition = set()
        self.set = set()
        self.segment = set()
        self.average = set()

class IsmrmrdAcquisitionBucket(object):
    def __init__(self):
        self.data = list()
        self.ref = list()
        self.datastats = list()
        self.refstats = list()

class SamplingLimit(object):
    def __init__(self):
        self.min = 0
        self.center = 0
        self.max = 0

class SamplingDescription(object):
    def __init__(self):
        self.encoded_FOV = np.array((3), dtype='float32')
        self.recon_FOV = np.array((3), dtype='float32')
        self.encoded_matrix = np.array((3), dtype='ushort')
        self.recon_matrix = np.array((3), dtype='ushort')
        self.sampling_limits = list()

class IsmrmrdDataBuffered(object):
    def __init__(self):
        self.data = np.empty((0), dtype='complex64')
        self.trajectory = np.empty((0), dtype='float32')
        self.headers = list()
        self.sampling = SamplingDescription()

class IsmrmrdReconBit(object):
    def __init__(self):
        self.data = IsmrmrdDataBuffered()
        self.ref = IsmrmrdDataBuffered()

class IsmrmrdReconData(object):
    def __init__(self):
        self.rbit = list()

class IsmrmrdImageArray(object):
    def __init__(self):
        self.data = np.empty((0), dtype='complex64')
        self.headers = list()
        self.meta = list()
