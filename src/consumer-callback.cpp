/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang wanglj11@mails.tsinghua.edu.cn 
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
    frame_cnt_v = 0;
    frame_cnt_a = 0;
    frame_rate_v = 7;
    frame_rate_a = 6;

//    std::cout << "Construction" << std::endl;
//    player.playbin_appsrc_init();
//    std::cout << "Construction Over" << std::endl;
  }
  
  void
  ConsumerCallback::processPayload(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
      frame_cnt_v_m.lock();

      Name suffix;
      con.getContextOption(SUFFIX, suffix);
      int frameNumber = std::stoi(suffix.get(0).toUri());
//      printf("Video Frame Counter: %d suffix: %s frameNumber: %d\n", frame_cnt_v, suffix.toUri().c_str(), frameNumber);
      payload_v += bufferSize;
      frame_cnt_v++;

      if (buffer == NULL) 
        printf("NONONO buffer == NULL\n");
      if (bufferSize == 0)
        printf("NONONO buffersize == 0\n");

      if (buffer != NULL && bufferSize > 0) {
        uint8_t* bufferTmp = new uint8_t[bufferSize];
        memcpy(bufferTmp, buffer, bufferSize);
        buffers_v[frameNumber % frame_rate_v].length = bufferSize;
        buffers_v[frameNumber % frame_rate_v].data = bufferTmp;
      }
      frame_cnt_v_m.unlock();
//    data_ready_payload_v = false;
//    std::cout << "processPayload video over " << std::endl;
  }

  void
  ConsumerCallback::processPayloadAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {

      frame_cnt_a_m.lock();

      Name suffix;
      con.getContextOption(SUFFIX, suffix);
      int frameNumber = std::stoi(suffix.get(0).toUri());
//      printf("Audio Frame Counter: %d suffix: %s frameNumber: %d\n", frame_cnt_a, suffix.toUri().c_str(), frameNumber);
      payload_a += bufferSize;
      frame_cnt_a++;

      if (buffer == NULL) 
        printf("NONONO buffer == NULL\n");
      if (bufferSize == 0)
        printf("NONONO buffersize == 0\n");

      if (buffer != NULL && bufferSize > 0) {
        uint8_t* bufferTmp = new uint8_t[bufferSize];
        memcpy(bufferTmp, buffer, bufferSize);
        buffers_a[frameNumber % frame_rate_a].length = bufferSize;
        buffers_a[frameNumber % frame_rate_a].data = bufferTmp;
      }

      frame_cnt_a_m.unlock();
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

      std::cout << "processStreaminfo " << streaminfo << std::endl;
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
      std::cout << "processStreaminfo_audio " << streaminfo << std::endl;
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
//    boost::lock_guard<boost::mutex> lock(interest_r_m);
    interest_r_m.lock();
    interest_r++;
    interest_r_m.unlock();
//    printf("DATA IN CNTX Name: %s  FinalBlockId: %s\n", data.getName().toUri().c_str(), data.getFinalBlockId().toUri().c_str());
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
//    boost::lock_guard<boost::mutex> lock(interest_s_m);
    interest_s_m.lock();
    interest_s ++;
    interest_s_m.unlock();
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
//    boost::lock_guard<boost::mutex> lock(interest_retx_m);
    interest_retx_m.lock();
    interest_retx ++;
    interest_retx_m.unlock();
    printf("Retransmitted %s\n", interest.getName().toUri().c_str());
  }

  void
  ConsumerCallback::onExpr(Consumer& con, Interest& interest)
  {
//    boost::lock_guard<boost::mutex> lock(interest_expr_m);
    interest_expr_m.lock();
    interest_expr ++;
    interest_expr_m.unlock();
    printf("Expired %s\n", interest.getName().toUri().c_str());
  }

} // namespace ndn
