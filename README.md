# Deep-SpeedDreams

## Intention

This is a fork of the SpeedDreams project (version 2.2.1) which works together with the DeepDriving implementation from the repository **ToDo: Add Link, if finished**. It allows to control a robot-car in this game by calculating 14 driving indicators out of the front-camera picture. 

The original DeepDriving implementation is a project from the princeton university (http://deepdriving.cs.princeton.edu/).

If you are simply looking for the original SpeedDreams project, please visit http://www.speed-dreams.org. Since there are some compile issues on windows with the original version, you can also checkout the tag ([original-2.2.1](https://bitbucket.org/Netzeband/deep-speeddreams/src/original-2.2.1)) from this respository, where several issues are fixed, without containing additional code from the DeepDriving project.

## Index

* Installation
    * [Compiling from Source (Windows)](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/InstallationSourceWindows)
    * [Compiling from Source (Linux)](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/InstallationSourceLinux)
* [Configuration](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/Configuration)
* [Adapting the Camera](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/Camera)
    * [Record Images from the Main Camera](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/Camera#markdown-header-record-images-from-the-main-camera)
    * [Record Images from a separate Camera](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/Camera#markdown-header-record-images-from-a-separate-camera)
    * [Shared-Memory](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/Camera#markdown-header-shared-memory)
* [Accessing the Output with Python](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/RecordInPython)
* [Label-Description](https://bitbucket.org/Netzeband/deep-speeddreams/wiki/LabelDescription)

## Licence

The SpeedDreams project is published under the GPL license. All extensions which came from the DeepDriving project are published under the licence from the original DeepDriving Code. All other extensions to the code are published under the GPLv3 licence if not stated differently in code.

The python modules on this repository are published under the MIT licence. Please consider the file COPYING.txt and LICENCE.txt in the repository root and sub-directories for more licence information.