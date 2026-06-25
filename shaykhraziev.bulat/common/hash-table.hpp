#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace shaykhraziev
{
  namespace detail
  {
    const std::size_t DEFAULT_BUCKET_SIZE = 4;
  }

  template< class Key, class Value, class Hash, class Equal >
  class HashTable
  {
  public:
    struct Entry
    {
      Key key;
      Value value;
      bool occupied;

      Entry():
        key(),
        value(),
        occupied(false)
      {}
    };

    class const_iterator;

    class iterator
    {
    public:
      iterator(HashTable* table = nullptr, std::size_t index = 0):
        table_(table),
        index_(index)
      {
        skipEmpty();
      }

      Entry& operator*() const
      {
        return table_->buckets_[index_];
      }

      Entry* operator->() const
      {
        return &table_->buckets_[index_];
      }

      iterator& operator++()
      {
        ++index_;
        skipEmpty();
        return *this;
      }

      bool operator==(const iterator& other) const
      {
        return table_ == other.table_ && index_ == other.index_;
      }

      bool operator!=(const iterator& other) const
      {
        return !(*this == other);
      }

    private:
      HashTable* table_;
      std::size_t index_;

      void skipEmpty()
      {
        if (!table_)
        {
          return;
        }
        while (index_ < table_->capacity() && !table_->buckets_[index_].occupied)
        {
          ++index_;
        }
      }

      friend class const_iterator;
    };

    class const_iterator
    {
    public:
      const_iterator(const HashTable* table = nullptr, std::size_t index = 0):
        table_(table),
        index_(index)
      {
        skipEmpty();
      }

      const_iterator(const iterator& other):
        table_(other.table_),
        index_(other.index_)
      {}

      const Entry& operator*() const
      {
        return table_->buckets_[index_];
      }

      const Entry* operator->() const
      {
        return &table_->buckets_[index_];
      }

      const_iterator& operator++()
      {
        ++index_;
        skipEmpty();
        return *this;
      }

      bool operator==(const const_iterator& other) const
      {
        return table_ == other.table_ && index_ == other.index_;
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(*this == other);
      }

    private:
      const HashTable* table_;
      std::size_t index_;

      void skipEmpty()
      {
        if (!table_)
        {
          return;
        }
        while (index_ < table_->capacity() && !table_->buckets_[index_].occupied)
        {
          ++index_;
        }
      }

      friend class HashTable;
    };

    explicit HashTable(std::size_t slots, std::size_t bucketSize = detail::DEFAULT_BUCKET_SIZE):
      buckets_(nullptr),
      slots_(slots),
      bucketSize_(bucketSize),
      size_(0),
      hash_(),
      equal_()
    {
      if (slots_ == 0 || bucketSize_ == 0)
      {
        throw std::invalid_argument("invalid hash table capacity");
      }
      buckets_ = new Entry[capacity()];
    }

    ~HashTable()
    {
      delete[] buckets_;
    }

    HashTable(const HashTable& other):
      buckets_(new Entry[other.capacity()]),
      slots_(other.slots_),
      bucketSize_(other.bucketSize_),
      size_(other.size_),
      hash_(other.hash_),
      equal_(other.equal_)
    {
      for (std::size_t i = 0; i < capacity(); ++i)
      {
        buckets_[i] = other.buckets_[i];
      }
    }

    HashTable(HashTable&& other) noexcept:
      buckets_(other.buckets_),
      slots_(other.slots_),
      bucketSize_(other.bucketSize_),
      size_(other.size_),
      hash_(std::move(other.hash_)),
      equal_(std::move(other.equal_))
    {
      other.buckets_ = nullptr;
      other.slots_ = 0;
      other.bucketSize_ = 0;
      other.size_ = 0;
    }

    HashTable& operator=(const HashTable& other)
    {
      if (this != &other)
      {
        HashTable tmp(other);
        swap(tmp);
      }
      return *this;
    }

    HashTable& operator=(HashTable&& other) noexcept
    {
      if (this != &other)
      {
        delete[] buckets_;
        buckets_ = other.buckets_;
        slots_ = other.slots_;
        bucketSize_ = other.bucketSize_;
        size_ = other.size_;
        hash_ = std::move(other.hash_);
        equal_ = std::move(other.equal_);
        other.buckets_ = nullptr;
        other.slots_ = 0;
        other.bucketSize_ = 0;
        other.size_ = 0;
      }
      return *this;
    }

    void swap(HashTable& other) noexcept
    {
      Entry* tmpBuckets = buckets_;
      buckets_ = other.buckets_;
      other.buckets_ = tmpBuckets;

      std::size_t tmpSlots = slots_;
      slots_ = other.slots_;
      other.slots_ = tmpSlots;

      std::size_t tmpBucketSize = bucketSize_;
      bucketSize_ = other.bucketSize_;
      other.bucketSize_ = tmpBucketSize;

      std::size_t tmpSize = size_;
      size_ = other.size_;
      other.size_ = tmpSize;
    }

    void clear() noexcept
    {
      for (std::size_t i = 0; i < capacity(); ++i)
      {
        buckets_[i].occupied = false;
      }
      size_ = 0;
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }

    std::size_t size() const noexcept
    {
      return size_;
    }

    std::size_t slots() const noexcept
    {
      return slots_;
    }

    std::size_t bucketSize() const noexcept
    {
      return bucketSize_;
    }

    std::size_t capacity() const noexcept
    {
      return slots_ * bucketSize_;
    }

    iterator begin() noexcept
    {
      return iterator(this, 0);
    }

    iterator end() noexcept
    {
      return iterator(this, capacity());
    }

    const_iterator begin() const noexcept
    {
      return const_iterator(this, 0);
    }

    const_iterator end() const noexcept
    {
      return const_iterator(this, capacity());
    }

    const_iterator cbegin() const noexcept
    {
      return begin();
    }

    const_iterator cend() const noexcept
    {
      return end();
    }

    bool add(const Key& key, const Value& value)
    {
      if (findEntry(key))
      {
        throw std::logic_error("duplicate key");
      }

      Entry* entry = findFreeEntry(key);
      if (!entry)
      {
        throw std::overflow_error("hash table is full");
      }
      entry->key = key;
      entry->value = value;
      entry->occupied = true;
      ++size_;
      return true;
    }

    bool set(const Key& key, const Value& value)
    {
      Entry* entry = findEntry(key);
      if (entry)
      {
        entry->value = value;
        return false;
      }
      add(key, value);
      return true;
    }

    bool drop(const Key& key)
    {
      Entry* entry = findEntry(key);
      if (!entry)
      {
        return false;
      }
      entry->occupied = false;
      --size_;
      return true;
    }

    bool has(const Key& key) const
    {
      return findEntry(key) != nullptr;
    }

    Value* find(const Key& key)
    {
      Entry* entry = findEntry(key);
      return entry ? &entry->value : nullptr;
    }

    const Value* find(const Key& key) const
    {
      const Entry* entry = findEntry(key);
      return entry ? &entry->value : nullptr;
    }

    Value& get(const Key& key)
    {
      Entry* entry = findEntry(key);
      if (!entry)
      {
        throw std::out_of_range("missing key");
      }
      return entry->value;
    }

    const Value& get(const Key& key) const
    {
      const Entry* entry = findEntry(key);
      if (!entry)
      {
        throw std::out_of_range("missing key");
      }
      return entry->value;
    }

    void rehash(std::size_t slots)
    {
      if (slots == 0)
      {
        throw std::invalid_argument("invalid hash table capacity");
      }
      HashTable tmp(slots, bucketSize_);
      for (const_iterator it = begin(); it != end(); ++it)
      {
        tmp.add(it->key, it->value);
      }
      swap(tmp);
    }

  private:
    Entry* buckets_;
    std::size_t slots_;
    std::size_t bucketSize_;
    std::size_t size_;
    Hash hash_;
    Equal equal_;

    std::size_t firstSlot(const Key& key) const
    {
      return hash_(key) % slots_;
    }

    std::size_t flatIndex(std::size_t slot, std::size_t index) const
    {
      return slot * bucketSize_ + index;
    }

    Entry* findEntry(const Key& key) noexcept
    {
      if (slots_ == 0)
      {
        return nullptr;
      }
      std::size_t start = firstSlot(key);
      for (std::size_t offset = 0; offset < slots_; ++offset)
      {
        std::size_t slot = (start + offset) % slots_;
        for (std::size_t i = 0; i < bucketSize_; ++i)
        {
          Entry& entry = buckets_[flatIndex(slot, i)];
          if (entry.occupied && equal_(entry.key, key))
          {
            return &entry;
          }
        }
      }
      return nullptr;
    }

    const Entry* findEntry(const Key& key) const noexcept
    {
      if (slots_ == 0)
      {
        return nullptr;
      }
      std::size_t start = firstSlot(key);
      for (std::size_t offset = 0; offset < slots_; ++offset)
      {
        std::size_t slot = (start + offset) % slots_;
        for (std::size_t i = 0; i < bucketSize_; ++i)
        {
          const Entry& entry = buckets_[flatIndex(slot, i)];
          if (entry.occupied && equal_(entry.key, key))
          {
            return &entry;
          }
        }
      }
      return nullptr;
    }

    Entry* findFreeEntry(const Key& key) noexcept
    {
      std::size_t start = firstSlot(key);
      for (std::size_t offset = 0; offset < slots_; ++offset)
      {
        std::size_t slot = (start + offset) % slots_;
        for (std::size_t i = 0; i < bucketSize_; ++i)
        {
          Entry& entry = buckets_[flatIndex(slot, i)];
          if (!entry.occupied)
          {
            return &entry;
          }
        }
      }
      return nullptr;
    }
  };
}

#endif
