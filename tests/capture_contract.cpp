#include "capture.hpp"
#include "xtsdk.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
static void require(bool condition){if(!condition)throw std::runtime_error("capture contract failure");}
int main(){try{
    auto capture=lidar::xt_capture({"sensor","board",10});require(capture->start());
    auto frame=std::make_shared<XinTan::Frame>();frame->width=2;frame->height=2;frame->frame_id=42;frame->timeStampS=123;frame->timeStampNS=456;frame->timeStampType=0;frame->timeStampState=1;frame->frame_version=3;
    frame->points={{0,0,2},{std::numeric_limits<float>::quiet_NaN(),0,0},{.1f,.1f,2},{0,0,0}};frame->distData={2000,964000,2000,0};XinTan::test_callback(frame);
    auto out=capture->latest();require(bool(out));require(out->points.size()==4);require(out->source_id==42&&out->sdk_seconds==123&&out->sdk_nanoseconds==456&&out->protocol_version==3);
    require(!out->points[1].xyz_valid&&out->points[1].depth==964000);require(out->points[3].depth==0);require(!out->captured_at&&!out->captured_start&&!out->captured_end&&out->mapping_id.empty());
    require(!capture->latest());require(capture->stop());XinTan::test_callback(frame);require(!capture->latest());
    XinTan::test_start_ok=false;require(!capture->start());
    std::cout<<"SDK adapter: full slots, validity, raw timestamp, latest-only, actual-state confirmation passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
