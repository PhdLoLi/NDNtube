/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

#include "video-generator.hpp"
#include <dirent.h>
#include <streambuf>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
//#include <iostream>
// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions

  std::vector<std::string> 
  getFilenameVec(std::string filepath, std::string &filelist)
  {
    DIR *dir;
    struct dirent *ent;
    std::string filename;
    std::vector<std::string> filediff;

    long length = 0;
    std::string configname = filepath + "/config";

    std::ifstream t(configname);
    std::stringstream buffer;
    buffer << t.rdbuf();
    filelist = buffer.str();


   
    if ((dir = opendir (filepath.c_str())) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
        filename = ent->d_name; 
        if(filename[0] != "."[0] && filename.size()>4 && filename.compare(filename.size()-4, 4, ".mp4") == 0 && filelist.find(filename) == std::string::npos)
        {
          filediff.push_back(filename);
          std::cout << "Add New File: "<<  filename << std::endl;
        }
      }
      closedir (dir);

    } else {
      /* could not open directory */
      perror ("");
    }
      
    std::cout << filediff.size() << " Files Added"<< std::endl; 
    return filediff;
  }


  int
  main(int argc, char** argv)
  {
    try {
      
      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini("../config.ini", pt);
      std::cout << "Video FilePath: " << pt.get<std::string>("video.path") << std::endl;
      std::cout << "Video Prefix: " << pt.get<std::string>("video.prefix") << std::endl; 

      std::string prefix = pt.get<std::string>("video.prefix");;
      VideoGenerator generator;

      std::string filepath = pt.get<std::string>("video.path");
      std::string videoFilename;
      std::string filelist;
      std::vector<std::string> filediff;
      
      if (argc > 1)
      {
        if(argv[1] != "all")
        {
          videoFilename = argv[1];
          generator.h264_generate_whole(prefix, filepath, videoFilename);
          sleep(5000000);
        }
        return 0;
      }

      while(1)
      {
        filediff = getFilenameVec(filepath, filelist); 
        std::cout << "file list before:\n" << filelist << std::endl;
        for(int i=0; i<filediff.size(); i++)
        {
          videoFilename = filediff[i];
          generator.h264_generate_whole(prefix, filepath, videoFilename);

          std::ofstream myfile;
          myfile.open (filepath + "/config", std::ios::out | std::ios::app);
          myfile << videoFilename + "\n";
          myfile.close();

          filelist += videoFilename + "\n" ;
        }
        std::cout << "\nfile list after:\n " << filelist << std::endl;

//        filelist_new = filelist_old;
        sleep(60);
      }

      if (argc > 1)
      {
        if(argv[1] != "all")
        {
          videoFilename = argv[1];
          generator.h264_generate_whole(prefix, filepath, videoFilename);
        }
      }
    
      std::cout << "OVER!" << std::endl;
      sleep(30000); // because attach() is non-blocking
      
      std::cout << "HERE!!" << std::endl;

    }
    catch(std::exception& e) {
      std::cout << "HAHA" << std::endl;
      std::cerr << "ERROR: " << e.what() << std::endl;
      return 1;
    }
    return 0;
  }

} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::main(argc, argv);
}
