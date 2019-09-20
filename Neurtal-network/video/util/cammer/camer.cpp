
#include <memory>
#include <iostream>
#include <unistd.h>
#include "camer.h"
#include "common.h"
#include "gstutility.h"

#include "cuda/cudamemory.h"
#include "cuda/cudaUtility.h"
#include "cuda/cudargb.h"

bool camera::mLatestRetrieved  = true;
/*
 * construt camer
*/
camera::camera()
{
    mAppSink = nullptr;
    mBus = nullptr;
    mPipeline =nullptr;

    mWidth = 0;
    mHeight = 0;

    mWaitEvent = std::make_unique<std::condition_variable>();
    mWaitMutex = std::make_unique<std::mutex>();

    mLatestRGB       = 0;
    mLatestRingbuffer = 0;

    for( uint32_t n=0; n < NUM_RINGBUFFERS; n++ )
    {
        mRingbufferCPU[n] = nullptr;
        mRingbufferGPU[n] = nullptr;
        mRGB[n]          = nullptr;
    }
}

camera::~camera()
{

}
// ConvertRGB
bool camera::convertrRGB( void* input, void** output)
{
    if( !input || !output )
        return false;

    if( !mRGB[0] )
    {
        const size_t size = mWidth * mHeight * sizeof(float4);

        for( uint32_t n=0; n < NUM_RINGBUFFERS; n++ )
        {
            if( CUDA_FAILED(cudaMalloc(&mRGB[n], size)) )
            {
                printf(LOG_CUDA "gstCamera -- failed to allocate memory for %ux%u RGB texture\n", mWidth, mHeight);
                return false;
            }

        }

        printf(LOG_CUDA "gstreamer camera -- allocated %u RGB ringbuffers\n", NUM_RINGBUFFERS);
    }
    if( CUDA_FAILED(cudaNV12ToRGBA32((uint8_t*)input, (float4*)mRGB[mLatestRGB], mWidth, mHeight)) )
        return false;

    *output     = mRGB[mLatestRGB];
    mLatestRGB = (mLatestRGB + 1) & (NUM_RINGBUFFERS-1);
    return true;
}

bool camera::capture(void ** cpu, void ** cuda)
{
    std::unique_lock<std::mutex> lk(*mWaitMutex);

    mWaitEvent->wait(lk, []{return mLatestRetrieved == false;});

    const uint32_t latest = mLatestRingbuffer;
    const bool retrieved = mLatestRetrieved;
    mLatestRetrieved = true;
    if( retrieved )
        return false;

    if( cpu != NULL )
        *cpu = mRingbufferCPU[latest];

    if( cuda != NULL )
        *cuda = mRingbufferGPU[latest];

    return true;
}

