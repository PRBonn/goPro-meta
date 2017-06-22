// basic stuff
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream> 
#include <string>

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
std::string sep = "\n=============================================";
std::string sh_sep = "--------------";

int main(int argc, char *argv[])
{
	int ret;
	bool verbose=false;

	// arguments
	std::string input_file,output_dir;
	float framerate = 1; // 1Hz by default

	// parser for command line options
	po::options_description desc("Options"); 
  desc.add_options() 
    ("help", "Print help messages") 
    ("verbose,v", "Verbose") 
    ("input,i",po::value<std::string>(), "Input video with metadata")
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
    if(vm.count("input")==0)
		{
			std::cerr << "ERROR: Input file is necessary. Exiting..." << std::endl;
			return gp_yml::CONV_INPUT_NON_EXISTENT;
		}
		else
		{
			std::cout << sep << std::endl << "Program options:" << std::endl 
								<< sh_sep << std::endl; 
			input_file.assign(vm["input"].as<std::string>());
			std::cout << "Input file: " << input_file << std::endl;
		}

		// check for output yaml file
		if(vm.count("output")==0)
		{
			std::cerr << "ERROR: Output folder is necessary. Exiting..." << std::endl;
			return gp_yml::CONV_OUTPUT_NON_EXISTENT;
		}
		else
		{
			//name as desired by user
			output_dir.assign(vm["output"].as<std::string>());
			std::cout << "Output folder: " << output_dir << std::endl;

			// check if folder exists
			fs::path dir(output_dir);
			if(fs::is_directory(dir))
			{
				std::cout << "Erasing folder and creating new one..." << std::endl;
				fs::remove_all(dir);
				if(fs::is_directory(dir))
				{
					std::cerr << "ERROR: Output folder can't be erased. Exiting..." << std::endl;
					return gp_yml::CONV_OUTPUT_NON_EXISTENT;
				}
			}

			//create folder
			if(!fs::create_directory(dir))
			{
				std::cerr << "ERROR: Output folder can't be created. Exiting..." << std::endl;
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

  // create the converter object
	gp_yml::converter parser(input_file,output_dir,framerate,verbose);

	// init the conversion
	std::cout << sep << std::endl;
	std::cout << "Init conversion" << std::endl;
	std::cout << "---------------" << std::endl;
	ret = parser.init();
	if(ret)
	{
		std::cerr << "ERROR initializing conversion. Exiting" << std::endl;
		std::cout << sep << std::endl;
		return gp_yml::CONV_ERROR;
	}

	// run the conversion
	std::cout << sep << std::endl << "Run conversion" << std::endl
						<< sh_sep << std::endl;
	ret = parser.run();
	if(ret)
	{
		std::cerr << "ERROR running conversion. Exiting" << std::endl;
		std::cout << sep << std::endl;
		return gp_yml::CONV_ERROR;
	}
	std::cout << sep << std::endl;
	return gp_yml::CONV_OK;
}
