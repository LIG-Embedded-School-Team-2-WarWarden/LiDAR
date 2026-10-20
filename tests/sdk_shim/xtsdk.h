#pragma once
// Test fixture matching the public API used by capture/xt_capture.cpp.
// SDK source reference: XT-Toffuture/xtsdk_cpp @ 3d3db067ae9bdc0528202c3087bc10fd3b706638.
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace XinTan {
enum ClOUDCOORD_TYPE{ClOUDCOORD_CAMERA};enum ImageType{IMG_POINTCLOUD};
enum DevStateCode{DevSTATE_IDLE=2,DevSTATE_STREAM=3};
struct CBEventData{};
struct Point{float x=0,y=0,z=0;};
struct Frame{
    unsigned width=0,height=0;std::uint64_t frame_id=0,timeStampS=0;
    std::uint32_t timeStampNS=0;std::uint8_t timeStampType=0,timeStampState=0,frame_version=0;
    std::vector<Point> points;std::vector<std::uint32_t> distData;
};
inline std::function<void(const std::shared_ptr<Frame>&)> test_callback;
inline bool test_start_ok=true,test_stop_ok=true;
class XtSdk{
    DevStateCode state_=DevSTATE_IDLE;
public:
    bool setConnectIpaddress(std::string){return true;}
    bool setSdkCloudCoordType(ClOUDCOORD_TYPE){return true;}
    bool setUdpDestIp(std::string,std::uint16_t){return true;}
    bool setMaxFps(std::uint8_t){return true;}
    void setCallback(std::function<void(const std::shared_ptr<CBEventData>&)> event={},std::function<void(const std::shared_ptr<Frame>&)> f={}){test_callback=std::move(f);}
    void startup(){} void shutdown(){} bool isconnect(){return true;}
    bool start(ImageType){if(test_start_ok)state_=DevSTATE_STREAM;return test_start_ok;}
    bool stop(){if(test_stop_ok)state_=DevSTATE_IDLE;return test_stop_ok;}
    DevStateCode getDevState(){return state_;}
};
}
