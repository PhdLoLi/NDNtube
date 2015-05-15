/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

#include "producer-callback.hpp"
#include "video-generator.hpp"
#include <dirent.h>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentiojs
  
  ProducerCallback::ProducerCallback()
  {
    count = 0;
  }
  
  void
  ProducerCallback::setProducer(Producer* p)
  {
    m_producer = p;
  }
  
  void
  ProducerCallback::setSampleNumber(size_t* n)
  {
    m_curnum = n;
  }
  
  void
  ProducerCallback::processConstData(Producer& pro, const Data& data){}
  
  /* When the request can't be satisfied from the content store */
  void
  ProducerCallback::processInterest(Producer& pro, const Interest& interest)
  {
    //if (interet.getName().get(-2).toSegment() < m_crrnFrameNumer)
    
    int sampleNumber =  std::stoi(interest.getName().get(-2).toUri());
//    std::cout << "Current Number" << std::dec << *m_curnum << std::endl;
    if (sampleNumber > *m_curnum)
    {
      std::cout << "My NACK!!!!!!" << std::endl;
      ApplicationNack appNack(interest, ApplicationNack::PRODUCER_DELAY);
      appNack.setDelay(5000); // in ms
      m_producer->nack(appNack);
    }
//    std::cout << "NO HIT Interest!" << interest.getName().toUri() << std::endl;
//    std::cout << "HAHA " <<std::dec<< sampleNumber << std::endl;
  }
  
  void
  ProducerCallback::processIncomingInterest(Producer& pro, const Interest& interest)
  {
    std::cout << "process Incoming Interest " << interest.getName() << std::endl;
  }
  
  bool
  ProducerCallback::verifyInterest(Interest& interest){return true;}
  
  void
  ProducerCallback::processConstInterest(Producer& pro, const Interest& interest){}

  void
  ProducerCallback::generateList(Producer& pro, const Interest& interest)
  {
    std::cout << "Cache Miss of List " << interest.getName() << std::endl;

    std::string key = interest.getName().get(4).toUri();
    if(key == "all")
    {
      std::string filelist;
      filelist = getFilename(); 
      uint64_t timestamp = toUnixTimestamp(ndn::time::system_clock::now()).count();
      std::string timestamp_str = std::to_string(timestamp);
      std::cout << "Timestamp: " << timestamp_str << std::endl;
      Name timeSuffix("all/" + timestamp_str);
      pro.produce(timeSuffix, (uint8_t *)filelist.c_str(), filelist.size());
    }else if(key == "file")
    {
      // NOT IMPLEMENTED YET
      std::string videoFilename = interest.getName().get(5).toUri();
//      videoFilename = filepath + "/" + videoFilename;
      std::cout << "videoFilename" << videoFilename << std::endl;
      VideoGenerator generator;
      std::string status = "DONE!";
      pro.produce(Name("file/"+videoFilename), (uint8_t *)status.c_str(), status.size());
//      generator.h264_generate_whole(prefix, filepath, videoFilename);

    }
  }

  std::string 
  ProducerCallback::getFilename()
  {
    DIR *dir;
    struct dirent *ent;
    std::string filelist;
    std::string filename;
    std::vector<std::string> filevec;
   
    if ((dir = opendir (filepath.c_str())) != NULL) {
      /* print all the files and directories within directory */
      int i = 0;
      while ((ent = readdir (dir)) != NULL) {
        filename = ent->d_name; 
//        if(filename[0] != "."[0])
        if(filename[0] != "."[0] && filename.size()>4 && filename.compare(filename.size()-4, 4, ".mp4") == 0)
        {
          filevec.push_back(filename);
          filelist += filename + "\n";
          std::cout << i << " " << filename << std::endl;
          i++;
        }
      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("");
    }
    return filelist;
  }

} // namespace ndn
