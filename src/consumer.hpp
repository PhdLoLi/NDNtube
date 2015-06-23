/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Regents of the University of California.  *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

#ifndef CONSUMER_HPP
#define CONSUMER_HPP
#include "consumer-callback.hpp"
#include <ndn-cxx/security/validator.hpp>
#include "threadpool.hpp"

using namespace boost::threadpool;

namespace ndn{

#define IDENTITY_NAME "/Lijing/Wang"
  
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

//  struct Consumer_Need
//  {
//    Name prefix;
//    std::string name;
//    ConsumerCallback *cb;
//  };
//
//  static void
//  *consume_thread (void * threadData)
//  {
//    Verificator* verificator = new Verificator();
//    Consumer_Need *con;
//    con = (Consumer_Need *) threadData;
//    int end;
////    int sleeptime;
//
//    Name sampleName = con->prefix;
//    sampleName.append(con->name + "/" + "content");
//
//    Consumer* sampleConsumer = new Consumer(sampleName, RDR);
//
//    if(con->name == "video")
//    {
//
//      sampleConsumer->setContextOption(CONTENT_RETRIEVED, 
//                          (ConsumerContentCallback)bind(&ConsumerCallback::processPayload, con->cb, _1, _2, _3));
//
//      end = con->cb->finalframe_video;
//    }else
//    {
//      sampleConsumer->setContextOption(CONTENT_RETRIEVED, 
//                          (ConsumerContentCallback)bind(&ConsumerCallback::processPayloadAudio, con->cb, _1, _2, _3));
//      end = con->cb->finalframe_audio;
//    }
//        
//    sampleConsumer->setContextOption(MUST_BE_FRESH_S, true);
//    sampleConsumer->setContextOption(INTEREST_LIFETIME, 2000);
//
//    sampleConsumer->setContextOption(INTEREST_RETX,16); //Retransmitted Attempted Time.
//
//    sampleConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
//                              (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingInterest, con->cb, _1, _2));
//    sampleConsumer->setContextOption(INTEREST_RETRANSMIT, 
//                              (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx, con->cb, _1, _2));
//    sampleConsumer->setContextOption(INTEREST_EXPIRED, 
//                              (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr, con->cb, _1, _2));
//    sampleConsumer->setContextOption(DATA_ENTER_CNTX, 
//                              (ConsumerDataCallback)bind(&ConsumerCallback::processData, con->cb, _1, _2));
//
//    for (int i=0; i<end; i++)
//    { 
//      Name sampleSuffix(std::to_string(i));
//      sampleConsumer->consume(sampleSuffix);
//    }
//  }
}
#endif
