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
                        CPPDEFINES=['NDEBUG']
                        )
                    )
config.VersionRange("7-*",
                    append=ConfigValues(
                        CCFLAGS=['/nologo','/Ox','/MT','/W3', '/Z7', '/GS-'],
                        CXXFLAGS=['/EHs-', '/EHc-', '/GR-'],
                        CPPDEFINES=['NDEBUG'],
                        LINKFLAGS=['/nodefaultlib'],
                        PDB=['${TARGETS[0].filebase}.pdb']
                        )
                    )

