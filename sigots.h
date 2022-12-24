#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <type_traits>

#include <iostream>

namespace sigots {

template <typename... Args>
class Signal {

public:
    using slot_t = std::function<void(Args...)>;
   
private:
    class Slot {
    
    private:
        slot_t m_slot;
        
        virtual bool isEqual(Slot& slot) {
            return
                m_slot.target_type() == slot.m_slot.target_type() &&
                m_slot.target<void(Args...)>() == slot.m_slot.target<void(Args...)>();
        }

    public:
        Slot(const slot_t slot) : m_slot(slot) {}

        virtual void call(Args... args) {
            m_slot(args...);
        };

        bool operator==(Slot& slot) {
            return isEqual(slot);
        }
    };

    template <typename T>
    class ObjectSlot : public Slot {

    private:
        T* m_object;
        void(T::* m_method)(Args...);

        bool isEqual(Slot& slot) override {
            bool b = m_object == static_cast<const ObjectSlot&>(slot).m_object;
            return b;
        }

    public:
        ObjectSlot(T* object, void(T::* method)(Args...)) :
            Slot(nullptr), m_object(object), m_method(method) {}

        void call(Args... args) override {
            (m_object->*m_method)(args...);
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
    void inline connect(T* object, void(T::*slot)(Args...)) {
        static_assert(std::is_class<T>::value);
        m_slots.push_back(std::make_unique<ObjectSlot<T>>(object, slot));
    }

    template <typename T>
    void inline disconnect(T* object, void(T::* slot)(Args...)) {
        static_assert(std::is_class<T>::value);
        ObjectSlot os{ object, slot };
        auto it = std::find_if(m_slots.begin(), m_slots.end(), [&](std::unique_ptr<Slot> const& s) {
            return *s == os;
        });

        if (it != m_slots.end()) { 
            m_slots.erase(it);
        }
    }

    void inline emit(Args... args) const {
        std::cout << "emits: " << m_slots.size() << std::endl;
        for (auto& s : m_slots) {
            std::cout << "slot: " << std::endl;
            s->call(args...);
        }
    }

    void operator()(Args... args) const {
        emit(args...);
    }
};

} // namespace sigots
