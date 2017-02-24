######################################
### Microsoft compiler configurations release
######################################

import sys
from parts.config import *
import SCons.Script

def map_default_version(env):
    return env['MSVC_VERSION']


config=configuration(map_default_version)

config.VersionRange("6.0",
                    append=ConfigValues(
                        CCFLAGS=['/nologo','/Ox','/MD','/W3'],
                        CXXFLAGS=['/EHsc','/GR'],
                        CPPDEFINES=['NDEBUG'],
                        LINKFLAGS=['/INCREMENTAL:NO'],
                        )
                    )
config.VersionRange("7-*",
                    append=ConfigValues(
                        CCFLAGS=[
                            '/nologo','/Ox','/MT','/W3', '/Z7', '/GS-',
                            '/Gy', # Enable function level linking
                            ],
                        CXXFLAGS=['/EHs-', '/EHc-', '/GR-'],
                        CPPDEFINES=['NDEBUG', r'TARGET_ARCH=L\"${TARGET_ARCH}\"',
                            'TARGET_ARCH_${TARGET_ARCH}'],
                        LINKFLAGS=[
                            '/nodefaultlib',
                            '/INCREMENTAL:NO',
                            '/OPT:REF,ICF=4' # Put only used functions to the resulting binary
                            ],
                        LINKTRAILFLAGS=['libcmt.lib', 'libcpmt.lib', 'libcpmt1.lib'],
                        PDB=['${TARGETS[0]}.pdb']
                        )
                    )

