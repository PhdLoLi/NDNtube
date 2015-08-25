/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang wanglj11@mails.tsinghua.edu.cn 
 */

#include "video-player.hpp"

namespace ndn {

  int t_video = 0;
  int t_audio = 0;


  VideoPlayer::VideoPlayer() : rate_ready(false)//, pipeline_pool(1)
  {
  }

  VideoPlayer::~VideoPlayer() 
  {
    pipe_thread->join();
  }
  int 
  VideoPlayer::get_video_rate()
  {
    VideoAudio *va = &s_va;
    App *video = &(va->v_app);
    return video->rate;
  }

  int 
  VideoPlayer::get_audio_rate()
  {
    VideoAudio *va = &s_va;
    App *audio = &(va->a_app);
    return audio->rate;
  }

  void
  VideoPlayer::get_streaminfo(std::string streaminfo)
  {
    VideoAudio *va =  &s_va;
    App *app = &(va->v_app);
    app->capstr = streaminfo; 
//    pthread_mutex_init(&(app->count_mutex), NULL);
//    pthread_cond_init (&(app->count_cond), NULL);
//    std::cout << "Video streaminfo " << streaminfo << std::endl;
//    pthread_t thread; 
//    int rc;
//    rc = pthread_create(&thread, NULL, h264_video_thread ,(void *)app);
//    std::cout << "h264_video_init OK! " << std::endl;
  }

  void
  VideoPlayer::get_streaminfo_audio(std::string streaminfo)
  {
    VideoAudio *va =  &s_va;
    App *app = &(va->a_app);
    app->capstr = streaminfo; 
//    pthread_mutex_init(&(app->count_mutex), NULL);
//    pthread_cond_init (&(app->count_cond), NULL);
//    std::cout << "Audio streaminfo " << streaminfo << std::endl;
//    pthread_t thread; 
//    int rc;
//    rc = pthread_create(&thread, NULL, h264_audio_thread ,(void *)app);
//    std::cout << "h264_audio_thread OK! " << std::endl;
    h264_appsrc_init();
//    std::cout << "get streaminfo audio over" << std::endl;
  }

  void
  VideoPlayer::h264_appsrc_data(const uint8_t* buffer, size_t bufferSize )
  {

    if (buffer != NULL && bufferSize > 0) {
      VideoAudio *va = &s_va;
      App *app = &(va->v_app);
      uint8_t* bufferTmp = new uint8_t[bufferSize];
      memcpy (bufferTmp, buffer, bufferSize);

      app->queue_m.lock();
      app->dataQue.push_back(DataNode(bufferSize, (guint8 *)bufferTmp));
      std::cout << "videoQueueSize " << (app->dataQue).size() <<std::endl;
      app->queue_m.unlock();

      t_video ++;
    } else {
      printf("h264_appsrc_data t_video:%d Terrible Happen\n", t_video);
    }
  }

  void
  VideoPlayer::h264_appsrc_data_audio(const uint8_t* buffer, size_t bufferSize )
  {

    if (buffer != NULL && bufferSize > 0) {
      VideoAudio *va = &s_va;
      App *app = &(va->a_app);
      uint8_t* bufferTmp = new uint8_t[bufferSize];
      memcpy (bufferTmp, buffer, bufferSize);

      app->queue_m.lock();
      app->dataQue.push_back(DataNode(bufferSize, (guint8 *)bufferTmp));
      std::cout << "audioQueueSize " << (app->dataQue).size() <<std::endl;
      app->queue_m.unlock();

      t_audio ++;
    } else {
      printf("h264_appsrc_data t_audio:%d Terrible Happen\n", t_audio);
    }
  }

/*
 * First INIT the h264_appsrc using thread
 * waiting in a loop
 */
  void
  VideoPlayer::h264_appsrc_init()
  {
    VideoAudio *va = &s_va;
//    pipeline_pool.schedule(boost::bind(&VideoPlayer::h264_appsrc_thread ,this, va));
    pipe_thread = new boost::thread(boost::bind(&VideoPlayer::h264_appsrc_thread ,this, va));
 
//    pthread_t thread; 
//    int rc;
//    rc = pthread_create(&thread, NULL, h264_appsrc_thread ,(void *)va);
//    std::cout << "Pipeline Initialisation DONE! " << std::endl;
  }
/* Call Consume Here From the start*/
  void
  VideoPlayer::consume_whole(Consumer *videoConsumer, Consumer *audioConsumer)
  {
    VideoAudio *va = &s_va;
    App *video = &(va->v_app);
    App *audio = &(va->a_app);
    std::cout << "Inside the consume_whole! " <<std::endl;
    int audio_time = audio->rate;
    int video_time = video->rate;
    for(int seconds = 0; seconds < 12 ; seconds++)
    {
      for(int j=0; j< audio_time; j++)
      {
        std::cout << "Consume Audio" << std::endl;
        Name audioSuffix(std::to_string(seconds*audio_time + j));
        audioConsumer->consume(audioSuffix);
//        usleep(100);
      }
      for(int i=0; i< video_time; i++)
      {
        std::cout << "Consume Video" << std::endl;
        Name videoSuffix(std::to_string(seconds*video_time + i));
        videoConsumer->consume(videoSuffix);
//        usleep(8000);
      }

    }
  }
} // namespace ndn
