/* 
 * GPMF to YAML Converter
 * 
 * Takes in mp4 video from gopro containing metadata and saves metadata to 
 * yaml file, for easy query from matlab, python or c++.
 *
 * May 2017 - Andres Milioto
 * 
 */

#ifndef _GPMF_TO_YAML_
#define _GPMF_TO_YAML_

// basic stuff
#include <string>
#include <stdint.h>
#include <map>
#include <vector>
#include "common.hpp"

// includes for metadata parsing
#include "GPMF_parser.h"
#include "GPMF_mp4reader.h"
extern "C" void PrintGPMF(GPMF_stream *ms);

// opencv stuff to get images
#include "mp4_img_extractor.hpp"
namespace img_extr = mp4_img_extractor;

// libyaml stuff
#include "yaml-cpp/yaml.h"

namespace gpmf_to_yaml
{
  
  typedef enum
  {
    CONV_OK = 0,
    CONV_ERROR,
    CONV_INIT_ERROR,
    CONV_INPUT_NON_EXISTENT,
    CONV_OUTPUT_NON_EXISTENT,
    CONV_NO_PAYLOAD,
    CONV_CANT_CREATE_OUTPUT,
    CONV_INVALID_STRUCT,
  }CONV_RET;

  typedef struct
  {
    float ts; // timestamp in seconds (from video start)
    float gps[5]; // GPS data (lat deg, long deg, altitude m , 2D ground speed m/s, 3D speed m/s)
    //gpst // GPS time (UTC)
    //gpsf // GPS fix? 0-no lock. 2 or 3, 2D or 3D lock
    //gpsp // GPS precision: Under 300 is good (tipically around 5m to 10m)
    //gyro // IMU Gyroscope data rad/s
    //accel // IMU Accelerometer data m/s²
    //isog // ISO gain (dimensionless)
    //ss // shutter speed in seconds
  }sensorframe_t;

  class converter
  {
    public:
      converter(bool verbose=false);
      converter(const std::string& in, const std::string& out_dir,float fr,bool verbose);
      ~converter();
      int32_t init(); //re-init parsing with same parameters
      int32_t init(const std::string& in, const std::string& out_dir,float fr,const uint32_t idx_offset=0); //init parsing changing parameters
      int32_t cleanup(); //cleanup and exit
      int32_t run(YAML::Emitter & out); //run conversion
      int32_t get_offset(); //offset for next run

    private:
      std::string _input;
      std::string _output_dir;  
      float _fr;
      img_extr::img_extractor _extractor;
      bool _verbose;
      uint32_t _idx_offset;
      YAML::Emitter _out;
      
      // intermediate functions
      int32_t gpmf_to_maps(); // take in stream and build maps
      int32_t populate_images(); // get still images at desired framerate
      int32_t sensors_to_sensorframes(); // interpolate at desired framerate
      int32_t sensorframes_to_yaml(YAML::Emitter & out); // output desired yaml

      // maps for parsed values
      std::map<float,std::vector<float> > _gps; //ts is key, gps data is value

      //map for interpolated values
      std::map<std::string,sensorframe_t> _sensor_frames; //this is what we store in yaml (key is image name, and value is a sensor frame)
      uint32_t _n_images; // final number of images in database

      // gpmf data
      GPMF_stream _metadata_stream, *_ms;
      float _metadatalength;
      uint32_t *_payload; //buffer to store GPMF samples from the MP4.

  };

}

#endif // _GPMF_TO_YAML_