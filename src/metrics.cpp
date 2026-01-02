#include "coap/metrics.hpp"
#include <sstream>

namespace coap {

Metrics& Metrics::instance() {
    static Metrics instance;
    return instance;
}

void Metrics::increment_requests_sent() {
    requests_sent++;
}

void Metrics::increment_responses_received() {
    responses_received++;
}

void Metrics::increment_mqtt_published() {
    mqtt_published++;
}

void Metrics::increment_errors() {
    errors++;
}

void Metrics::increment_mqtt_errors() {
    mqtt_errors++;
}

void Metrics::record_response_time(const std::string& sensor, double ms) {
    std::lock_guard<std::mutex> lock(response_time_mutex);
    sensor_response_times[sensor] += static_cast<uint64_t>(ms);
}

void Metrics::set_temperature(double celsius) {
    temperature_celsius.store(celsius);
}

void Metrics::set_vibration(double mm_per_sec) {
    vibration_mm_per_sec.store(mm_per_sec);
}

void Metrics::set_power(double kw) {
    power_kw.store(kw);
}

void Metrics::set_state(const std::string& state) {
    std::lock_guard<std::mutex> lock(state_mutex);
    current_state = state;
    
    // Convert state to numeric value for Prometheus
    if (state == "normal") {
        state_value.store(0);
    } else if (state == "degraded") {
        state_value.store(1);
    } else if (state == "failed") {
        state_value.store(2);
    } else {
        state_value.store(-1);  // unknown
    }
}

void Metrics::set_uptime(double seconds) {
    uptime_seconds.store(seconds);
}

std::string Metrics::export_prometheus() const {
    std::ostringstream oss;
    
    // Existing counters
    oss << "# HELP coap_requests_sent_total Total number of CoAP requests sent\n";
    oss << "# TYPE coap_requests_sent_total counter\n";
    oss << "coap_requests_sent_total " << requests_sent.load() << "\n\n";
    
    oss << "# HELP coap_responses_received_total Total number of CoAP responses received\n";
    oss << "# TYPE coap_responses_received_total counter\n";
    oss << "coap_responses_received_total " << responses_received.load() << "\n\n";
    
    oss << "# HELP mqtt_messages_published_total Total number of MQTT messages published\n";
    oss << "# TYPE mqtt_messages_published_total counter\n";
    oss << "mqtt_messages_published_total " << mqtt_published.load() << "\n\n";
    
    oss << "# HELP coap_errors_total Total number of CoAP errors encountered\n";
    oss << "# TYPE coap_errors_total counter\n";
    oss << "coap_errors_total " << errors.load() << "\n\n";
    
    oss << "# HELP mqtt_errors_total Total number of MQTT errors encountered\n";
    oss << "# TYPE mqtt_errors_total counter\n";
    oss << "mqtt_errors_total " << mqtt_errors.load() << "\n\n";
    
    // Per-sensor response times
    oss << "# HELP coap_response_time_ms Total CoAP response time in milliseconds per sensor\n";
    oss << "# TYPE coap_response_time_ms counter\n";
    {
        std::lock_guard<std::mutex> lock(response_time_mutex);
        for (const auto& [sensor, time_ms] : sensor_response_times) {
            oss << "coap_response_time_ms{sensor=\"" << sensor << "\"} " << time_ms << "\n";
        }
    }
    oss << "\n";
    
    // Sensor gauges
    oss << "# HELP machine_temperature_celsius Current machine temperature in Celsius\n";
    oss << "# TYPE machine_temperature_celsius gauge\n";
    oss << "machine_temperature_celsius " << temperature_celsius.load() << "\n\n";
    
    oss << "# HELP machine_vibration_mm_per_second Current machine vibration in mm/s\n";
    oss << "# TYPE machine_vibration_mm_per_second gauge\n";
    oss << "machine_vibration_mm_per_second " << vibration_mm_per_sec.load() << "\n\n";
    
    oss << "# HELP machine_power_kw Current machine power consumption in kW\n";
    oss << "# TYPE machine_power_kw gauge\n";
    oss << "machine_power_kw " << power_kw.load() << "\n\n";
    
    oss << "# HELP machine_state Current machine state (0=normal, 1=degraded, 2=failed, -1=unknown)\n";
    oss << "# TYPE machine_state gauge\n";
    oss << "machine_state " << state_value.load() << "\n\n";
    
    oss << "# HELP machine_uptime_seconds Machine uptime in seconds\n";
    oss << "# TYPE machine_uptime_seconds gauge\n";
    oss << "machine_uptime_seconds " << uptime_seconds.load() << "\n\n";
    
    return oss.str();
}

}  // namespace coap
