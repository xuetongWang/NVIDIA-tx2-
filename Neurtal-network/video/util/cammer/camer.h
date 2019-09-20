#pragma once

#include <gst/gst.h>
#include <string>
#include <gst/app/gstappsink.h>
#include<iostream>
#include <memory>
#include <mutex>
#include <condition_variable>

class camera
{
public:
    static std::shared_ptr<camera> create(uint32_t h, uint32_t w);

     camera();
    ~camera();

    bool open();
    void close();

    bool capture(void ** cpu, void ** cuda);

    bool convertrRGB(void * input, void ** output);

    inline uint32_t getWidth() const	  { return mWidth; }
    inline uint32_t getHeight() const	  { return mHeight; }


private:
    static void onEOS(_GstAppSink* sink, void* user_data);
    static GstFlowReturn onPreroll(_GstAppSink* sink, void* user_data);
    static GstFlowReturn onBuffer(_GstAppSink* sink, void* user_data);

    bool init();
    bool buildLaunchStr();
    void checkMsgBus();
    void checkBuffer();

    _GstBus*     mBus;
    _GstAppSink* mAppSink;
    _GstElement* mPipeline;

    std::string  mLaunchStr;

    uint32_t mWidth;
    uint32_t mHeight;

    static const uint32_t NUM_RINGBUFFERS = 16;

    void* mRingbufferCPU[NUM_RINGBUFFERS];
    void* mRingbufferGPU[NUM_RINGBUFFERS];

    std::unique_ptr<std::condition_variable> mWaitEvent;

    std::unique_ptr<std::mutex> mWaitMutex;

    uint32_t mLatestRGB;
    uint32_t mLatestRingbuffer;

    static bool mLatestRetrieved;

    void* mRGB[NUM_RINGBUFFERS];

};
