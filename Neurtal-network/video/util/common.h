#pragma once

#include <vector>
#include <assert.h>
#include <istream>
#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <map>

#include "../yolo/network/yolov3.h"

namespace fs = std::experimental::filesystem;

#define CHECK(status)                                                                          \
{                                                                                              \
    if (status != 0)                                                                           \
    {                                                                                          \
        std::cout << "Cuda failure: " << cudaGetErrorString(status) << " in file " << __FILE__ \
                  << " at line " << __LINE__ << std::endl;                                     \
        abort();                                                                               \
    }                                                                                          \
}

#define LOG_GSTRM "[camer.cpp]"


class common
{
public:
    common() {}

    static std::vector<std::string> loadListFromTextFile(const std::string filename)
    {
        assert(fileExists(filename));
        std::vector<std::string> list;

        std::ifstream f(filename);
        if (!f)
        {
            std::cout << "failed to open " << filename;
            assert(0);
        }

        std::string line;
        while (std::getline(f, line))
        {
            if (line.empty())
                continue;

            else
                list.push_back(trim(line));
        }

        return list;
    }

   static bool fileExists(const std::string fileName)
    {
        //fs::path file = fileName;
        if (!fs::exists(fs::path(fileName)))
        {
            std::cout << "File does not exist : " << fileName << std::endl;
            return false;
        }
        return true;
    }

    static void leftTrim(std::string& s)
    {
        s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) { return !isspace(ch); }));
    }

    static void rightTrim(std::string& s)
    {
        s.erase(find_if(s.rbegin(), s.rend(), [](int ch) { return !isspace(ch); }).base(), s.end());
    }

    static std::string trim(std::string s)
    {
        leftTrim(s);
        rightTrim(s);
        return s;
    }

    static bool splitCfgfile(int &batchsize , NetworkInfo &yoloInfo, InferParams &yoloInferParams,const std::vector<std::string> &config_Info)
    {
        std::map<std::string,std::string> map_config_info;
        for(int id=0;id < config_Info.size();id++)
        {
           std::string  cur_string = config_Info[id];
           int pos = cur_string.find('=');
           if(pos == std::string::npos) continue;
           std::string name_string = cur_string.substr(0,pos);
           map_config_info[name_string] = cur_string.substr(pos+1);
        }
        std::string network_type = map_config_info["network_type"];
        std::string config_file_path = map_config_info["config_file_path"];
        std::string wts_file_path = map_config_info["wts_file_path"];
        std::string labels_file_path = map_config_info["labels_file_path"];
        std::string precision = map_config_info["precision"];
        std::string deviceType = map_config_info["deviceType"];
        batchsize = std::stoi(map_config_info["batch_size"]);
        float prob_thresh = std::stof(map_config_info["prob_thresh"]);
        float nms_thresh = std::stof(map_config_info["nms_thresh"]);
        int npos = wts_file_path.find(".weights");
        std::string dataPath = wts_file_path.substr(0, npos);
        std::string engine_file_path = dataPath + "-" + precision + "-" + deviceType + "-batch"
            + std::to_string(batchsize) + ".engine";

        yoloInfo = NetworkInfo{network_type,config_file_path, wts_file_path,
               labels_file_path,precision,deviceType,"", engine_file_path, "data"};
        yoloInferParams = InferParams{0, 0, "", "",
                prob_thresh,nms_thresh};

        return true;
    }
};
