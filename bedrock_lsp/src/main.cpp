#include "rpc/rpc_manager.hpp"

#include <hjson.h>
#include <rpc/transport_layer.hpp>

Hjson::Value subtract(const Hjson::Value& param) {
    if (param.type() != Hjson::Type::Vector) {
        throw std::runtime_error("subtract error, not vector");
    }

    return param[0] - param[1];
}

int main() {
    bsp::test::ShimLayer layer{};
    layer.inject(
        R"([{"jsonrpc": "2.0", "method": "subtract", "params": [23, 42], "id": 3},
                 {"jsonrpc": "2.0", "method": "subtract", "params": [23, 42], "id": 4}])"
    );

    auto            ptr = std::make_unique<bsp::test::ShimLayer>(std::move(layer));
    bsp::RpcManager manager{std::move(ptr)};
    manager.bind("subtract", subtract);
    manager.run();
}