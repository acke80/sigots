#pragma once

#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <type_traits>

#ifndef slots
#define slots
#endif

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

        virtual void callback(Args... args) {
            m_slot(args...);
        };

        bool operator==(Slot& slot) {
            return isEqual(slot);
        }
    }; // class Slot

    template <typename T>
    class ObjectSlot : public Slot {

    private:
        T* m_object;
        void(T::* m_method)(Args...);

        std::shared_ptr<T> m_objectTracker{nullptr};

        bool isEqual(Slot& slot) override {
            const ObjectSlot& os = static_cast<const ObjectSlot&>(slot);
            return m_object == os.m_object && m_method == os.m_method;
        }

    public:
        ObjectSlot(T* object, void(T::* method)(Args...)) :
            Slot(nullptr), m_object(object), m_method(method) {}

        ObjectSlot(std::shared_ptr<T> objectTracker, void(T::* method)(Args...)) :
            Slot(nullptr), m_object(objectTracker.get()), m_method(method), m_objectTracker(objectTracker) {}

        void callback(Args... args) override {
            (m_object->*m_method)(args...);
        };

        const T* getObject() {
            return m_object;
        }

    }; // class ObjectSlot

    std::set<std::unique_ptr<Slot>> m_slots;

    void removeSlot(Slot& slot) {
        auto it = std::find_if(m_slots.begin(), m_slots.end(),
            [&](const std::unique_ptr<Slot>& s) { return *s == slot; });

        if (it != m_slots.end()) m_slots.erase(it);
    }

public:
    /**
    * Connects free function as slot to signal. 
    */
    void inline connect(slot_t slot) {
        m_slots.insert(std::make_unique<Slot>(slot));
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

        m_slots.insert(std::make_unique<ObjectSlot<T>>(object, slot));
    }

    template <typename T>
    void inline connect(std::shared_ptr<T> object, void(T::*slot)(Args...)) {
        static_assert(std::is_class<T>::value);
        
        m_slots.insert(std::make_unique<ObjectSlot<T>>(object, slot));
    }

    /**
    * Disconnects free function slot.
    */
    void inline disconnect(slot_t slot) {
        removeSlot({ slot });
    }

    /**
    * Disconnects member function slot.
    * @tparam T     The class that owns the member function.
    * @param object The object instance of type T.
    * @param slot   The member function of object to disconnect.
    */
    template <typename T>
    void inline disconnect(T* object, void(T::*slot)(Args...)) {
        static_assert(std::is_class<T>::value);
        ObjectSlot<T> os{ object, slot };
        removeSlot(os);
    }

    /**
    * Disconnects all slots.
    */
    void inline clear() {
        m_slots.clear();
    }

    void inline emit(Args... args) const {
        for (auto& s : m_slots) s->callback(args...);
    }

    void operator()(Args... args) const {
        emit(args...);
    }
};

} // namespace sigots
