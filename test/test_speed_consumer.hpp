/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Regents of the University of California.  *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

#ifndef TEST_SPEED_CONSUMER_HPP
#define TEST_SPEED_CONSUMER_HPP
#include "consumer-callback.hpp"
#include <pthread.h>
#include <ndn-cxx/security/validator.hpp>

namespace ndn{

#define IDENTITY_NAME "/Lijing/Wang"
   time_t time_start;
  ConsumerCallback cb_consumer; 
    static void sig_int(int num)
  {
    time_t time_end = std::time(0);
    double seconds = difftime(time_end, time_start);
    std::cout << "  " << std::endl;
    std::cout << "｡:.ﾟヽ(*´∀`)ﾉﾟ. Test Result ヽ(✿ﾟ▽ﾟ)ノ  " << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "-------- Throughput --------  " << std::endl;
    std::cout << "Video Bytes: " << std::dec << cb_consumer.payload_v << "  PayLoad_Throughput: " << cb_consumer.payload_v/seconds << " Bytes/Seconds" <<std::endl;
    std::cout << "Audio Bytes: " << cb_consumer.payload_a << "  PayLoad_Throughput: " << cb_consumer.payload_a/seconds << " Bytes/Seconds" <<std::endl;
    std::cout << "Total Bytes: " << cb_consumer.payload_v + cb_consumer.payload_a << "  PayLoad_Throughput: " << (cb_consumer.payload_v + cb_consumer.payload_a)/seconds << " Bytes/Seconds" << std::endl;

    std::cout << "Video Frames: " << std::dec << cb_consumer.frame_cnt_v << "  Frame_Throughput: " << cb_consumer.frame_cnt_v/seconds << " /Seconds" <<std::endl;
    std::cout << "Audio Frames: " << std::dec << cb_consumer.frame_cnt_a << "  Frame_Throughput: " << cb_consumer.frame_cnt_a/seconds << " /Seconds" <<std::endl;
    std::cout << "Total Frames: " << cb_consumer.frame_cnt_v + cb_consumer.frame_cnt_a << "  Frame_Throughput: " << (cb_consumer.frame_cnt_v + cb_consumer.frame_cnt_a)/seconds << " /Seconds" << std::endl;

    std::cout << "  " << std::endl;
    std::cout << "-------- Interest --------  " << std::endl;
    std::cout << "Interest_Send: " << cb_consumer.interest_s << "  Interest_Send_Speed: " <<  cb_consumer.interest_s/seconds << " /Seconds" <<  std::endl;
    std::cout << "Interest_Receive: " << cb_consumer.interest_r << "  Interest_Receive_Speed: " <<  cb_consumer.interest_r /seconds << " /Seconds" <<  std::endl;
    std::cout << "Interest_Retransmit: " << cb_consumer.interest_retx << " Interest_Retransfer_Speed: " << cb_consumer.interest_retx/seconds << " /Seconds" << std::endl;
    std::cout << "Interest_Expire: " << cb_consumer.interest_expr << " Interest_Retransfer_Speed: " << cb_consumer.interest_expr/seconds << " /Seconds" << std::endl;
    std::cout << "Interest_Useful Percentage: " << double(cb_consumer.interest_r)/double(cb_consumer.interest_s)*100 <<"%" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "---------- OVER ----------  " << seconds <<" seconds" << std::endl;

    return;
  }


  class Verificator
  {
  public:
    Verificator()
    {
      Name identity(IDENTITY_NAME);
      Name keyName = m_keyChain.getDefaultKeyNameForIdentity(identity);
      m_publicKey = m_keyChain.getPublicKey(keyName); 
    };
    
    bool
    onPacket(Data& data)
    {
    //  return true;
      if (Validator::verifySignature(data, *m_publicKey))
      {
        std::cout << "Verified Name:" << data.getName() <<  std::endl;
        return true;
      }
      else
      {
        std::cout << "NOT Verified" << data.getName() << std::endl;
        return false;
      }
    }
    
  private:
    KeyChain m_keyChain;
    shared_ptr<PublicKey> m_publicKey;
  };

  struct Consumer_Need
  {
    Name prefix;
    std::string name;
    ConsumerCallback *cb;
  };

