/*
 *  Copyright 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/api/rtcstatsreport.h"

#include "webrtc/api/rtcstats.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/gunit.h"

namespace webrtc {

class RTCTestStats1 : public RTCStats {
 public:
  RTCTestStats1(const std::string& id, double timestamp)
      : RTCStats(id, timestamp),
        integer("integer") {}

  WEBRTC_RTCSTATS_IMPL(RTCStats, RTCTestStats1,
      &integer);

  RTCStatsMember<int32_t> integer;
};

const char RTCTestStats1::kType[] = "test-stats-1";

class RTCTestStats2 : public RTCStats {
 public:
  RTCTestStats2(const std::string& id, double timestamp)
      : RTCStats(id, timestamp),
        number("number") {}

  WEBRTC_RTCSTATS_IMPL(RTCStats, RTCTestStats2,
      &number);

  RTCStatsMember<double> number;
};

const char RTCTestStats2::kType[] = "test-stats-2";

class RTCTestStats3 : public RTCStats {
 public:
  RTCTestStats3(const std::string& id, double timestamp)
      : RTCStats(id, timestamp),
        string("string") {}

  WEBRTC_RTCSTATS_IMPL(RTCStats, RTCTestStats3,
      &string);

  RTCStatsMember<std::string> string;
};

const char RTCTestStats3::kType[] = "test-stats-3";

TEST(RTCStatsReport, AddAndGetStats) {
  rtc::scoped_refptr<RTCStatsReport> report = RTCStatsReport::Create();
  EXPECT_EQ(report->size(), static_cast<size_t>(0));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("a0", 1.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("a1", 2.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("b0", 4.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("b1", 8.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("a2", 16.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("b2", 32.0)));
  EXPECT_EQ(report->size(), static_cast<size_t>(6));

  EXPECT_EQ(report->Get("missing"), nullptr);
  EXPECT_EQ(report->Get("a0")->id(), "a0");
  EXPECT_EQ(report->Get("b2")->id(), "b2");

  std::vector<const RTCTestStats1*> a = report->GetStatsOfType<RTCTestStats1>();
  EXPECT_EQ(a.size(), static_cast<size_t>(3));
  uint32_t mask = 0;
  for (const RTCTestStats1* stats : a)
    mask |= static_cast<uint32_t>(stats->timestamp());
  EXPECT_EQ(mask, static_cast<uint32_t>(1 | 2 | 16));

  std::vector<const RTCTestStats2*> b = report->GetStatsOfType<RTCTestStats2>();
  EXPECT_EQ(b.size(), static_cast<size_t>(3));
  mask = 0;
  for (const RTCTestStats2* stats : b)
    mask |= static_cast<uint32_t>(stats->timestamp());
  EXPECT_EQ(mask, static_cast<uint32_t>(4 | 8 | 32));

  EXPECT_EQ(report->GetStatsOfType<RTCTestStats3>().size(),
            static_cast<size_t>(0));
}

TEST(RTCStatsReport, StatsOrder) {
  rtc::scoped_refptr<RTCStatsReport> report = RTCStatsReport::Create();
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("C", 2.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("D", 3.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("B", 1.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("A", 0.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("E", 4.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("F", 5.0)));
  report->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats2("G", 6.0)));
  size_t i = 0;
  for (const RTCStats& stats : *report) {
    EXPECT_EQ(static_cast<size_t>(stats.timestamp()), i);
    ++i;
  }
  EXPECT_EQ(i, static_cast<size_t>(7));
}

TEST(RTCStatsReport, TakeMembersFrom) {
  rtc::scoped_refptr<RTCStatsReport> a = RTCStatsReport::Create();
  a->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("B", 1.0)));
  a->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("C", 2.0)));
  a->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("E", 4.0)));
  rtc::scoped_refptr<RTCStatsReport> b = RTCStatsReport::Create();
  b->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("A", 0.0)));
  b->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("D", 3.0)));
  b->AddStats(std::unique_ptr<RTCStats>(new RTCTestStats1("F", 5.0)));

  a->TakeMembersFrom(b);
  EXPECT_EQ(b->size(), static_cast<size_t>(0));
  size_t i = 0;
  for (const RTCStats& stats : *a) {
    EXPECT_EQ(static_cast<size_t>(stats.timestamp()), i);
    ++i;
  }
  EXPECT_EQ(i, static_cast<size_t>(6));
}

}  // namespace webrtc
