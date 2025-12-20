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

void Metrics::record_response_time(double ms) {
    total_response_time_ms += static_cast<uint64_t>(ms);
}

std::string Metrics::export_prometheus() const {
    std::ostringstream oss;
    
    oss << "# HELP coap_requests_sent_total Total number of CoAP requests sent\n";
    oss << "# TYPE coap_requests_sent_total counter\n";
    oss << "coap_requests_sent_total " << requests_sent.load() << "\n\n";
    
    oss << "# HELP coap_responses_received_total Total number of CoAP responses received\n";
    oss << "# TYPE coap_responses_received_total counter\n";
    oss << "coap_responses_received_total " << responses_received.load() << "\n\n";
    
    oss << "# HELP mqtt_messages_published_total Total number of MQTT messages published\n";
    oss << "# TYPE mqtt_messages_published_total counter\n";
    oss << "mqtt_messages_published_total " << mqtt_published.load() << "\n\n";
    
    oss << "# HELP coap_errors_total Total number of errors encountered\n";
    oss << "# TYPE coap_errors_total counter\n";
    oss << "coap_errors_total " << errors.load() << "\n\n";
    
    oss << "# HELP coap_total_response_time_ms Total CoAP response time in milliseconds\n";
    oss << "# TYPE coap_total_response_time_ms counter\n";
    oss << "coap_total_response_time_ms " << total_response_time_ms.load() << "\n\n";
    
    return oss.str();
}

}  // namespace coap
