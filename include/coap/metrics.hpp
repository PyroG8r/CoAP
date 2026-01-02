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
    void increment_mqtt_errors();
    void record_response_time(const std::string& sensor, double ms);
    
    // Sensor value gauges
    void set_temperature(double celsius);
    void set_vibration(double mm_per_sec);
    void set_power(double kw);
    void set_state(const std::string& state);
    void set_uptime(double seconds);
    
    std::string export_prometheus() const;

private:
    Metrics() = default;
    
    std::atomic<uint64_t> requests_sent{0};
    std::atomic<uint64_t> responses_received{0};
    std::atomic<uint64_t> mqtt_published{0};
    std::atomic<uint64_t> errors{0};
    std::atomic<uint64_t> mqtt_errors{0};
    
    // Per-sensor response times
    mutable std::mutex response_time_mutex;
    std::map<std::string, uint64_t> sensor_response_times;
    
    // Sensor gauges (using double for atomic operations)
    std::atomic<double> temperature_celsius{0.0};
    std::atomic<double> vibration_mm_per_sec{0.0};
    std::atomic<double> power_kw{0.0};
    std::atomic<int> state_value{0};  // 0=normal, 1=degraded, 2=failed
    std::atomic<double> uptime_seconds{0.0};
    
    mutable std::mutex state_mutex;
    std::string current_state{"unknown"};
};

}  // namespace coap
