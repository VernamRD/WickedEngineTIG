#pragma once

#include <memory>
#include <vector>
#include <mutex>

namespace wi::pool
{
    template <typename PolledType>
    struct Pool
    {
    public:
        using value_type = PolledType;

        struct ObjectPoolDeleter
        {
            Pool& pool;

            void operator()(value_type* p) const
            {
                if (p)
                {
                    // Return the object to the pool instead of deleting it
                    pool.Release(p);  
                }
            }
        };

        Pool() = delete;
        Pool(size_t initialSize = 256)
            : m_initialSize(initialSize)
            , m_destroying(false)
        {
            m_pooledItems.reserve(initialSize);
        }

        void Initialize()
        {
            std::scoped_lock lock(m_mtx);
            for (size_t i = 0; i < m_initialSize; ++i)
            {
                MakeItem();
            }
        }

        ~Pool()
        {
            std::scoped_lock lock(m_mtx);
            
            // Prevent Release() from adding items back to pool during destruction
            m_destroying = true;

            // Call OnDeinitialize for all in-use items
            for (auto& weakItem : m_inUseItems)
            {
                if (auto item = weakItem.lock())
                {
                    OnDeinitialize(item.get());
                }
            }
            
            // Call OnDeinitialize for all pooled items
            for (auto& item : m_pooledItems)
            {
                OnDeinitialize(item.get());
            }

            // Clear weak pointers first
            m_inUseItems.clear();
            
            // Clear shared pointers - custom deleter will be called but won't re-add to pool
            m_pooledItems.clear();
        }

        std::shared_ptr<value_type> Acquire()
        {
            std::scoped_lock lock(m_mtx);

            if (m_pooledItems.empty())
            {
                // Expand the pool if empty
                MakeItem();
            }

            std::shared_ptr<value_type> item = std::move(m_pooledItems.back());
            m_pooledItems.pop_back();
            m_inUseItems.push_back(item);

            OnAcquire(item.get());
            
            return item;
        }

        void Release(value_type* item)
        {
            // Don't return to pool if we're destroying
            if (m_destroying)
            {
                delete item;
                return;
            }
            
            
            OnRelease(item);

            std::scoped_lock lock(m_mtx);
            m_pooledItems.push_back(std::shared_ptr<value_type>(item, ObjectPoolDeleter{*this}));
        }

    protected:
        virtual void OnInitialize(value_type* item) {}
        virtual void OnAcquire(value_type* item) {}
        virtual void OnRelease(value_type* item) {}
        virtual void OnDeinitialize(value_type* item) {}

    private:
        void MakeItem()
        {
            auto item = std::shared_ptr<value_type>(new value_type(), ObjectPoolDeleter{*this});
            OnInitialize(item.get());
            
            m_pooledItems.emplace_back(std::move(item));
        }

        size_t m_initialSize;
        bool m_destroying;
        std::mutex m_mtx;
        std::vector<std::shared_ptr<value_type>> m_pooledItems;
        std::vector<std::weak_ptr<value_type>> m_inUseItems;
    };
}  // namespace wi::pool