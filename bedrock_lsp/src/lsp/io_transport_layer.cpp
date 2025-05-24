#include "io_transport_layer.hpp"
#include <iostream>
#include <sstream>

namespace bsp {
    struct RpcPacketInfo {
        size_t content_length{};
    };

    static std::string read_line_clrf() {
        std::string line;
        char        ch;
        char        prev = 0;

        while (std::cin.get(ch)) {
            line.push_back(ch);
            if (prev == '\r' && ch == '\n') {
                line.pop_back();
                line.pop_back();
                return line;
            }
            prev = ch;
        }

        return {};
    }

    void IOTransportLayer::next_unchecked_message(std::string& output) {
        RpcPacketInfo info{};
        output.clear();
        while (true) {
            if (const auto line = read_line_clrf(); line.starts_with("Content-Length: ")) {
                info.content_length = std::stoi(line.substr(16));
            } else if (line.empty()) {
                break; // Means we hit a line of \r\n
            }
        }

        for (size_t x = 0; x < info.content_length; x++) {
            output += std::cin.get();
        }
    }

    void IOTransportLayer::post_raw(const std::string_view data_to_send) {
        TransportLayer::post_raw(data_to_send);
        std::cout << std::format(
                         "Content-Length: {}\r\n\r\n{}", data_to_send.length(), data_to_send
                     )
                  << std::endl;
    }
} // namespace bsp