// basic stuff
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream> 
#include <string>
#include <algorithm>    // std::sort

// boost program options to parse args
#include "boost/program_options.hpp"
namespace po = boost::program_options;
#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;

// parser
#include "gpmf_to_yaml.hpp"
namespace gp_yml = gpmf_to_yaml;

//config file
#include "config.h"

// separator
std::string sep = "\n=========================================================";
std::string sh_sep = "--------------";

int main(int argc, char *argv[])
{
  int ret;
  bool verbose=false;

  // arguments
  std::string input_file,input_directory,output_dir;
  float framerate = 1; // 1Hz by default

  // parser for command line options
  po::options_description desc("Options"); 
  desc.add_options() 
    ("help", "Print help messages") 
    ("verbose,v", "Verbose") 
    ("input,i",po::value<std::string>(), "Input video with metadata")
    ("directory,d",po::value<std::string>(), "Input directory with partial metadata videos (from one run)")
    ("output,o",po::value<std::string>(), "Output directory for yaml and images")
    ("framerate,f",po::value<float>() ,"Frame rate for image extraction and metadata interpolation"); 

  // parse args
  po::variables_map vm; 
  try 
  { 
    po::store(po::parse_command_line(argc, argv, desc),vm); // can throw 

    // help! 
    if(vm.count("help")) 
    { 
      std::cout << "Extracts metadata (GPS, Gyro, Accelerometer, etc) from" 
                   " GoPro cameras." << std::endl << desc << std::endl; 
      return gp_yml::CONV_OK;
    }

    // check for input video file
    if(vm.count("input")==0 and vm.count("directory")==0)
    {
      std::cerr << "ERROR: Input file/directory is necessary. Exiting..." << std::endl;
      return gp_yml::CONV_INPUT_NON_EXISTENT;
    }
    else if(vm.count("input") and vm.count("directory"))
    {
      std::cerr << "ERROR: Input file/directory are mutually exclusive. Exiting..." << std::endl;
      return gp_yml::CONV_INPUT_NON_EXISTENT;
    }
    else if(vm.count("input"))
    {
      std::cout << sep << std::endl << "Program options:" << std::endl 
                << sh_sep << std::endl; 
      input_file.assign(vm["input"].as<std::string>());
      std::cout << "Input file: " << input_file << std::endl;
    }
    else if(vm.count("directory"))
    {
      std::cout << sep << std::endl << "Program options:" << std::endl 
                << sh_sep << std::endl; 
      input_directory.assign(vm["directory"].as<std::string>());
      std::cout << "Input directory: " << input_directory << std::endl;
    }

    // check for output yaml file
    if(vm.count("output")==0)
    {
      std::cerr << "ERROR: Output directory is necessary. Exiting..." << std::endl;
      return gp_yml::CONV_OUTPUT_NON_EXISTENT;
    }
    else
    {
      //name as desired by user
      output_dir.assign(vm["output"].as<std::string>());
      std::cout << "Output directory: " << output_dir << std::endl;

      // check if directory exists
      fs::path out_path(output_dir);
      if(fs::is_directory(out_path))
      {
        std::cout << "Erasing directory and creating new one..." << std::endl;
        fs::remove_all(out_path);
        if(fs::is_directory(out_path))
        {
          std::cerr << "ERROR: Output directory can't be erased. Exiting..." << std::endl;
          return gp_yml::CONV_OUTPUT_NON_EXISTENT;
        }
      }

      //create directory
      if(!fs::create_directory(out_path))
      {
        std::cerr << "ERROR: Output directory can't be created. Exiting..." << std::endl;
        return gp_yml::CONV_OUTPUT_NON_EXISTENT;
      } 
    }

    // check for frame-rate yaml
    if(vm.count("framerate")==0)
    {
      //use default
      std::cout << "Frame-rate: " << framerate << " fps (default)" << std::endl;  
    }
    else
    {
      // frame-rate desired by user
      framerate = vm["framerate"].as<float>();
      std::cout << "Frame-rate: " << framerate << " fps" << std::endl;  
    }
    std::cout << sep << std::endl;

    // verbose output
    if(vm.count("verbose"))
    {
      // print info of program build
      verbose = true; //for use later
      std::cout << sep << std::endl << "Verbose output:" 
                << std::endl <<  sh_sep << std::endl
                << "Build/git flags: " << std::endl
                << "BUILD_GIT_HASH: " << BUILD_GIT_HASH << std::endl
                << "BUILD_DATE: " << BUILD_DATE << std::endl
                << "BUILD_DEV: " << BUILD_DEV << std::endl
                << sep << std::endl;
    }

    po::notify(vm); // throws on error, so do after help in case 
                    // there are any problems 
  } 
  catch(po::error& e) 
  { 
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << desc << std::endl; 
    return gp_yml::CONV_ERROR; 
  } 

  // list of files:
  std::vector<std::string> files;
  if(vm.count("input"))
  {
    std::cout << "Using single input file: " << input_file << std::endl;
    files.push_back(input_file);
  }
  else //directory
  {
    // get dir
    fs::path in_path(input_directory);
    if (!in_path.empty())
    {
      std::cout << "Using multiple input files from dir: " << input_directory << std::endl;
      for(auto& f:boost::make_iterator_range(fs::directory_iterator(in_path),{}))
      {  
        // check that the file actually is a video
        if(f.path().extension().string() == ".MP4")
        {
          std::cout << "File: " << f.path().string() << std::endl;
          files.push_back(f.path().string());
        }
      }
      /* order files in dir to be in the same order they were taken
       * 1) GOPR0XXX.mp4
       * 2) GP01XXX.mp4
       * 3) GP02XXX.mp4
       * 4) ...
       * Therefore ascending order should be enough
      */
      std::sort(files.begin(),files.end()); 

      // sorted
      std::cout << "Sorted: " << std::endl;
      for(auto& f:files)
        std::cout << "File: " << f << std::endl;  

    }
    else
    {
      std::cerr << "Input dir " << input_directory << std::endl;
      return gp_yml::CONV_ERROR;
    }
  }

  // Open the metadata file and start the map
  // create yaml file
  YAML::Emitter out;
  std::fstream fs;
  std::string filename=output_dir+"/metadata.yaml";
  fs.open(filename, std::fstream::out);
  out << YAML::BeginMap;

  // create a converter instance
  gp_yml::converter parser(verbose);

  //loop for all files in the file list and convert
  uint32_t offset=0;
  for(auto& f:files)
  {
    // init the conversion
    std::cout << sep << std::endl;
    std::cout << "Init conversion for file: " << f << std::endl;
    std::cout << sh_sep << std::endl;
    ret = parser.init(f,output_dir,framerate,offset);
    if(ret)
    {
      std::cerr << "ERROR initializing conversion. Exiting" << std::endl;
      std::cout << sep << std::endl;
      return gp_yml::CONV_ERROR;
    }

    // run the conversion
    std::cout << std::endl << "Run conversion" << std::endl
              << sh_sep << std::endl;
    ret = parser.run(out);
    if(ret)
    {
      std::cerr << "ERROR running conversion. Exiting" << std::endl;
      std::cout << sep << std::endl;
      return gp_yml::CONV_ERROR;
    }
    // cleanup
    parser.cleanup();
    
    std::cout << sep << std::endl;

    // Get offset to initialize next round (next file)
    offset = parser.get_offset();
  }

  // output yaml to file
  out << YAML::EndMap;
  fs << out.c_str();
  // close the file
  fs.close();
  //exit
  return gp_yml::CONV_OK;
  
}
