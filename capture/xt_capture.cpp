#include "capture.hpp"
#include "xtsdk.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace lidar {
class XtCapture final:public Capture {
    XinTan::XtSdk sdk_;
    std::atomic<bool> collecting_{false};
    std::mutex mutex_;
    std::optional<Frame> pending_;
public:
    explicit XtCapture(const Config& c){
        if(!sdk_.setConnectIpaddress(c.sensor_ip) || !sdk_.setSdkCloudCoordType(XinTan::ClOUDCOORD_CAMERA) || !sdk_.setUdpDestIp(c.board_ip,7687))throw std::runtime_error("LiDAR SDK configuration failed");
        sdk_.setCallback([](const std::shared_ptr<XinTan::CBEventData>&){},[this](const std::shared_ptr<XinTan::Frame>& source){
            if(!collecting_)return;
            const auto count=std::uint64_t(source->width)*source->height;
            if(!count || count>500000 || source->points.size()!=count || source->distData.size()!=count)return;
            Frame f;f.width=source->width;f.height=source->height;f.source_id=source->frame_id;
            f.sdk_seconds=source->timeStampS;f.sdk_nanoseconds=source->timeStampNS;
            f.sync_type=source->timeStampType;f.sync_state=source->timeStampState;
            f.protocol_version=source->frame_version;
            for(std::size_t i=0;i<count;++i){const auto& p=source->points[i];const bool valid=std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);
                f.points.push_back({valid?p.x:0,valid?p.y:0,valid?p.z:0,source->distData[i],valid});}
            // SDK tick/host receive time is NOT verified sensor acquisition UTC.
            std::lock_guard<std::mutex> lock(mutex_);if(collecting_)pending_=std::move(f);
        });
        sdk_.startup();
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(30);
        while(!sdk_.isconnect() && std::chrono::steady_clock::now()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(!sdk_.isconnect() || !sdk_.stop() || !sdk_.setMaxFps(c.fps) ){
            sdk_.setCallback();sdk_.shutdown();throw std::runtime_error("LiDAR SDK initialization failed");
        }
    }
    ~XtCapture()override{collecting_=false;sdk_.stop();sdk_.setCallback();sdk_.shutdown();}
    bool start()override{
        {std::lock_guard<std::mutex> lock(mutex_);pending_.reset();}
        if(!sdk_.start(XinTan::IMG_POINTCLOUD))return false;
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
        while(sdk_.getDevState()!=XinTan::DevSTATE_STREAM&&std::chrono::steady_clock::now()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(20));
        const bool ok=sdk_.getDevState()==XinTan::DevSTATE_STREAM;collecting_=ok;
        if(!ok)sdk_.stop();return ok;
    }
    bool stop()override{
        collecting_=false;bool ok=sdk_.stop();
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
        while(ok&&sdk_.getDevState()!=XinTan::DevSTATE_IDLE&&std::chrono::steady_clock::now()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(20));
        ok=ok&&sdk_.getDevState()==XinTan::DevSTATE_IDLE;
        std::lock_guard<std::mutex> lock(mutex_);pending_.reset();return ok;
    }
    std::optional<Frame> latest()override{std::lock_guard<std::mutex> lock(mutex_);auto out=std::move(pending_);pending_.reset();return out;}
};
std::unique_ptr<Capture> xt_capture(const Config& c){return std::make_unique<XtCapture>(c);}
}
