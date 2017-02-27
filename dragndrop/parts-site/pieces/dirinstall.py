'''
Defines a builder object to copy files into specified directory
'''

import SCons.Action
import SCons.Builder
import SCons.Node.FS
import SCons.Node.Python
import SCons.Script

import parts.api as api
import parts.packaging as packaging

import shutil

from parts.errors import GetPartStackFrameInfo, ResetPartStackFrameInfo, SetPartStackFrameInfo

def target_scanner(node, env, _):
    groups = env.MetaTagValue(node, 'groups', 'DirInstall', default=[])
    stackframe = env.MetaTagValue(node, 'stackframe', 'DirInstall', default=None)
    files, _ = env.GetFilesFromPackageGroups(node, groups, stackframe=stackframe)
    env.MetaTag(node, 'DirInstall', files=files)
                                                                     
    return files

def emitter(target, source, env):
    target = env.arg2nodes([target[0].File('tag')])
    SetPartStackFrameInfo()
    try:
        env.MetaTag(target, 'DirInstall', groups=[str(x) for x in source],
                stackframe=GetPartStackFrameInfo())
    finally:
        ResetPartStackFrameInfo()

    return target, source

def action(target, source, env):
    for node in target:
        files = env.MetaTagValue(node, 'files', 'DirInstall', default=[])
        for entry in files:
            try:
                shutil.copy(entry.abspath, node.dir.abspath)
            except BaseException, e:
                api.output.warning_msgf("error: {0}", e, stackframe=env.MetaTagValue(node, 'stackframe', 'DirInstall', default=None))
        with open(node.abspath, "w") as output:
            output.write('\n'.join([x.abspath for x in files]))

class GroupName(SCons.Node.Python.Value):
    @property
    def name(self):
        return str(self)

api.register.add_builder(
    'DirInstall',
    SCons.Builder.Builder(
        action=SCons.Action.Action(action),
        emitter=emitter,
        target_scanner=SCons.Script.Scanner(target_scanner),
        target_factory=SCons.Node.FS.Dir,
        source_factory=GroupName
    )
)

# vim: set et ts=4 sw=4 ai :