void camera::checkBuffer()
{
    if( !mAppSink )
        return;
    GstSample* gstSample = gst_app_sink_pull_sample(mAppSink);

    if( !gstSample )
    {
        printf(LOG_GSTRM "gstreamer camera -- gst_app_sink_pull_sample() returned NULL...\n");
        return;
    }

    GstBuffer* gstBuffer = gst_sample_get_buffer(gstSample);

    if( !gstBuffer )
    {
        printf(LOG_GSTRM "gstreamer camera -- gst_sample_get_buffer() returned NULL...\n");
        return;
    }

    // retrieve
    GstMapInfo map;

    if(	!gst_buffer_map(gstBuffer, &map, GST_MAP_READ) )
    {
        printf(LOG_GSTRM "gstreamer camera -- gst_buffer_map() failed...\n");
        return;
    }

    void* gstData = map.data; //GST_BUFFER_DATA(gstBuffer);
    const uint32_t gstSize = map.size; //GST_BUFFER_SIZE(gstBuffer);

    if( !gstData )
    {
        printf(LOG_GSTRM "gstreamer camera -- gst_buffer had NULL data pointer...\n");
        gst_sample_unref(gstSample);
        return;
    }

    // retrieve caps
    GstCaps* gstCaps = gst_sample_get_caps(gstSample);

    if( !gstCaps )
    {
        printf(LOG_GSTRM "gstreamer camera -- gst_buffer had NULL caps...\n");
        gst_sample_unref(gstSample);
        return;
    }

    GstStructure* gstCapsStruct = gst_caps_get_structure(gstCaps, 0);

    if( !gstCapsStruct )
    {
        printf(LOG_GSTRM "gstreamer camera -- gst_caps had NULL structure...\n");
        gst_sample_unref(gstSample);
        return;
    }

    // get width & height of the buffer
    int width  = 0;
    int height = 0;

    if( !gst_structure_get_int(gstCapsStruct, "width", &width) ||
        !gst_structure_get_int(gstCapsStruct, "height", &height) )
    {
        printf(LOG_GSTRM "gstreamer camera -- gst_caps missing width/height...\n");
        gst_sample_unref(gstSample);
        return;
    }

    if( width < 1 || height < 1 ){
        gst_sample_unref(gstSample);
        return;
    }

    mWidth  = width;
    mHeight = height;

    if( !mRingbufferCPU[0] )
    {
        for( uint32_t n=0; n < NUM_RINGBUFFERS; n++ )
        {
            if( !cudaAllocMapped(&mRingbufferCPU[n], &mRingbufferGPU[n], gstSize) )
                printf(LOG_CUDA "gstreamer camera -- failed to allocate ringbuffer %u  (size=%u)\n", n, gstSize);
        }

        printf(LOG_CUDA "gstreamer camera -- allocated %u ringbuffers, %u bytes each\n", NUM_RINGBUFFERS, gstSize);
    }

    // copy to next ringbuffer
    const uint32_t nextRingbuffer = (mLatestRingbuffer + 1) & (NUM_RINGBUFFERS-1);
    memcpy(mRingbufferCPU[nextRingbuffer], gstData, gstSize);
    gst_buffer_unmap(gstBuffer, &map);
    //gst_buffer_unref(gstBuffer);
    gst_sample_unref(gstSample);


    // update and signal sleeping threads
    std::lock_guard<std::mutex> lk(*mWaitMutex);
    mLatestRingbuffer = nextRingbuffer;
    mLatestRetrieved  = false;
    mWaitEvent->notify_all();
}



bool camera::buildLaunchStr()
{
    std::ostringstream ss;

    ss <<  "rtspsrc location=rtsp://admin:Sleton888@192.168.1.64/cam/realmonitor?channel=1&subtype=0 latency=0 !";
    ss << " decodebin !";
    ss << " nvvidconv !";
    ss << " video/x-raw,format=(string)NV12,width=(int)" << mWidth << ", height=(int)" << mHeight << ",fFramerate=30/1 !";
    ss << " appsink name=mysink ";

    mLaunchStr = ss.str();

    printf(LOG_GSTRM "gstreamer decoder pipeline string:\n");
    printf("%s\n", mLaunchStr.c_str());
    return true;
}

