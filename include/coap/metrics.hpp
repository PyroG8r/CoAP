#pragma once

#include <atomic>
#include <string>
#include <mutex>
#include <map>

namespace coap {

class Metrics {
public:
    static Metrics& instance();

    void increment_requests_sent();
    void increment_responses_received();
    void increment_mqtt_published();
    void increment_errors();
    void record_response_time(double ms);
    
    std::string export_prometheus() const;

private:
    Metrics() = default;
    
    std::atomic<uint64_t> requests_sent{0};
    std::atomic<uint64_t> responses_received{0};
    std::atomic<uint64_t> mqtt_published{0};
    std::atomic<uint64_t> errors{0};
    std::atomic<uint64_t> total_response_time_ms{0};
};

}  // namespace coap
