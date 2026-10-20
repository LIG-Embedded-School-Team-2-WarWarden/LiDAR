# C2 원본 취득 라이브러리

`capture/`는 firmware 공통 런타임에서 사용하는 SDK 취득 라이브러리입니다.
기존 live_sender.cpp·뷰어·Detector/BoxTracker는 변경하지 않았습니다.
C2 원본 전송 경로에서는 여기서 객체 검출·거리 필터·군집 라벨 생성을 수행하지 않습니다.

- width×height 전체 SDK slots, 원본 XYZ, sdkDepthRaw, SDK frame ID·시각·종류·상태·프로토콜 버전을 보존합니다.
- 유효하지 않은 XYZ는 invalid로 표시하고 원본 depth는 유지합니다.
- SDK callback은 최신 프레임 1개만 보관합니다. 네트워크 전송은 firmware가 담당합니다.
- start/stop은 SDK 응답에 이어 실제 SDK 장치 상태(STREAM/IDLE)를 확인합니다.
- 수신·콜백 시각을 취득 UTC로 포장하지 않습니다. 원시 SDK 시각만 확인된 경우 capturedAt·취득 구간은 미제공입니다.
- 실물 취득 UTC·UART 자세 연결은 아직 별도 구현이 필요합니다. 합성 backend만 검증된 취득시각을 생성합니다.

API 대조 기준: XT-Toffuture/xtsdk_cpp `3d3db067ae9bdc0528202c3087bc10fd3b706638`의 xtsdk.h·frame.h.
해당 SDK의 실제 보드 설치 버전·장치의 timestamp/노출/필터 설정과 같다는 뜻은 아닙니다.

```sh
cmake -S . -B build -DLIDAR_WITH_XTSDK=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

기본 빌드는 합성 backend와 실제 adapter source를 API fixture에 연결한 계약 시험을 제공합니다.
실제 SDK를 링크하려면 LIDAR_WITH_XTSDK=ON, XTSDK_INCLUDE_DIR, XTSDK_LIBRARY를 지정합니다.
이 계약 시험은 SDK 바이너리·센서가 실제 측정을 시작/정지했음을 검증한 결과는 아닙니다.

## 실물 SDK 링크 조건

`XTSDK_LIBRARY`는 `XinTan::XtSdk` C++ wrapper를 제공하는 `libxtsdk.a` 등의 라이브러리입니다.
공식 저장소의 `libxtsdk_shared.so`만 지정하면 wrapper 전체를 대신하지 못합니다.
정적 wrapper의 OpenCV·Boost·dl 등 의존성은 `XTSDK_EXTRA_LIBRARIES`로 함께 지정하고,
wrapper가 실행 중 읽는 제조사 바이너리도 SDK가 기대하는 경로에 배치해야 합니다.

확인한 SDK 커밋의 제조사 Linux 바이너리는 x86_64/aarch64만 제공합니다.
firmware의 xc7z010 보드(32비트 ARM)에는 armhf용 제조사 바이너리를 별도 확보해야 합니다.
CI의 ARM object 컴파일은 소스 이식성 확인이며 전체 SDK 링크·센서 실행 검증이 아닙니다.
