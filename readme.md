> [!IMPORTANT]
> **Repository Archived**  
> This repository has been archived and is no longer actively maintained. You are welcome to explore the code, but please note that no further updates, issues, or pull requests will be accepted.
>
> Thank you for your interest and contributions.</p>


# App to extract images associated with GPS

## Purpose:
  
Given a GoPro movie with metadata (gpmf format), the application extracts images
with the given framerate and interpolates the GPS coordinates, speed, 
accelerometer, gyro, and any other desired metadata.

## Dependencies

- Boost (for filesystem stuff):

```sh
  $ sudo apt install libboost-all-dev
```
- Opencv3: [Link](http://docs.opencv.org/3.0-beta/doc/tutorials/introduction/linux_install/linux_install.html)

- libyaml-cpp for yaml file creation and parsing: [Link](https://github.com/jbeder/yaml-cpp):

  - Ubuntu 16.04 (apt-get installs v0.5.2):

    ```sh
      $ sudo apt-get install libyaml-cpp-dev
    ```
  - Ubuntu 14.04 (apt-get installs v0.5.1 which does not contain yaml-cpp-config.cmake):

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

As a design choice, the GoPro never saves videos bigger than 4Gb (not even when 
SD is extFat). If a video is bigger than this, it splits it into sub videos, 
with a sort of complicated way to handle the metadata. If this is the case, 
use the -d option, instead of the -i option. 

The following example gets all the videos that correspond to one
video in the folder /tmp/input and it deals with everything so that you don't have
to merge them externally (not even ffmpeg is currently able to merge gopro videos
keeping the metadata):

```sh
  $ ./img_gps_extractor -d /tmp/input -f 3 -o /tmp/output
```

The directory should look something like this:

```sh
  $ tree /tmp/input
  /tmp/input
  ├── GOPRXXXX.MP4
  ├── GP01XXXX.MP4
  └── GP02XXXX.MP4
```

The only check that we do is for the .MP4 extension and then we order in alphabetical 
order to recover the order structure, so don't rename the files please :)


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
000000.jpg:  # Original File: /tmp/input/GOPRXXXX.MP4, Frame rate: 0.020000
  ts: 0
  gps:
    lat: 50.72745
    long: 7.087335
    alt: 121.54
    2dv: 0.061
    3dv: 0.11
000001.jpg:
  ts: 50.00829
  gps:
    lat: 50.72681
    long: 7.088741
    alt: 117.3692
    2dv: 7.321869
    3dv: 7.33
000002.jpg:  # Original File: /tmp/input/GP01XXXX.MP4, Frame rate: 0.020000 (-< when the video it was taken from changes, we comment!)
  ts: 99.99573
  gps:
    lat: 50.7245
    long: 7.09001
    alt: 91.91341
    2dv: 7.956978
    3dv: 7.976304
```

To parse the yaml file from python:

```python
import yaml
with open("metadata.yaml", 'r') as stream:
  data= yaml.load(stream)
data['000001.jpg']['gps']['lat'] #latitude gps of image 000001.jpg
data['000034.jpg']['ts'] #timestamp for image 000034.jpg
```
# Comments and credits

- Only gps extraction is implemented so far. More metadata extraction 
will be added.

- Libraries used:
  - [Boost](https://github.com/boostorg/boost) ([@boostorg](https://github.com/boostorg))
  - [GoPro GPMF Parser](https://github.com/gopro/gpmf-parser) ([@gopro](https://github.com/gopro))
  - [OpenCV](https://github.com/opencv/opencv) ([@opencv](https://github.com/opencv))
  - [libyaml-cpp](https://github.com/jbeder/yaml-cpp) ([@jbeder](https://github.com/jbeder))
