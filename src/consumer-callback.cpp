/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang phdloli@ucla.edu
 */

#include "video-player.hpp"
#include "consumer-callback.hpp"
#include <string>
#include <pthread.h>
#include <ctime>
#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions

  int times_video = 0;
  int times_audio = 0;
//  time_t time_start;
//  time_t time_end;

  ConsumerCallback::ConsumerCallback()
  {
    data_ready = false;
    data_ready_payload_v = false;
    data_ready_payload_a = false;
    payload_v = 0;
    payload_a = 0;
    interest_s = 0;
    interest_r = 0;
    interest_retx = 0;
    interest_expr = 0;

//    std::cout << "Construction" << std::endl;
//    player.playbin_appsrc_init();
//    std::cout << "Construction Over" << std::endl;
  }
  
  void
  ConsumerCallback::processPayload(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
//    std::cout << "video times processPayload " << std::dec << times_video <<std::endl;
//    std::cout << "video bufferSize " << bufferSize <<std::endl;
//    std::cout << "@buffer " << &buffer <<std::endl;
    payload_v += bufferSize;
    player.h264_appsrc_data(buffer, bufferSize);
    times_video ++;
    boost::lock_guard<boost::mutex> lock(mut_payload_v);
    data_ready_payload_v = true;
    cond_payload_v.notify_all();
//    data_ready_payload_v = false;
//    std::cout << "processPayload video over " << std::endl;
  }

  void
  ConsumerCallback::processPayloadAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
//    std::cout << "audio times processPayload " << std::dec << times_audio <<std::endl;
//    std::cout << "audio bufferSize " << bufferSize <<std::endl;
//    std::cout << "@buffer " << &buffer <<std::endl;
    payload_a += bufferSize;
    player.h264_appsrc_data_audio(buffer, bufferSize);
    times_audio ++;
    boost::lock_guard<boost::mutex> lock(mut_payload_a);
    data_ready_payload_a = true;
    cond_payload_a.notify_all();
//    data_ready_payload_a = false;
//    std::cout << "processPayload audio over " << std::endl;
  }
  
  void
  ConsumerCallback::processStreaminfo(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
    Name suffix;
    con.getContextOption(SUFFIX, suffix);
    std::string suffix_str = suffix.get(0).toUri();
//    std::cout << "suffix_str: " << suffix_str << std::endl;
    if(suffix_str == "pipeline")
    {
      std::string streaminfo((char*) buffer);
    //  long fileLength = std::stol(content);
    //  std::cout << "bufferSize " << bufferSize <<std::endl;
    //  std::cout << "buffer " << buffer <<std::endl;
   //   std::cout << "streaminfo " << streaminfo <<std::endl;
    //  std::cout << "fileLength " << fileLength <<std::endl;
 //     std::cout << "processStreaminfo " << streaminfo << std::endl;
      player.get_streaminfo(streaminfo);
    }else
    {
      finalframe_video = atoi((char*) buffer); 
      std::cout << "video finalframe " << finalframe_video << std::endl;
    }
  }

  void
  ConsumerCallback::processStreaminfoAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
    Name suffix;
    con.getContextOption(SUFFIX, suffix);
    std::string suffix_str = suffix.get(0).toUri();
//    std::cout << "suffix_str: " << suffix_str << std::endl;
    if(suffix_str == "pipeline")
    {
      std::string streaminfo((char*) buffer);
//      std::cout << "processStreaminfo_audio " << streaminfo << std::endl;
      player.get_streaminfo_audio(streaminfo);
    }else
    {
      finalframe_audio = atoi((char*) buffer); 
      std::cout << "audio finalframe " << finalframe_audio << std::endl;
    }
  }

  void
  ConsumerCallback::processData(Consumer& con, const Data& data)
  {
    interest_r++;
//    std::cout << "DATA IN CNTX Name: " << data.getName() << "  FinalBlockId: " <<data.getFinalBlockId() << std::endl;
  }
  
  bool
  ConsumerCallback::verifyData(Consumer& con, Data& data)
  {
    return true;
  }
  
  void
  ConsumerCallback::processConstData(Consumer& con, const Data& data)
  {}
  
  void
  ConsumerCallback::processLeavingInterest(Consumer& con, Interest& interest)
  {
    interest_s ++;
//    std::cout << "LEAVES " << interest.toUri() << std::endl;
//    std::cout << "LEAVES name " << interest.getName() << std::endl;
  }  

  void
  ConsumerCallback::processList(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
    std::cout << "Video List: " << std::endl;
    std::vector<std::string> strs;
    boost::split(strs, (char *&)buffer, boost::is_any_of("\n"));

    for (int i=0; i<strs.size()-1; i++)
    {
      list.push_back(strs[i]);
      std::cout << i << "  " << strs[i] << std::endl ;
    }
    boost::lock_guard<boost::mutex> lock(mut);
    data_ready=true;
    cond.notify_one();
  }

  void
  ConsumerCallback::onRetx(Consumer& con, Interest& interest)
  {
    interest_retx ++;
//    std::cout << "Retransmitted " << interest.getName() << std::endl;
  }

  void
  ConsumerCallback::onExpr(Consumer& con, Interest& interest)
  {
    interest_expr ++;
//    std::cout << "Expired " << interest.getName() << std::endl;
  }

} // namespace ndn
