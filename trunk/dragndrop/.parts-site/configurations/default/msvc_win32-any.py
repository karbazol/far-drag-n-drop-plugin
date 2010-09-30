######################################
### Microsoft compiler configurations default
######################################

import sys
from parts.config import *
import SCons.Script

def map_default_version(env):
    return env['MSVC_VERSION']

config=configuration(map_default_version)

config.VersionRange("6.0",
                    append=ConfigValues(
                        CPPDEFINES=['WIN32','_WINDOWS', '_WIN32_WINNT=0x0500',
                            'UNICODE', '_UNICODE'],
                        CCFLAGS=['/DMSVC_VERSION=$MSVC_VERSION']
                        )
                    )
config.VersionRange("7-*",
                    append=ConfigValues(
                        CPPDEFINES=['WIN32','_WINDOWS', '_WIN32_WINNT=0x0500',
                            'UNICODE', '_UNICODE'],
                        CCFLAGS=['/DMSVC_VERSION=$MSVC_VERSION']
                        )
                    )
    



