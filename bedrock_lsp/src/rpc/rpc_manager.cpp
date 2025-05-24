#include "rpc_manager.hpp"
#include "transport_layer.hpp"
#include <hjson.h>
#include <iostream>

namespace bsp {
    RpcManager& RpcManager::bind(std::string&& proc_name, RPCMethod&& callback) {
        this->methods[std::move(proc_name)] = std::move(callback);
        return *this;
    }

    bool RpcManager::process_rpc(std::string& buffer) {
        this->transport_layer->next_unchecked_message(buffer);

        if (buffer.empty()) {
            this->running = false;
            return true;
        }

        constexpr auto options = Hjson::DecoderOptions{};

        auto hjson = Hjson::Unmarshal(buffer, options);

        try {
            if (hjson.type() == Hjson::Type::Map) (void)this->validate_rpc_object(hjson);
            else if (hjson.type() == Hjson::Type::Vector) (void)this->validate_rpc_batch(hjson);

        } catch (const JsonRPCValidationError& error) {
            if (!json_has_member(hjson, "id")) {
                return false;
            }

            this->send_error({.code = error.code, .message = error.what()}, hjson.at("id"));
            return false;
        }

        try {
            const auto value = this->process_request(hjson);

            if (!value.defined()) {
                return false;
            }

            if (value.type() == Hjson::Type::Vector) {
                this->transport_layer->post_raw(Hjson::MarshalJson(value));
            } else {
                send_response(value, hjson["id"]);
            }

        } catch (const std::runtime_error& error) {

            if (!json_has_member(hjson, "id")) {
                return false;
            }

            this->send_error({.code = -32000, .message = error.what()}, hjson.at("id"));
        }

        return false;
    }

    void RpcManager::run() {
        std::string buffer{};
        buffer.reserve(1024);

        while (this->running) {
            try {
                if (process_rpc(buffer)) return;

            } catch (std::runtime_error& e) {
                // TODO: Handle by sending window/logMessage with type 1
                // This should be pulled into its own error function

                std::cerr << std::format("RPC Error: {}", e.what()) << std::endl;
            }
        }
    }

    bool RpcManager::validate_rpc_object(const Hjson::Value& json) {
        try {
            if (const auto version = json.at("jsonrpc").to_string(); version != "2.0") {
                throw JsonRPCValidationError("Invalid RPC version");
            }
        } catch (Hjson::index_out_of_bounds& /* unused */) {
            throw JsonRPCValidationError("missing jsonrpc key");
        } catch (Hjson::type_mismatch& /*unused */) {
            throw JsonRPCValidationError("jsonrpc not string");
        }

        try {
            (void)json.at("method").to_string();
        } catch (Hjson::index_out_of_bounds& /* unused */) {
            throw JsonRPCValidationError("Missing method key");
        } catch (Hjson::type_mismatch& /*unused */) {
            throw JsonRPCValidationError("\"method\" was not a string");
        }

        try {
            if (const auto& id = json.at("id"); !json_one_of<3>(
                    id, {Hjson::Type::Int64, Hjson::Type::Undefined, Hjson::Type::String}
                )) {
                throw JsonRPCValidationError("Id was not a string or null or int");
            }
        } catch (Hjson::index_out_of_bounds& /* unused */) {}

        return true;
    }

    bool RpcManager::validate_rpc_batch(const Hjson::Value& json) {
        const auto size = json.size();

        if (json.type() != Hjson::Type::Vector) {
            throw JsonRPCValidationError("Invalid RPC object");
        }

        for (size_t x = 0; x < size; x++) {
            const auto& object = json[x];

            (void)validate_rpc_object(object);
        }

        return true;
    }

    // TODO: Handle batched methods erroring
    // TODO: Handle empty array
    Hjson::Value RpcManager::process_request(Hjson::Value& json) {
        if (json.type() == Hjson::Type::Vector) {
            const auto size = json.size();

            Hjson::Value result = {};
            for (size_t x = 0; x < size; x++) {
                auto value = this->process_request(json[x]);

                if (!json_has_member(json[x], "id")) {
                    continue;
                }

                result.push_back(this->make_response(std::move(value), json[x]["id"]));
            }
            return result;
        }
        try {

            const Hjson::Value& param = [&]() -> const Hjson::Value& {
                if (json_has_member(json, "params")) {
                    return json.at("params");
                }
                return Hjson::Value{};
            }();

            auto value = this->methods.at(json.at("method").to_string())(param);

            if (!json_has_member(json, "id")) {
                return Hjson::Value();
            }

            return value;
        } catch (std::out_of_range& e) {
            throw JsonRPCValidationError("Method not found", -32601);
        } catch (const JsonRPCValidationError& error) {
            throw error; // Just a simple passthrough to note that we handle it
        }
    }

    void RpcManager::send_error(const ErrorInfo& error, const Hjson::Value& id) const {
        Hjson::Value response;
        response["id"]      = id;
        response["jsonrpc"] = "2.0";
        response["error"]   = [&]() -> Hjson::Value {
            Hjson::Value error_response;
            error_response["code"]    = error.code;
            error_response["message"] = error.message;
            return error_response;
        }();

        this->transport_layer->post_raw(Hjson::Marshal(response));
    }

    Hjson::Value RpcManager::make_response(const Hjson::Value& object, const Hjson::Value& id) {
        Hjson::Value response;
        response["id"]      = id;
        response["jsonrpc"] = "2.0";
        response["result"]  = object;
        return response;
    }

    void RpcManager::send_response(const Hjson::Value& object, const Hjson::Value& id) const {

        this->transport_layer->post_raw(Hjson::MarshalJson(make_response(object, id)));
    }

    bool RpcManager::json_has_member(const Hjson::Value& json, const std::string& key) {
        try {
            (void)json.at(key);
            return true;
        } catch (Hjson::index_out_of_bounds&) {
            return false;
        }
    }
} // namespace bsp