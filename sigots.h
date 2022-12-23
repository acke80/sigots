#pragma once

#include <functional>
#include <memory>
#include <vector>

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

    private:
        T& m_object;
        void(T::* m_method)(Args...);

    public:
        ObjectSlot(T& object, void(T::* method)(Args...)) :
            Slot(nullptr), m_object(object), m_method(method) {}

        void call(Args... args) override {
            (m_object.*m_method)(args...);
        };
    };

    std::vector<std::unique_ptr<Slot>> m_slots;

public:
    /**
    * Connects free function as slot to signal. 
    */
    void inline connect(slot_t slot) {
        m_slots.push_back(std::make_unique<Slot>(slot));
    }

    /**
    * Connects member function as slot to signal.
    * @tparam T     The class that owns the member function.
    * @param object The object instance of type T.
    * @param slot   The member function of object to connect.
    */
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
