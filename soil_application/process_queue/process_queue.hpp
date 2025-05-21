
#pragma once
#include <BS_thread_pool.hpp>
#include <eventpp/eventqueue.h>
#include <functional>
#include <typeindex>

namespace soil {

    class TypeErasedSharedPtr {
    public:
        template <typename T>
        explicit TypeErasedSharedPtr(std::shared_ptr<T> value)
            : type_info(typeid(T)), value_ptr(value) {}

        template <typename T> std::shared_ptr<T> up_cast() const {
            if (type_info == typeid(T)) {
                return std::static_pointer_cast<T>(value_ptr);
            }
            throw std::bad_cast();
        }

        void reset() {
            this->value_ptr.reset();
            this->type_info = typeid(void);
        }

    private:
        std::reference_wrapper<const std::type_info> type_info;
        std::shared_ptr<void>                        value_ptr{};
    };

    // Future ideas:
    // If needed add a method to allow callbacks to be run on the threadpool

    class ProcessQueue {
    public:
        ProcessQueue()                    = default;
        ProcessQueue(ProcessQueue const&) = delete;
        ProcessQueue(ProcessQueue&&)      = delete;

        template <typename Event> void listen(std::function<void(Event&)>&& callback) {
            this->process_queue.appendListener(
                typeid(Event),
                [&, our_callback = std::move(callback)](TypeErasedSharedPtr ptr) {
                    const auto event = ptr.up_cast<Event>();
                    our_callback(*event);
                }
            );
        }

        template <typename Event> void fire(Event&& event) {
            this->process_queue.enqueue(
                typeid(Event),
                TypeErasedSharedPtr(std::make_shared<Event>(std::forward<Event>(event)))
            );
        }

        template <typename EventType>
        void spawn_job(
            std::function<void()>&& callback, EventType et, const BS::priority_t priority = 0
        ) {
            (void)this->execution_pool.submit_task(
                [clbk = std::move(callback), this, event = std::move(et)] {
                    clbk();
                    this->fire<EventType>(event);
                },
                priority
            );
        }

        template <typename EventType>
        void
        spawn_job(std::function<EventType()>&& callback, const BS::priority_t priority = 0) {
            (void)this->execution_pool.submit_task(
                [clbk = std::move(callback), this] {
                    auto event = clbk();
                    this->fire<EventType>(std::move(event));
                },
                priority
            );
        }

        void tick() { this->process_queue.process(); }

    private:
        BS::thread_pool<BS::tp::priority> execution_pool{
            std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1
                                                    : 1
        };

        eventpp::EventQueue<std::type_index, void(TypeErasedSharedPtr)> process_queue{};
    };

} // namespace soil
