### English Version can be found below

Это плагин, позволяющий таскать файлы с и на фар.

### English version

## Building the plug-in ##

Before you build the plug-in run 
```
#!bash

git submodule update --init
```
to make sure you have downloaded [Parts](https://bitbucket.org/sconsparts/parts) and [Tut framework](https://github.com/mrzechonek/tut-framework.git) to the repository.

To control build process I use [SCons](https://bitbucket.org/scons/scons) a softwore construction tool which can be treated as advanced 'Make' utility. To run scons you need Python 2.7 to be installed on your machine. I use [ActivePython 2.7](http://www.activestate.com/activepython/downloads). After you have download and installed Python you can install SCons by running command:
```
#!bash
easy_install scons
```
Version 2.5.1 worked for me.

The plug-ins code is built using MSVC 2012 atleast. You need to install it.

After you have set up the environment go to 'dragndrop' directory and run command:
```
#!bash
scons CONFIG=release build::
```

You can specify '-j N' option where N is number of CPUs installed on your machine. The option will make scons build the plug-in in multiple threads.

# MSVC only Compilation
The original code has used [SCons](http://www.scons.org/)+[Parts](https://bitbucket.org/sconsparts/parts/overview) for build
and [tut framework](http://mrzechonek.github.io/tut-framework/download.html) for unit tests.
I have failed to configure them, so I have made the solution `dragndrop.sln` for Visual Studio 2015 instead;
old build scripts in `*.parts` are still kept for reference. The solution does not include tests.
`dragndrop.sln` assumes that FAR has been installed into `C:\Program Files\Far Manager` with Plugin SDK,
so `C:\Program Files\Far Manager\PluginSDK\Headers.c\` contains `plugin.hpp`;
fix the include path in `plugin` project if your path is different.
