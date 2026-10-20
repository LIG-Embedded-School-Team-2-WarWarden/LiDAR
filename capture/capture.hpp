#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace lidar {
struct Point { float x=0,y=0,z=0;std::uint32_t depth=0;bool xyz_valid=false; };
struct Frame {
    std::uint32_t width=0,height=0;
    std::uint64_t source_id=0,sdk_seconds=0;
    std::uint32_t sdk_nanoseconds=0,sync_type=0,sync_state=0,protocol_version=0;
    std::vector<Point> points;
    // Only a separately verified acquisition mapping may populate these.
    std::optional<double> captured_start,captured_at,captured_end,uncertainty_ms;
    std::string mapping_id;
};
struct Config { std::string sensor_ip,board_ip;unsigned fps=10; };
class Capture {
public:
    virtual ~Capture()=default;
    virtual bool start()=0;
    virtual bool stop()=0;
    virtual std::optional<Frame> latest()=0;
};
std::unique_ptr<Capture> simulated_capture();
#ifdef LIDAR_WITH_XTSDK
std::unique_ptr<Capture> xt_capture(const Config&);
#endif
}
