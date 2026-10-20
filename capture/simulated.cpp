#include "capture.hpp"
#include <atomic>
#include <chrono>
#include <cmath>

namespace lidar {
class Simulated final:public Capture {
    std::atomic<bool> collecting_{false};
    std::chrono::steady_clock::time_point last_{};
    std::uint64_t sequence_=0;
public:
    bool start()override{collecting_=true;return true;}
    bool stop()override{collecting_=false;return true;}
    std::optional<Frame> latest()override{
        const auto now=std::chrono::steady_clock::now();
        if(!collecting_ || now-last_<std::chrono::milliseconds(120))return {};
        last_=now;Frame f;f.width=8;f.height=6;f.source_id=++sequence_;
        const auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()-3;
        f.captured_at=ms/1000.;f.captured_start=*f.captured_at-.001;f.captured_end=*f.captured_at+.001;
        f.sdk_seconds=ms/1000;f.sdk_nanoseconds=(ms%1000)*1000000;f.sync_type=2;f.sync_state=1;f.protocol_version=3;
        f.mapping_id="synthetic-clock-v1";f.uncertainty_ms=1;
        for(unsigned i=0;i<48;++i)f.points.push_back({float(.2+(int(i%8)-3.5)*.02+.02*std::sin(sequence_*.1)),float(.1+(int(i/8)-2.5)*.02),3,3000,true});
        return f;
    }
};
std::unique_ptr<Capture> simulated_capture(){return std::make_unique<Simulated>();}
}
