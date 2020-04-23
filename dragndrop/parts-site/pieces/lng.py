import SCons.Builder
import os
from parts import api
from codecs import open


class LngTemplate:
    def __init__(self, node):
        self.HEADER_NAME = node.name + '.h'
        self.ENCODING = None
        self.LANGUAGES = []
        self.HEADER = ""
        self.FOOTER = ""
        self.ITEMS = []

        with open(node.srcnode().abspath, "rb") as src_file:
            code = compile(src_file.read(), node.srcnode().abspath, "exec")
        exec(code, {}, self.__dict__)

def lngEmitter(target, source, env):
    if len(source) < 1:
        return target, source

    target = []
    for node in source:
        template = LngTemplate(node)
        target.append(template.HEADER_NAME)
        for name, lng, desc in template.LANGUAGES:
            target.append(name)

    return target, source

def lngBuilder(target, source, env):
    index = 0
    for src in source:
        templ = LngTemplate(src)
        # Override encoding with one from env
        if not templ.ENCODING:
            templ.ENCODING = env.get('ENCODING', None)
        assert target[index].name == templ.HEADER_NAME
        header = open(target[index].abspath, "w")
        index += 1
        header.write('\n'.join(templ.HEADER.splitlines()) + '\n\n')
        header.write("enum {\n")
        lngs = []
        for name, lng, desc in templ.LANGUAGES:
            assert target[index].name == name
            file = open(target[index].abspath, "w", encoding = templ.ENCODING)
            if templ.ENCODING.lower() == 'utf-8':
                os.write(file.fileno(), bytes((0xef, 0xbb, 0xbf)))
            file.write(".Language=%s,%s\n\n" % (lng, desc))
            lngs.append(file)
            index += 1

        for item in templ.ITEMS:
            i = 0
            header.write("\t%s,\n" % item[i])
            for lng in lngs:
                i += 1
                lng.write('"%s"\n' % item[i])
        for lng in lngs:
            lng.close()
        header.write("\t};\n")
        header.write('\n'.join(templ.FOOTER.splitlines()) + '\n\n')
        header.close()

    return None


api.register.add_builder(
    'Lng',
    SCons.Builder.Builder(
        action = SCons.Action.Action(
            lngBuilder,
            cmdstr = 'Generating header and lng files from ${SOURCE}'
            ),
        emitter = lngEmitter
        )
    )

# vim: set et ts=4 sw=4 ai :

