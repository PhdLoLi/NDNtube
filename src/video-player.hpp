/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

#ifndef VIDEO_PLAYER_HPP
#define VIDEO_PLAYER_HPP

#include <fstream>
#include <gst/gst.h>
#include <math.h>
#include <iostream>
#include <deque>
#include <pthread.h>
#include <unistd.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <ndn-cxx/contexts/consumer-context.hpp>

namespace ndn {

  class VideoPlayer
  {
    public:

      VideoPlayer();
      void
      h264_appsrc_init ();
      void
      h264_appsrc_data (const uint8_t *buffer, size_t bufferSize);
      void 
      get_streaminfo(std::string streaminfo);
      void
      h264_appsrc_data_audio (const uint8_t *buffer, size_t bufferSize);
      void 
      get_streaminfo_audio(std::string streaminfo);
      void
      consume_whole(Consumer *frameConsumer, Consumer *sampleConsumer);

    private:

      struct DataNode
      {
        gsize length;
        guint8 *data;
      };

      struct App
      {
        GstElement *appsrc;
        guint sourceid;

        std::deque<DataNode> dataQue;
        std::string capstr;
        int rate;

        GstElement *decoder;
        GstElement *sink;
        GstElement *queue;

//        pthread_mutex_t count_mutex;
//        pthread_cond_t count_cond;
      };

      struct VideoAudio
      {
        App v_app;
        App a_app;
      };

      VideoAudio s_va;

  /* PUSH MODE USED */
      static gboolean
      read_data (App * app)
      {
        GstFlowReturn ret;
//        pthread_mutex_lock(&(app->count_mutex));
//        while((app -> dataQue).size() == 0 )
//        {
//           pthread_cond_wait(&(app->count_cond), &(app->count_mutex));
//        }
//        pthread_mutex_unlock(&(app->count_mutex));

        if( (app -> dataQue).size() == 0  )
        {
////          usleep(1000);
////          if(app->rate == 44)
////            std::cout<< "audioQueue EMPTY!!" << std::endl;
          return TRUE; 
        }
        
//        std::cout << "Readrate !" << app->rate << std::endl;
        DataNode tmpNode = (app -> dataQue).front();
        GstBuffer *buffer;
        
//        buffer = gst_buffer_new_allocate (NULL, tmpNode.length, NULL);


//        uint8_t* bufferTmp = new uint8_t[tmpNode.length];
//        memcpy (bufferTmp, tmpNode.data, tmpNode.length);

        buffer = gst_buffer_new();
        buffer = gst_buffer_new_wrapped(tmpNode.data, tmpNode.length);
        g_signal_emit_by_name(app->appsrc, "push-buffer", buffer, &ret);
//  gst_buffer_unmap (buffer, &info);
        gst_buffer_unref(buffer);
        (app -> dataQue).pop_front();

        if (ret != GST_FLOW_OK) {
          /* some error, stop sending data */
          std::cout << "FALSE!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
          return FALSE;
        }
        return TRUE;
      }
      
      /* This signal callback is called when appsrc needs data, we add an idle handler
       * to the mainloop to start pushing data into the appsrc */
      static void
      start_feed (GstElement * pipeline, guint size, App * app)
      {
        if (app->sourceid == 0) {
          app->sourceid = g_idle_add ((GSourceFunc) read_data, app);
        }
      }
      
      /* This callback is called when appsrc has enough data and we can stop sending.
       * We remove the idle handler from the mainloop */
      static void
      stop_feed (GstElement * pipeline, App * app)
      {
        if (app->sourceid != 0) {
          g_source_remove (app->sourceid);
          app->sourceid = 0;
        }
      }
      
