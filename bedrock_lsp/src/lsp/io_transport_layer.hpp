#pragma once
#include "rpc/transport_layer.hpp"

namespace bsp {

    class IOTransportLayer final : public TransportLayer {
    public:
        ~IOTransportLayer() override = default;
        void next_unchecked_message(std::string& output) override;
        void inject_null() override {}
        void post_raw(std::string_view data_to_send) override;
    };

} // namespace bsp
