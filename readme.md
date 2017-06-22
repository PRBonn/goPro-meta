# App to extract images asociated with GPS

## Purpose:
  
Given a GoPro movie with metadata (gpmf format), the application extracts images
with the given framerate and interpolates the GPS coordinates, speed, 
accelerometer, gyro, and any other desired metadata.

## Dependencies

- We use boost for some filesystem stuff, so you need to install it:

```sh
  $ sudo apt install libboost-all-dev
```
- Opencv2.4 needs to be installed in the system: [Link](http://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html)

- We use libyaml-cpp for yaml file creation and parsing, so it needs to be installed. [Link](https://github.com/jbeder/yaml-cpp). Standard cmake pipeline for install:

```sh
  $ git clone https://github.com/jbeder/yaml-cpp.git
  $ cd yaml-cpp/
  $ mkdir build
  $ cd build/
  $ cmake ..
  $ make -j 
  $ sudo make install
```

## Usage

This code uses the gpmf-parser so that submodule has to be cloned into its
folder (../gpmf-parser):

```sh
  $ git submodule init
  $ git submodule update
```

After that we can compile and run the script, which parses the metadata in
video.mp4 at a frame rate of 3fps, and it saves images and yaml file to folder
/tmp/output:

```sh
  $ cd img_gps_extractor
  $ mkdir build && cd build
  $ cmake ..
  $ make -j
  $ ./img_gps_extractor -i video.mp4 -f 3 -o /tmp/output
```


## Format of the output .yaml file:

This is temporary, but it gives an idea of how the final one will look like:

- ts is timestamp in seconds
- gps is:
  - lat (degrees)
  - long (degrees)
  - altitude (meters)
  - 2D earth speed magnitude (m/s)
  - 3D speed magnitude (m/s)

```yaml
000000.jpg:
  ts: 0
  gps:
    lat: 33.1265
    long: -117.3274
    alt: -20.184
    2dv: 0.167
    3dv: 0.19
000001.jpg:
  ts: 1.001
  gps:
    lat: 33.1265
    long: -117.3274
    alt: -20.05719
    2dv: 0.730125
    3dv: 0.689375
```

To parse the yaml file from python:

```python
import yaml
with open("metadata.yaml", 'r') as stream:
  data= yaml.load(stream)
data['000001.jpg']['gps']['lat'] #latitude gps of image 000001.jpg
data['000034.jpg']['ts'] #timestamp for image 000034.jpg
```