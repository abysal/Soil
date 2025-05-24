#pragma once
#include <deque>
#include <print>
#include <string>
#include <string_view>

namespace bsp {

    class TransportLayer {
    public:
        virtual ~TransportLayer() = default;

        // This is a param instead of return for shared memory allocation
        virtual void next_unchecked_message(std::string& output) = 0;

        // This function is called when we want to force the next value to be a "" null string.
        // This is mainly used for flushing
        virtual void inject_null() = 0;

        virtual void post_raw(std::string_view data_to_send) {}
    };

    namespace test {
        class ShimLayer final : public TransportLayer {
        public:
            ~ShimLayer() override = default;
            void next_unchecked_message(std::string& output) override {
                if (messages.size() == 0) {
                    output = "";
                    return;
                }

                output = messages.front();
                messages.pop_front();
            }

            void inject_null() override { this->messages.push_front(""); }

            void post_raw(std::string_view data_to_send) override {
                std::println("RPCData: {}", data_to_send);
            }

            void inject(std::string&& str) { this->messages.emplace_back(std::move(str)); }

        private:
            std::deque<std::string> messages{};
        };
    } // namespace test
} // namespace bsp