  static void
  *consume_thread (void * threadData)
  {
    Verificator* verificator = new Verificator();
    Consumer_Need *con;
    con = (Consumer_Need *) threadData;
    int end;
    int sleeptime;

    Name sampleName = con->prefix;
    sampleName.append(con->name + "/" + "content");

    Consumer* sampleConsumer = new Consumer(sampleName, RDR);

    if(con->name == "video")
    {
//      sampleConsumer->setContextOption(EMBEDDED_MANIFESTS, true);
//      sampleConsumer->setContextOption(CONTENT_RETRIEVAL_SIZE, 1024*1024);
//      sampleConsumer->setContextOption(MIN_WINDOW_SIZE, 8);
      sampleConsumer->setContextOption(CONTENT_RETRIEVED, 
                          (ConsumerContentCallback)bind(&ConsumerCallback::processTestPayload, con->cb, _1, _2, _3));

      ndn::shared_ptr<Face> f1;
      sampleConsumer->getContextOption(FACE, f1);
//      std::cout << " Video Face = " << f1 << std::endl;

//      end = con->cb->finalframe_video;
      end = 16062;
      sleeptime = 10;
    }else
    {
      sampleConsumer->setContextOption(CONTENT_RETRIEVED, 
                          (ConsumerContentCallback)bind(&ConsumerCallback::processTestPayloadAudio, con->cb, _1, _2, _3));
      ndn::shared_ptr<Face> f2;
      sampleConsumer->getContextOption(FACE, f2);
//      std::cout << " Audio Face = " << f2 << std::endl;
      sleeptime = 10;
//      end = con->cb->finalframe_audio;
      end = 13838;
    }
        
    sampleConsumer->setContextOption(MUST_BE_FRESH_S, true);
    sampleConsumer->setContextOption(INTEREST_LIFETIME, 500);

    sampleConsumer->setContextOption(INTEREST_RETX,16); //Retransmitted Attempted Time.
   // there is no need for other callback now
//    sampleConsumer->setContextOption(DATA_TO_VERIFY,
//                    (DataVerificationCallback)bind(&Verificator::onPacket, verificator, _1));
//    sampleConsumer->setContextOption(MIN_WINDOW_SIZE, 1);
//    sampleConsumer->setContextOption(MAX_WINDOW_SIZE, 10);
    sampleConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingInterest, con->cb, _1, _2));
    sampleConsumer->setContextOption(INTEREST_RETRANSMIT, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx, con->cb, _1, _2));
    sampleConsumer->setContextOption(INTEREST_EXPIRED, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr, con->cb, _1, _2));
    sampleConsumer->setContextOption(DATA_ENTER_CNTX, 
                              (ConsumerDataCallback)bind(&ConsumerCallback::processData, con->cb, _1, _2));

    
    std::cout << "HERE!" << std::endl;
    for (int i=0; i<end; i++)
    { 
      Name sampleSuffix(std::to_string(i));
//      usleep(sleeptime); 
      sampleConsumer->setContextOption(MIN_WINDOW_SIZE, 10);
      sampleConsumer->setContextOption(MAX_WINDOW_SIZE, 20);
      sampleConsumer->consume(sampleSuffix);
//      if (con->name == "video") {
//        std::cout << "video frame number " << std::dec << i << std::endl;
//        con->cb->cur_frame_v = i;
//        boost::unique_lock<boost::mutex> lock(con->cb->mut_payload_v);
//        while(!con->cb->data_ready_payload_v)
//        {
//          std::cout << "I'm HERE! Video Start waiting" << std::endl;
//          con->cb->cond_payload_v.wait(lock);
//          std::cout << "I'm HERE! Video Over waiting" << std::endl;
//        }
//        con->cb->data_ready_payload_v = false;
//      } else {
//        std::cout << "audio frame number " << std::dec << i  << std::endl;
//        con->cb->cur_frame_a = i;
//        boost::unique_lock<boost::mutex> lock(con->cb->mut_payload_a);
//        while(!con->cb->data_ready_payload_a)
//        {
//          std::cout << "I'm HERE! Audio Start waiting" << std::endl;
//          con->cb->cond_payload_a.wait(lock);
//          std::cout << "I'm HERE! Audio Over waiting" << std::endl;
//        }
//        con->cb->data_ready_payload_a = false;
//      }
//      if(i % 500 == 0)
//      {
//        std::cout << "Time Reached!" << i << std::endl;
//        sleep(sleeptime);
//      }
    }
    sig_int(0);
    pthread_exit(NULL);
  }
}
#endif
