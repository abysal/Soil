#pragma once
#include "transport_layer.hpp"
#include <array>
#include <functional>
#include <hjson.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace bsp {
    constexpr static int32_t InvalidRequest = -32600;
    class TransportLayer;

    class JsonRPCValidationError final : public std::runtime_error {
    public:
        explicit JsonRPCValidationError(
            const std::string& message, const int32_t code = InvalidRequest
        )
            : runtime_error(std::format("JsonRPC Error: {}", message)), code(code) {}
        int32_t code{};
    };

    // All methods that this calls must be multi thread safe

    class RpcManager {
    private:
        using RpcCall   = const class Hjson::Value&;
        using RPCMethod = std::function<Hjson::Value(RpcCall)>;
        using RPCLookup = std::unordered_map<std::string, RPCMethod>;
        struct ErrorInfo {
            int32_t     code{};
            std::string message{};
        };

    public:
        explicit RpcManager(std::unique_ptr<TransportLayer> transportLayer)
            : transport_layer(std::move(transportLayer)) {}

        [[maybe_unused]] RpcManager& bind(std::string&& proc_name, RPCMethod&& callback);
        bool                         process_rpc(std::string& buffer);

        // This function takes control of the thread, and any bind calls after this are UB
        void run();

    private:
        static bool validate_rpc_object(const class Hjson::Value& json);
        static bool validate_rpc_batch(const class Hjson::Value& json);

        Hjson::Value process_request(Hjson::Value& json);

        template <size_t N>
        static bool
        json_one_of(const Hjson::Value& json, const std::array<Hjson::Type, N>& array) {
            for (const auto val : array) {
                if (json.type() == val) {
                    return true;
                }
            }
            return false;
        }

        void send_error(const ErrorInfo& error, const class Hjson::Value& id) const;
        static Hjson::Value make_response(const Hjson::Value& object, const Hjson::Value& id);

        void send_response(const Hjson::Value& object, const Hjson::Value& id) const;

        static bool json_has_member(const class Hjson::Value& json, const std::string& key);

    private:
        std::unique_ptr<TransportLayer> transport_layer = {};
        RPCLookup                       methods         = {};
        bool                            running         = {true};
    };
}; // namespace bsp
