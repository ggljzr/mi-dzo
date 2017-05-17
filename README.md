# mi-dzo

Semestral work for MI-DZO (Digital Images Processing) course at [CTU Prague](https://www.cvut.cz/en). It has three separate parts:

* Image blur based on depth map
* Motion blur
* Hybrid images

For details see ``report.pdf`` (in Czech).

## Requirements

* **depth_blur** and **motion_blur**
    * cmake
    * [OpenCV 3.2.0](http://opencv.org/releases.html)
* **hybrid_images**
    * OpenCV 3.2.0 Python bindings
    * Numpy

Python bindings for OpenCV should be installed with the library.

## Building

Each folder contains ``CmakeLists.txt`` file.

``
$ cmake .
``

``
$ make
``