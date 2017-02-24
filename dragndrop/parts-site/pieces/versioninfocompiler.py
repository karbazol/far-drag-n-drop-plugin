import SCons.Builder
from parts import api
VERSION_INFO_RC_TEMPLATE = \
"""
#include <windows.h>

1 VERSIONINFO
 FILEVERSION ${FILE_VERSION}
 PRODUCTVERSION ${PRODUCT_VERSION}
 FILEFLAGSMASK 0x0L
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x4L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "000004e4"
        BEGIN
            VALUE "CompanyName", "${COMPANY_NAME}"
            VALUE "FileDescription", "${FILE_DESCRIPTION}"
            VALUE "FileVersion", "${FILE_VERSION_STRING}"
            VALUE "InternalName", "${INTERNAL_NAME}"
            VALUE "LegalCopyright", "${LEGAL_COPYRIGHT}"
            VALUE "OriginalFilename", "${ORIGINAL_FILE_NAME}"
            VALUE "ProductName", "${PRODUCT_NAME}"
            VALUE "ProductVersion", "${PRODUCT_VERSION_STRING}"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x0, 1252
    END
END

"""

api.register.add_variable('_dotsToCommas', lambda x: ",".join(x.split('.')), '')
api.register.add_variable('PRODUCT_VERSION_STRING', '$PART_VERSION', '')
api.register.add_variable('PRODUCT_VERSION', '${_dotsToCommas(PRODUCT_VERSION_STRING)}', '')
api.register.add_variable('FILE_VERSION_STRING', '$PRODUCT_VERSION_STRING', '')
api.register.add_variable('FILE_VERSION', '$PRODUCT_VERSION', '')
api.register.add_variable('COMPANY_NAME', '', '')
api.register.add_variable('FILE_DESCRIPTION', '', '')
api.register.add_variable('INTERNAL_NAME', '', '')
api.register.add_variable('ORIGINAL_FILE_NAME', '$INTERNAL_NAME', '')
api.register.add_variable('LEGAL_COPYRIGHT', '', '')
api.register.add_variable('PRODUCT_NAME', '', '')
api.register.add_variable('VERSION_INFO_RC_TEMPLATE', VERSION_INFO_RC_TEMPLATE, '')

def buildVersioInfo(target, source, env):
    with open(str(target[0]), "w") as targetFile:
        targetFile.write(env.subst('$VERSION_INFO_RC_TEMPLATE'))

def emittVersionInfo(target, source, env):
    if len(target) < 1:
        target.append(env.File('${PART_NAME}_version_info.rc'))

    return target, source

api.register.add_builder(
    'VersionInfo',
    SCons.Builder.Builder(
        action=SCons.Action.Action(
            buildVersioInfo,
            cmdstr='Building version info ${TARGETS[0]}',
            varlist=['PRODUCT_VERSION_STRING', 'COMPANY_NAME',
                     'PRODUCT_VERSION', 'FILE_VERSION', 'FILE_VERSION_STRING'
                     'FILE_DESCRIPTION', 'INTERNAL_NAME', 'ORIGINAL_FILE_NAME',
                     'LEGAL_COPYRIGHT', 'PRODUCT_NAME', 'VERSION_INFO_RC_TEMPLATE'
                    ]
            ),
        suffix='.rc',
        emitter=emittVersionInfo
        )
    )

# vim: set et ts=4 sw=4 ai ft=python :

