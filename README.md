# raspi_yolo_test
## Overview
(Semi-)Real-time YOLO prediction with NNPACK on Raspberry pi.

This runs YOLOv2 tiny prediction on the images taken by Raspberry PI camera module, and constantly shows the result on your screen.

Output rate is around 0.5~1.0 fps. Image size is 320x240.

## Hardware
- Raspberry Pi 3
- Camera module

## Requirement
Install OpenCV 2.
```
sudo apt-get install libopencv-dev
```

Then, install these repositories following the README.


darknet-nnpack

https://github.com/jonatechout/darknet-nnpack


raspicam

https://github.com/cedricve/raspicam


## Build and install
1. Open Makefile, and edit INCS and LIBDIR to your darknet-nnpack directory path.
```
INCS = -I(your darknet-nnpack path)/include/
LIBDIR = -L(your darknet-nnpack path)
```

2. build it.
```
make
```

3. Copy some files from darknet-nnpack. (This has to be done because of hard-code path in darknet.)
```
cp -r ../darknet-nnpack/cfg/ ../darknet-nnpack/data/ .
# Change directory name if your structure is different.
```
4. Download Yolo weight file ("yolov2-tiny-voc.weights" or similar) from [YOLO homepage](https://pjreddie.com/darknet/yolo/).
 Place it in the repository root directory.
 
## Run
```
./raspiyolotest cfg/voc.data cfg/yolov2-tiny-voc.cfg yolov2-tiny-voc.weights 
```

## End
Ctrl + C on the terminal.
