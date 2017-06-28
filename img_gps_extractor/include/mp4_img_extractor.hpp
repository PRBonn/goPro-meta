/* 
 * MP4 to frame extractor
 * 
 * Takes in mp4 video from gopro containing metadata processes frame
 * by frame operations, like getting a frame given the timestamp.
 *
 * May 2017 - Andres Milioto
 * 
 */

#ifndef _MP4_IMG_EXTRACTOR_H_
#define _MP4_IMG_EXTRACTOR_H_

// opencv stuff to get images
#include "opencv2/opencv.hpp"

// basic stuff
#include <string>
#include <iostream>
#include "common.hpp"

namespace mp4_img_extractor
{

  typedef enum
  {
    EXTR_OK=0,
    EXTR_SKIPPING_FRAME,
    EXTR_ERROR,
    EXTR_CANT_LOAD_VIDEO,
    EXTR_CANT_FRAME_OUT_OF_BOUNDS,
  }EXTR_RET;

  class img_extractor
  {
    public:
      img_extractor(bool verbose=false);
      img_extractor(const std::string& in, const std::string& out_dir,bool verbose);
      ~img_extractor();
      int32_t init();
      int32_t init(const std::string& in, const std::string& out_dir);
      int32_t get_frame(float ts, float & real_ts, uint32_t idx, std::string &name);

    private:
      std::string _input;
      std::string _output_dir;
      cv::VideoCapture _cap;
      cv::Mat _frame;
      float _duration;
      bool _verbose;
  };

}

#endif // _MP4_IMG_EXTRACTOR_H_