      /* 
       * This works well for playing back MP4 Video with Audio 
       * Lijig Wang
       * 2014/12/04 
       */
      static void
      *h264_appsrc_thread (void * threadData)
      {
        VideoAudio * va;
        va = (VideoAudio *) threadData;

        GstBus *bus;
        GMainLoop *loop;
        GstElement *pipeline, *mqueue;
        App *video = &(va -> v_app);
        App *audio = &(va -> a_app);
        
        /* init GStreamer */
        gst_init (NULL, NULL);
        loop = g_main_loop_new (NULL, FALSE);
        /* setup pipeline */
        pipeline = gst_pipeline_new ("pipeline");

        mqueue = gst_element_factory_make("multiqueue", "multiqueue");
//        g_object_set (G_OBJECT (mqueue), "max-size-time", 20000000000, NULL);
//        g_object_set (G_OBJECT (mqueue), "max-size-buffers", 0, NULL);
//        g_object_set (G_OBJECT (mqueue), "max-size-bytes", 1000000, NULL);
//        g_object_set (G_OBJECT (mqueue), "use-buffering", 1, NULL);
//        g_object_set (G_OBJECT (mqueue), "sync-by-running-time", 1, NULL);


/**********   Video Part **************/
        video->appsrc = gst_element_factory_make ("appsrc", "video_source");
        video->queue = gst_element_factory_make("queue", "video_queue");
        video->decoder = gst_element_factory_make ("avdec_h264", "video_decoder");
        video->sink = gst_element_factory_make ("autovideosink", "video_sink");
        const gchar * streaminfo = video->capstr.c_str();
        /* setup streaminfo */
        GstCaps *caps = gst_caps_from_string(streaminfo); 
//        gint width, height, num, denum;
//        const char * name, stream-format, alignment;
//        caps = gst_caps_make_writable(caps);
        GstStructure *str = gst_caps_get_structure (caps, 0);
        int num, denom;
        gst_structure_get_fraction (str, "framerate", &num, &denom);
//        gst_structure_remove_fields (str,"level", "profile", "height", "width", "framerate", "pixel-aspect-ratio", "parsed", NULL);
        video->rate = ceil(num/double(denom)); //FIX ME
//        std::cout << "video->rate " << video->rate << std::endl; 

        g_object_set (G_OBJECT (video->appsrc), "caps", caps, NULL);
        gst_bin_add_many (GST_BIN (pipeline), video->appsrc, mqueue, video->decoder, video->sink, NULL);
        g_assert(gst_pad_link(gst_element_get_static_pad(video->appsrc, "src"), gst_element_get_request_pad(mqueue,"sink_0")) == GST_PAD_LINK_OK);
        g_assert(gst_pad_link(gst_element_get_static_pad(mqueue,"src_0"),gst_element_get_static_pad(video->decoder, "sink")) == GST_PAD_LINK_OK); 
        gst_element_link_many (video->decoder, video->sink, NULL);
        /* setup appsrc */
        g_signal_connect (video->appsrc, "need-data", G_CALLBACK (start_feed), video);
        g_signal_connect (video->appsrc, "enough-data", G_CALLBACK (stop_feed), video);
        g_object_set(G_OBJECT(video->appsrc), "max-bytes", 0, NULL);
//        gst_app_src_set_max_bytes((GstAppSrc *)video->appsrc, 10000);
/**********   Video Part Over**************/

/******* Audio Part *********************/
        audio->appsrc = gst_element_factory_make ("appsrc", "audio_source");
        audio->queue = gst_element_factory_make("queue", "audio_queue");
        audio->decoder = gst_element_factory_make ("faad", "audio_decoder"); 
        audio->sink = gst_element_factory_make ("autoaudiosink", "audio_sink");
        const gchar * streaminfo_audio = audio->capstr.c_str();
        /* setup streaminfo */
        GstCaps *caps_audio =  gst_caps_from_string(streaminfo_audio); 
        GstStructure *str_audio = gst_caps_get_structure (caps_audio, 0);
        int samplerate;
        gst_structure_get_int (str_audio, "rate", &samplerate);
        audio->rate = samplerate/1000; //FIX ME
//        std::cout << "audio->rate " << audio->rate << std::endl; 
      
        g_object_set (G_OBJECT (audio->appsrc), "caps", caps_audio, NULL);
        gst_bin_add_many (GST_BIN (pipeline), audio->appsrc, audio->decoder, audio->sink, NULL);
        g_assert(gst_pad_link(gst_element_get_static_pad(audio->appsrc, "src"), gst_element_get_request_pad(mqueue,"sink_1")) == GST_PAD_LINK_OK);
        g_assert(gst_pad_link(gst_element_get_static_pad(mqueue,"src_1"),gst_element_get_static_pad(audio->decoder, "sink")) == GST_PAD_LINK_OK); 
        gst_element_link_many (audio->decoder, audio->sink, NULL);
        /* setup appsrc */
        g_signal_connect (audio->appsrc, "need-data", G_CALLBACK (start_feed), audio);
        g_signal_connect (audio->appsrc, "enough-data", G_CALLBACK (stop_feed), audio);
        g_object_set(G_OBJECT(audio->appsrc), "max-bytes", 0, NULL);
/*********Audio Part Over **************/        


        /* Set bus */
        bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
        gst_bus_add_watch (bus, (GstBusFunc)bus_call, pipeline);
        gst_object_unref (bus);
        /* play */
        gst_element_set_state (pipeline, GST_STATE_PLAYING);
        std::cout << "start running" <<std::endl;

        g_main_loop_run (loop);
        
        std::cout << "Never here" <<std::endl;
        /* clean up */
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
        g_main_loop_unref (loop);

        pthread_exit(NULL);
      }
      