// init
bool camera::init()
{
    GError* err = nullptr;

    if( !buildLaunchStr() )
    {
        printf(LOG_GSTRM "gstreamer decoder failed to build pipeline string\n");
        return false;
    }

    mPipeline = gst_parse_launch(mLaunchStr.c_str(), &err);

    if( err != nullptr )
    {
        printf(LOG_GSTRM "gstreamer decoder failed to create pipeline\n");
        printf(LOG_GSTRM "   (%s)\n", err->message);
        g_error_free(err);
        return false;
    }

    GstPipeline* pipeline = GST_PIPELINE(mPipeline);

    if( !pipeline )
    {
        printf(LOG_GSTRM "gstreamer failed to cast GstElement into GstPipeline\n");
        return false;
    }

    // retrieve pipeline bus
    /*GstBus**/ mBus = gst_pipeline_get_bus(pipeline);

    if( !mBus )
    {
        printf(LOG_GSTRM "gstreamer failed to retrieve GstBus from pipeline\n");
        return false;
    }


    // get the appsrc
    GstElement* appsinkElement = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");
    GstAppSink* appsink = GST_APP_SINK(appsinkElement);

    if( !appsinkElement || !appsink)
    {
        printf(LOG_GSTRM "gstreamer failed to retrieve AppSink element from pipeline\n");
        return false;
    }

    mAppSink = appsink;

    // setup callbacks
    GstAppSinkCallbacks cb;
    memset(&cb, 0, sizeof(GstAppSinkCallbacks));

    cb.eos         = onEOS;
    cb.new_preroll = onPreroll;
    cb.new_sample  = onBuffer;

    gst_app_sink_set_callbacks(mAppSink, &cb, (void*)this, NULL);

    return true;
}

std::shared_ptr<camera> camera::create(uint32_t h, uint32_t w)
{
    if( !gstreamerInit() )
    {
        printf(LOG_GSTRM "failed to initialize gstreamer API\n");
        return NULL;
    }

    std::shared_ptr<camera> cam= std::make_shared<camera>();

    cam-> mWidth  = h;
    cam->mHeight = w;

    if( !cam )
        return nullptr;

    if( !cam->init() )
    {
        printf(LOG_GSTRM "failed to init gstCamera\n");
        return nullptr;
    }
    return cam;
}

void camera::checkMsgBus()
{
    while(true)
    {
        GstMessage* msg = gst_bus_pop(mBus);

        if( !msg )
            break;

  //      gst_message_print(mBus, msg, this);
        gst_message_unref(msg);
    }
}

bool camera::open()
{
    printf(LOG_GSTRM "gstermer open \n");

    const GstStateChangeReturn result = gst_element_set_state(mPipeline, GST_STATE_PLAYING);

    if( result == GST_STATE_CHANGE_ASYNC )
    {
#if 0
        GstMessage* asyncMsg = gst_bus_timed_pop_filtered(mBus, 5 * GST_SECOND,
                                  (GstMessageType)(GST_MESSAGE_ASYNC_DONE|GST_MESSAGE_ERROR));

        if( asyncMsg != NULL )
        {
            gst_message_print(mBus, asyncMsg, this);
            gst_message_unref(asyncMsg);
        }
        else
            printf(LOG_GSTREAMER "gstreamer NULL message after transitioning pipeline to PLAYING...\n");
#endif
    }
    else if( result != GST_STATE_CHANGE_SUCCESS )
    {
        printf(LOG_GSTRM "failed to set pipeline state to PLAYING (error %u)\n", result);
        return false;
    }

    checkMsgBus();
    usleep(100*1000);
    checkMsgBus();

    return true;
}

void camera::close()
{
    // stop pipeline
    printf(LOG_GSTRM "gstreamer transitioning pipeline to GST_STATE_NULL\n");

    const GstStateChangeReturn result = gst_element_set_state(mPipeline, GST_STATE_NULL);

    if( result != GST_STATE_CHANGE_SUCCESS )
        printf(LOG_GSTRM "gstreamer failed to set pipeline state to PLAYING (error %u)\n", result);

    usleep(250*1000);
}

// onEOS
void camera::onEOS(_GstAppSink* sink, void* user_data)
{
    printf(LOG_GSTREAMER "gstreamer decoder onEOS\n");
}


// onPreroll
GstFlowReturn camera::onPreroll(_GstAppSink* sink, void* user_data)
{
    return GST_FLOW_OK;
}


// onBuffer
GstFlowReturn camera::onBuffer(_GstAppSink* sink, void* user_data)
{
    if( !user_data )
        return GST_FLOW_OK;

    camera* dec = (camera*)user_data;

    dec->checkBuffer();
    dec->checkMsgBus();
    return GST_FLOW_OK;
}

