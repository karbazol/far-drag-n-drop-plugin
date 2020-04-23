import subprocess
import xml.dom.minidom as xml
from parts import api

def getSvnRevision(env):
    try:
        so, se = subprocess.Popen('svn info --xml %s' % env.subst('$SRC_DIR'),
                stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        d = xml.parseString(so)
        buildstr = d.getElementsByTagName('entry')[0].getAttribute('revision')
        return int(buildstr)
    except Exception:
        return -1

api.register.add_variable('_getSvnRevision', getSvnRevision, '')
api.register.add_variable('SVN_REVISION', '${_getSvnRevision(__env__)}', '')

# vim: set et ts=4 sw=4 ft=python ai :