      static void
      *h264_audio_thread (void * threadData)
      {
        GstBus *bus;
        GMainLoop *loop;
        GstElement *pipeline;
        App *audio = (App *) threadData;
        
        /* init GStreamer */
        gst_init (NULL, NULL);
        loop = g_main_loop_new (NULL, FALSE);
        /* setup pipeline */
        pipeline = gst_pipeline_new ("pipeline");

/******* Audio Part *********************/
        audio->appsrc = gst_element_factory_make ("appsrc", "audio_source");
        audio->queue = gst_element_factory_make("queue", "audio_queue");
        audio->decoder = gst_element_factory_make ("faad", "audio_decoder"); 
        audio->sink = gst_element_factory_make ("autoaudiosink", "audio_sink");
        const gchar * streaminfo_audio = audio->capstr.c_str();
        /* setup streaminfo */
        GstCaps *caps_audio =  gst_caps_from_string(streaminfo_audio); 
        GstStructure *str_audio = gst_caps_get_structure (caps_audio, 0);
        int samplerate;
        gst_structure_get_int (str_audio, "rate", &samplerate);
        audio->rate = samplerate/1000; //FIX ME
        std::cout << "audio->rate " << audio->rate << std::endl; 
      
        g_object_set (G_OBJECT (audio->appsrc), "caps", caps_audio, NULL);
        gst_bin_add_many (GST_BIN (pipeline), audio->appsrc, audio->queue, audio->decoder, audio->sink, NULL);
        gst_element_link_many (audio->appsrc, audio->queue, audio->decoder, audio->sink, NULL);
        /* setup appsrc */
        g_signal_connect (audio->appsrc, "need-data", G_CALLBACK (start_feed), audio);
        g_signal_connect (audio->appsrc, "enough-data", G_CALLBACK (stop_feed), audio);
/*********Audio Part Over **************/        

        /* Set bus */
        bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
        gst_bus_add_watch (bus, (GstBusFunc)bus_call, pipeline);
        gst_object_unref (bus);
        /* play */
        gst_element_set_state (pipeline, GST_STATE_PLAYING);
        g_main_loop_run (loop);
        
        /* clean up */
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
        g_main_loop_unref (loop);

        pthread_exit(NULL);
      }

/* Only playback video */
      static void
      *h264_video_thread (void * threadData)
      {
        GstBus *bus;
        GMainLoop *loop;
        GstElement *pipeline;
        App *video = (App *) threadData;
        
        /* init GStreamer */
        gst_init (NULL, NULL);
        loop = g_main_loop_new (NULL, FALSE);
        /* setup pipeline */
        pipeline = gst_pipeline_new ("pipeline");
/**********   Video Part **************/
        video->appsrc = gst_element_factory_make ("appsrc", "video_source");
        video->queue = gst_element_factory_make("queue", "video_queue");
        video->decoder = gst_element_factory_make ("avdec_h264", "video_decoder");
        video->sink = gst_element_factory_make ("autovideosink", "video_sink");
        const gchar * streaminfo = video->capstr.c_str();
        /* setup streaminfo */
        GstCaps *caps = gst_caps_from_string(streaminfo); 
        GstStructure *str = gst_caps_get_structure (caps, 0);
        int num, denom;
        gst_structure_get_fraction (str, "framerate", &num, &denom);
        video->rate = ceil(num/double(denom)); //FIX ME
        std::cout << "video->rate " << video->rate << std::endl; 

        g_object_set (G_OBJECT (video->appsrc), "caps", caps, NULL);
        gst_bin_add_many (GST_BIN (pipeline), video->appsrc, video->queue, video->decoder, video->sink, NULL);
        gst_element_link_many (video->appsrc, video->queue, video->decoder, video->sink, NULL);
        /* setup appsrc */
        g_signal_connect (video->appsrc, "need-data", G_CALLBACK (start_feed), video);
        g_signal_connect (video->appsrc, "enough-data", G_CALLBACK (stop_feed), video);
/**********   Video Part Over**************/

        /* Set bus */
        bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
        gst_bus_add_watch (bus, (GstBusFunc)bus_call, pipeline);
        gst_object_unref (bus);
        /* play */
        gst_element_set_state (pipeline, GST_STATE_PLAYING);
        g_main_loop_run (loop);
        
        /* clean up */
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
        g_main_loop_unref (loop);

        pthread_exit(NULL);
      }

      static gboolean
      bus_call (GstBus * bus, GstMessage *msg, GstElement *pipeline)
      {
        switch (GST_MESSAGE_TYPE (msg)) {
          case GST_MESSAGE_BUFFERING: {
            gint percent = 0;
            gst_message_parse_buffering (msg, &percent);
            g_print ("Buffering (%3d%%)\r", percent);
            /* Wait until buffering is complete before start/resume playing */
            if (percent < 100)
              gst_element_set_state (pipeline, GST_STATE_PAUSED);
            else
              gst_element_set_state (pipeline, GST_STATE_PLAYING);
            break;
          }
          case GST_MESSAGE_EOS:{
            g_print ("End-of-stream\n");
//            g_main_loop_quit (app->loop);
            break;
          }
          case GST_MESSAGE_ERROR:{
            gchar *debug;
            GError *err;
            gst_message_parse_error (msg, &err, &debug);
            g_printerr ("Debugging info: %s\n", (debug) ? debug : "none");
            g_free (debug);
            g_print ("Error: %s\n", err->message);
            g_error_free (err);
//            g_main_loop_quit (app->loop);
            break;
          }
          case GST_MESSAGE_CLOCK_LOST:{
          /* Get a new clock */
            gst_element_set_state (pipeline, GST_STATE_PAUSED);
            gst_element_set_state (pipeline, GST_STATE_PLAYING);
            break;
          }
          default:
            break;
        }
      return TRUE;
    }

  };
} // namespace ndn
#endif //VIDEO_PLAYER_HPP
