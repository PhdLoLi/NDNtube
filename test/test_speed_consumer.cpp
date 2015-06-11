/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2015 Regents of the University of California.
 *
 * @author Lijing Wang phdloli@ucla.edu
 */

#include "test_speed_consumer.hpp"
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions


  
  int
  main(int argc, char** argv)
  {
    try {
  

      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini("config.ini", pt);

  		std::string prefix = pt.get<std::string>("ndntube.prefix");
      std::string filename ="";

      Name listName(Name(prefix).append("playlist"));
      Consumer* listConsumer = new Consumer(listName, SDR);
      listConsumer->setContextOption(INTEREST_LIFETIME, 50);
      listConsumer->setContextOption(MUST_BE_FRESH_S, true);
      listConsumer->setContextOption(RIGHTMOST_CHILD_S, true);
      listConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
        (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingInterest, &cb_consumer, _1, _2));
      listConsumer->setContextOption(CONTENT_RETRIEVED, 
        (ConsumerContentCallback)bind(&ConsumerCallback::processList, &cb_consumer, _1, _2, _3));
      listConsumer->setContextOption(DATA_ENTER_CNTX, 
        (ConsumerDataCallback)bind(&ConsumerCallback::processData, &cb_consumer, _1, _2));
  		if(argc < 2)
      {
        listConsumer->consume(Name("all"));
        boost::unique_lock<boost::mutex> lock(cb_consumer.mut);
        while(!cb_consumer.data_ready)
        {
            cb_consumer.cond.wait(lock);
        }
  
        int numberInput = 0;
        std::cout << "Please Input the Video Number " <<std::endl;
        std::cin >> numberInput;
        while(std::cin.fail() || numberInput >= cb_consumer.list.size())
        {
          if(std::cin.fail())
            std::cout << "Please Input a NUMBER "  <<std::endl;
          else
            std::cout << "Please Input a number <= " << cb_consumer.list.size() <<std::endl;
          std::cin >> numberInput;
        }
  
        filename = cb_consumer.list[numberInput];
  
      }else
        filename = argv[1];

      signal(SIGINT, sig_int);

      std::cout << "Asking For " << filename << std::endl;
      std::cout << "Only for speed test" <<std::endl;


      int rc_audio;
      Consumer_Need audioData;
      audioData.prefix = Name(prefix).append(filename);
      audioData.name = "audio";
      audioData.cb = &cb_consumer ;
      pthread_t thread_audio; 

      time_start = std::time(0);

      rc_audio = pthread_create(&thread_audio, NULL, consume_thread , (void *)&audioData);

      int rc_video;
      Consumer_Need videoData;
      videoData.prefix = Name(prefix).append(filename);
      videoData.name = "video";
      videoData.cb = &cb_consumer ;
      pthread_t thread_video; 
      rc_video = pthread_create(&thread_video, NULL, consume_thread , (void *)&videoData);
      std::cout << "Thread over start waiting" << std::endl;

//      cb_consumer.player.consume_whole(videoConsumer, audioConsumer);
//      time_t time_end_0  = std::time(0);
//      std::cout << "After consume " << time_end_0 << std::endl;
//      double seconds = difftime(time_end_0, time_start_0);
//      std::cout << seconds << " seconds have passed" << std::endl;

      sleep(3000); // because consume() is non-blocking
      
    }
    catch(std::exception& e) {
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
