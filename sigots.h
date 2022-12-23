#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <any>

namespace sigots {


template <typename... Args>
class Signal {

public:

    using slot_t = std::function<void(Args...)>;
   
private:
    
    class Slot {
    
    private:
        slot_t m_slot;
    
    public:
        Slot(slot_t slot) : m_slot(slot) {}

        virtual void call(Args... args) {
            m_slot(args...);
        };
    };

    template <typename T>
    class ObjectSlot : public Slot {

    public:
        ObjectSlot(T& obj, void(T::* s)(Args...)) :
        Slot(nullptr), object(obj), slot(s) {}

        T& object;
        void(T::* slot)(Args...);

        void call(Args... args) override {
            ((object).*slot)(args...);
        };

    };

    std::vector<std::unique_ptr<Slot>> m_slots;

public:

    void inline connect(slot_t slot) {
        m_slots.push_back(std::make_unique<Slot>(slot));
    }

    template <typename T>
    void inline connect(T& object, void(T::*slot)(Args...)) {
        m_slots.push_back(std::make_unique<ObjectSlot<T>>(object, slot));
    }

    void inline emit(Args... args) const {
        for (auto& s : m_slots) {
            s->call(args...);
        }
    }

    void operator()(Args... args) const {
        emit(args...);
    }
};

} // namespace sigots