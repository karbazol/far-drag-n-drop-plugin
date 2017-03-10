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

To control build process I use [SCons](https://bitbucket.org/scons/scons) a softwore
construction tool which can be treated as advanced 'Make' utility.
It is written in Python and alows you to build software for different platforms in a single run.
To run scons you need Python 2.7 to be installed on your machine. 
I use [ActivePython 2.7](http://www.activestate.com/activepython/downloads).
After you have download and installed Python you can install SCons by running command:
```
#!bash
easy_install scons
```
Version 2.5.1 worked for me.

The plug-ins code is built using MSVC 2012 atleast. You need to install it.

After you have set up the environment go to `dragndrop` directory and run command:
```
#!bash
scons CONFIG=release build::
```

You can specify `-j N` option where N is number of CPUs installed on your machine. The option will make scons build the plug-in in multiple threads.

# MSVC 2015 only Compilation
If you don't want to play with Python, SCons, and Parts you can try `dragndrop.sln` file located in `dragndrop/msvs2015` directory.
