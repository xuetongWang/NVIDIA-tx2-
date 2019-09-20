#include <stdio.h>
#include <map>
#include <vector>
#include "common.h"
#include "cammer/camer.h"
#include "display/display.h"
#include "network/yolov3.h"
#include "cuda/cudargb.h"

int main(int argc, char** agrv){

 std::string init_file = "./config/yolov3.txt";
 int batchsize;
 NetworkInfo yoloInfo;
 InferParams yoloInferParams;
 void* fRm = nullptr;

 if(fRm == nullptr ){
     cudaMalloc(&fRm,3*512*512*sizeof(float4));
 }

 std::vector<std::string> config_info = common::loadListFromTextFile(init_file);


 if(!common::splitCfgfile(batchsize ,yoloInfo, yoloInferParams,config_info)){

     std::cout << "model initial failed" << std::endl;
     return 0;
 }
    std::unique_ptr<Yolo> inferNetwork = std::make_unique<YoloV3>(batchsize, yoloInfo, yoloInferParams) ;

    std::shared_ptr<camera> camer(camera::create(512, 512));

    if(!camer){
        std::cout << "camer initial failed" << std::endl;
        return 0;
    }
    if(!camer->open()){
        std::cout << "camer initial open failed" << std::endl;
        return 0;
    }

    std::shared_ptr<glDisplay> display(glDisplay::Create("Sleton | VID_", 0.05f, 0.05f, 0.05f));
    if( !display )
    {
        printf("\ngl-display:  failed to create openGL display\n");
        return 0;
    }

    glTexture* texture = glTexture::Create(512, 512, GL_RGBA32F_ARB/*GL_RGBA8*/);

    if( !texture )
        printf("gst-camera:  failed to create openGL texture\n");


    while(true){

        void* imgCPU = nullptr;
        void* imgGPU = nullptr;
        void* imgRGB = nullptr;


        if(!camer->capture(&imgCPU , &imgGPU)){
            std::cout << "camer capture failed" << std::endl;
        }

        if(!camer->convertrRGB(imgGPU , &imgRGB)){
            std::cout << "camer convert img from NVL to RGB failed" << std::endl;
        }

        CUDA(cudaNormalizeRGB((float4*)imgRGB, make_float2(0.0f, 255.0f),
                           (float4*)fRm, make_float2(0.0f, 1.0f),
                           512, 512));

        if( display != nullptr )
        {
            display->UserEvents();
            display->BeginRender();

            if( texture != nullptr )
            {
                void* tex_map = texture->MapCUDA();

                if( tex_map != nullptr )
                {
                    cudaMemcpy(tex_map, fRm, texture->GetSize(), cudaMemcpyDeviceToDevice);
                    CUDA(cudaDeviceSynchronize());

                    texture->Unmap();
                }
                //texture->UploadCPU(texIn);

                texture->Render(100,100);
            }

            display->EndRender();
        }
    }


    cudaFree(fRm);
    fRm == nullptr;

}
