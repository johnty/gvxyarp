ofxgvf
=============

This is a visualiser of the GVF library for 2-dimensional gestures.

Copy the ofxgvfVisualiser folder to openframewroks myapps folder. 

Link with the library (GestureVariationFollower.*) by reimporting these source files into the xCode project.

Run and play!

===========

updated by johnty:

yarpified mouse input; use with YarpMouse eclipse processing project.

need to set Project.xcconfig with yarp/ace source and build trees to compile (static lib for yarp; dylib for ace), and will need libACE.dylib in usr/lib to run.