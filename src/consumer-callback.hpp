/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang wanglj11@mails.tsinghua.edu.cn 
 */


#ifndef CONSUMER_CALLBACK_HPP
#define CONSUMER_CALLBACK_HPP
#include <Consumer-Producer-API/consumer-context.hpp>
#include <fstream>
#include <iostream>
#include "video-player.hpp"

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions

  struct DataNode_G
  {
    uint64_t length;
    uint8_t *data;
    DataNode_G(uint64_t len, uint8_t *d) : length(len), data(d)
    {
    }
  };

  class ConsumerCallback
  {
      
  public:
    ConsumerCallback();
    
    void
    processTestPayload(Consumer& con, const uint8_t* buffer, size_t bufferSize) 
    {
      payload_v += bufferSize;
      frame_cnt_v++;
      printf("Video Frame: %d\n", frame_cnt_v);
    }

    void 
    processTestPayloadAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize) 
    {
      payload_a += bufferSize;
      frame_cnt_a++;
      printf("Audio Frame: %d\n", frame_cnt_a);
    }

    void
    wait_rate()
    {
      boost::unique_lock<boost::mutex> lock(player.rate_mut);
      while(!player.rate_ready)
      {
//        std::cout << "waiting for rate" << std::endl;
        player.rate_con.wait(lock);
//        std::cout << "waiting for rate over" << std::endl;
      }
      frame_rate_v = player.get_video_rate() / 1;
      frame_rate_a = player.get_audio_rate() / 1;
      buffers_v.resize(frame_rate_v, DataNode_G(0, NULL));
      buffers_a.resize(frame_rate_a, DataNode_G(0, NULL));
    }

    void
    processPayload(Consumer& con, const uint8_t* buffer, size_t bufferSize);
    
    void
    processStreaminfo(Consumer& con, const uint8_t* buffer, size_t bufferSize);

    void
    processPayloadAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize);
    
    void
    processStreaminfoAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize);
    
    void
    processData(Consumer& con, const Data& data);
    
    bool
    verifyData(Consumer& con, Data& data);
    
    void
    processConstData(Consumer& con, const Data& data);
    
    void
    processLeavingInterest(Consumer& con, Interest& interest);
    
    void
    processList(Consumer& con, const uint8_t* buffer, size_t bufferSize);


    void
    onRetx(Consumer& con, Interest& interest);

    void
    onExpr(Consumer& con, Interest& interest);

    void
    processFile(Consumer& con, const uint8_t* buffer, size_t bufferSize)
    {
      std::ofstream filestr;
    	// binary open
      
    	filestr.open ("/Users/lijing/next-ndnvideo/build/haha.mp3",  std::ios::out | std::ios::app | std::ios::binary);
      filestr.write((char *)buffer, bufferSize);
      filestr.close();
 
    }

    VideoPlayer player;
    std::vector<std::string> list;
    boost::condition_variable cond;
    boost::mutex mut;
    bool data_ready;
    boost::condition_variable cond_payload_v;
    boost::mutex mut_payload_v;
    bool data_ready_payload_v;
    boost::condition_variable cond_payload_a;
    boost::mutex mut_payload_a;
    bool data_ready_payload_a;

    boost::mutex frame_cnt_v_m;
    boost::mutex frame_cnt_a_m;
    boost::mutex interest_s_m;
    boost::mutex interest_r_m;
    boost::mutex interest_retx_m;
    boost::mutex interest_expr_m;

    int cur_frame_v;
    int cur_frame_a;
    int finalframe_video;
    int finalframe_audio;
    int frame_cnt_v;
    int frame_cnt_a;
    gsize payload_v;
    gsize payload_a;
    gsize interest_s;
    gsize interest_r;
    gsize interest_retx;
    gsize interest_expr;
    int frame_rate_v;
    int frame_rate_a;

    std::vector<DataNode_G> buffers_v;
    std::vector<DataNode_G> buffers_a;
  };

} // namespace ndn
#endif
