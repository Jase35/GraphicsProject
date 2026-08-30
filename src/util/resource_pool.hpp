#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace engine{
    template<typename T>
    struct Handle{
        static constexpr uint16_t invalidIndex = 0xFFFF;
        uint16_t index = invalidIndex;
        uint16_t generation = 0;
        bool valid() const{ return index != invalidIndex; }
        bool operator==(const Handle&) const = default;
    };

    template<typename T>
    class ResourcePool{
        public:
        static constexpr size_t maxSlots = Handle<T>::invalidIndex;
        
        Handle<T> add(T&& resource){
            uint16_t index;
            if(!freeList.empty()){
                index = freeList.back();
                freeList.pop_back();
                slots[index].resource = std::move(resource);
                slots[index].alive = true;
            }else{
                if(slots.size() >= maxSlots){
                    return {};
                }
                index = static_cast<uint16_t>(slots.size());
                slots.push_back({std::move(resource), 0, true});
            }
            return { index, slots[index].generation };
        }

        T* get(Handle<T> h){
            if(h.index >= slots.size()){
                return nullptr;
            }
            auto& s = slots[h.index];
            return (s.alive && s.generation == h.generation) ? &s.resource : nullptr;
        }

        T* get(uint16_t index){
            if(index >= slots.size()){
                return nullptr;
            }
            auto& s = slots[index];
            return s.alive ? &s.resource : nullptr;
        }

        Handle<T> handleAt(uint16_t index) const{
            if(index >= slots.size() || !slots[index].alive){
                return {};
            }
            return { index, slots[index].generation };
        }

        void remove(Handle<T> h){
            if(get(h)){
                slots[h.index].alive = false;
                slots[h.index].generation++;
                freeList.push_back(h.index);
            }
        }

        void clear(){
            slots.clear();
            freeList.clear();
        }

        private:
        struct Slot{
            T resource;
            uint16_t generation;
            bool alive;
        };
        std::vector<Slot> slots;
        std::vector<uint16_t> freeList;
    };
}