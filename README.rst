Deep-SpeedDreams
##############################


Intention
====================

This is a fork of the SpeedDreams project (version 2.2.1) which works together with the DeepDriving implementation from this repository (*ToDo: Add Link, if finished*). It allows to control a robot-car in this game by calculating 14 driving indicators out of the front-camera picture. 

The original DeepDriving implementation is a project from the princeton university (http://deepdriving.cs.princeton.edu/).

If you are simply looking for the original SpeedDreams project, please look at http://www.speed-dreams.org. Since there are some compile issues on windows with the original version, you can also checkout the tag (*ToDo*) from this respository, where several issues are fixed, without containing additional code from the DeepDriving project.

Index
====================

* Installation
    * Compiling from Source (Windows)
    * Compiling from Source (Linux)  
* Configuration
* Adapting the Camera
    * Framebuffer output (current Screen)
	* Adding a second Camera as Output
* Accessing the Output with Python

====================
* Licence
====================

The SpeedDreams project is published under the GPL license. All extensions which came from the DeepDriving project are published under the licence from the original DeepDriving Code. All other extensions to the code are published under the GPLv3 licence if not stated differently in code.

The python modules on this repository are published under the MIT licence. Please consider the file COPYING.txt and LICENCE.txt in the repository root and sub-directories for more licence information.