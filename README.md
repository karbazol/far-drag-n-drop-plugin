# far-drag-n-drop-plugin
Automatically exported from code.google.com/p/far-drag-n-drop-plugin
This is a drag-n-drop supporting plug-in for Far Manager. It worked in Far 1.7* and Far 2.0. Need to be implemented again.

# Compilation
The original code has used [SCons](http://www.scons.org/)+[Parts](https://bitbucket.org/sconsparts/parts/overview) for build
and [tut framework](http://mrzechonek.github.io/tut-framework/download.html) for unit tests.
I have failed to configure them, so I have made the solution `dragndrop.sln` for Visual Studio 2015 instead;
old build scripts in `*.parts` are still kept for reference. The solution does not include tests.
`dragndrop.sln` assumes that FAR has been installed into `C:\Program Files\Far Manager` with Plugin SDK,
so `C:\Program Files\Far Manager\PluginSDK\Headers.c\` contains `plugin.hpp`;
fix the include path in `plugin` project if your path is different.
