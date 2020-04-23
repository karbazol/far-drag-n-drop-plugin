### English Version can be found below

Это плагин, позволяющий таскать файлы с и на фар.

### English version

## Building the plug-in ##

Before you build the plug-in run 
```
git submodule update --init
```
to make sure you have downloaded [Parts](https://bitbucket.org/SConsparts/parts) and [Tut framework](https://github.com/mrzechonek/tut-framework.git) to the repository.

To control build process I use [SCons](https://bitbucket.org/SCons/SCons) a software
construction tool which can be treated as advanced 'Make' utility.
It is written in Python and allows you to build software for different platforms in a single run.
To run SCons you need Python 3.8.1 to be installed on your machine. 
After you have download and installed Python you can install SCons by running command:
```
easy_install SCons
```
Version 3.1.2 worked for me.

The plug-ins code is built using MSVC 2015 at least. You need to install it.

After you have set up the environment go to `dragndrop` directory and run command:
```
SCons CONFIG=release build::
```

You can specify `-j N` option where N is number of CPUs installed on your machine. The option will make SCons build the plug-in in multiple threads.

# MSVC 2015 only Compilation
If you don't want to play with Python, SCons, and Parts you can try `dragndrop.sln` file located in `dragndrop/msvs2015` directory.
