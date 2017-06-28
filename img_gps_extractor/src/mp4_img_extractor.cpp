/* 
 * MP4 to frame extractor
 * 
 * Takes in mp4 video from gopro containing metadata processes frame
 * by frame operations, like getting a frame given the timestamp.
 *
 * May 2017 - Andres Milioto
 * 
 */

#include <ctime>
#include "mp4_img_extractor.hpp"

namespace mp4_img_extractor
{
  img_extractor::img_extractor(bool verbose):_verbose(verbose)
  {
  }

  img_extractor::img_extractor(const std::string& in,
                               const std::string& out_dir,
                               bool verbose):
                               _input(in),_output_dir(out_dir),
                               _verbose(verbose),_cap(_input)
  {
  }

  img_extractor::~img_extractor()
  {
  }
  
  int32_t img_extractor::init()
  {
    if(!_cap.isOpened())
    {
      // error!
      DEBUG("Can not load video. Exiting...\n");
      return EXTR_CANT_LOAD_VIDEO;
    }
    else
    {
      // Success!
      DEBUG("Opened cv capture successfully...\n");
    }

    float frames = _cap.get(CV_CAP_PROP_FRAME_COUNT);
    DEBUG("There are %f frames in the video.\n",frames);
    float fps = _cap.get(CV_CAP_PROP_FPS);
    DEBUG("Fps is %f for video.\n",fps);
    _duration = frames / fps;
    DEBUG("Duration of video is %f.\n",_duration);

    return EXTR_OK;
  }

  int32_t img_extractor::init(const std::string& in, const std::string& out_dir)
  {
    _input = in;
    _output_dir = out_dir;
    _cap.release();
    _cap = cv::VideoCapture(_input);

    // init
    int ret = init();

    return ret;
  }
  

  // gets the frame closest to ts and returns the real ts extracted and 
  // the filename
  int32_t img_extractor::get_frame(float ts, float & real_ts, uint32_t idx, std::string &name)
  {
    int ret = EXTR_OK;

    // Check if timestamp is within boundaries
    if(ts > _duration)
    {
      DEBUG("Frame %f requested is out of bounds. Exiting\n",ts);
      return EXTR_CANT_FRAME_OUT_OF_BOUNDS;
    }

    // timestamp comes in seconds, and opencv works in ms, so convert:
    float timestamp = ts * 1000;

    // set the timestamp in video to obtain frame
    _cap.set(CV_CAP_PROP_POS_MSEC,timestamp);
    DEBUG("Setting timestamp to %.5f\n",timestamp);

    // get real timestamp to return
    real_ts = _cap.get(CV_CAP_PROP_POS_MSEC);
    DEBUG("Real timestamp set to %.5f\n",real_ts);
  
    // get frame
    std::clock_t begin_time = std::clock();
    _cap >> _frame;

    // if frame was not captured, try again n times
    int tries=1;
    while(_frame.empty() && tries<=50)
    {
      DEBUG("Frame skipped trying again for %d time: Real timestamp set to %.5f\n",tries,real_ts);
      real_ts = _cap.get(CV_CAP_PROP_POS_MSEC);
      _cap >> _frame;
      tries++;
    }

    // 50 times is an exaggeration, if it still didn't work, something is wrong,
    // report back to user.
    if(_frame.empty())
    {
      std::cerr << "Skipping frame at " << real_ts << "ms. Exiting" <<std::endl;
      return EXTR_SKIPPING_FRAME;
    }

    DEBUG("Time cv cap>>frame: %f\n",float(clock()-begin_time)/CLOCKS_PER_SEC);

    // create filename as real ts with 6 digits(assume less than 1 million imgs)
    // pad with 0's
    name = std::to_string(idx);
    int zero_pad = 6 - name.length();
    int i = 0;
    while(i < zero_pad)
    {
      name = '0' + name;
      i++;
    }

    // generate the path
    name+=".jpg";
    std::string save_path = _output_dir + "/" + name;

    // write the frame
    DEBUG("Saving image in %s\n", save_path.c_str());
    begin_time = std::clock();
    cv::imwrite(save_path,_frame);
    DEBUG("Time cv::imwrite: %f\n",float(clock()-begin_time)/CLOCKS_PER_SEC);

    // return real_ts in seconds again
    real_ts /= 1000;

    return ret;

  }

}