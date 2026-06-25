#ifndef CUCKOO_HASH_TABLE_HPP
#define CUCKOO_HASH_TABLE_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace shaykhraziev
{
  namespace detail
  {
    const std::size_t DEFAULT_CUCKOO_BUCKETS = 8;
    const std::size_t DEFAULT_CUCKOO_BUCKET_SIZE = 4;
    const std::size_t CUCKOO_REHASH_ATTEMPTS = 4;

    inline std::size_t mixCuckooHash(std::size_t value, std::size_t seed) noexcept
    {
      value ^= seed + 0x9e3779b97f4a7c15ULL + (value << 6) + (value >> 2);
      value ^= value >> 33;
      value *= 0xff51afd7ed558ccdULL;
      value ^= value >> 33;
      value *= 0xc4ceb9fe1a85ec53ULL;
      value ^= value >> 33;
      return value;
    }
  }

  template< class Key, class Value, class Hash, class Equal >
  class CuckooHashTable
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
      iterator(CuckooHashTable* table = nullptr, std::size_t index = 0):
        table_(table),
        index_(index)
      {
        skipEmpty();
      }

      Entry& operator*() const
      {
        return table_->entries_[index_];
      }

      Entry* operator->() const
      {
        return &table_->entries_[index_];
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
      CuckooHashTable* table_;
      std::size_t index_;

      void skipEmpty()
      {
        if (!table_)
        {
          return;
        }
        while (index_ < table_->physicalCapacity() && !table_->entries_[index_].occupied)
        {
          ++index_;
        }
      }

      friend class const_iterator;
    };

    class const_iterator
    {
    public:
      const_iterator(const CuckooHashTable* table = nullptr, std::size_t index = 0):
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
        return table_->entries_[index_];
      }

      const Entry* operator->() const
      {
        return &table_->entries_[index_];
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
      const CuckooHashTable* table_;
      std::size_t index_;

      void skipEmpty()
      {
        if (!table_)
        {
          return;
        }
        while (index_ < table_->physicalCapacity() && !table_->entries_[index_].occupied)
        {
          ++index_;
        }
      }

      friend class CuckooHashTable;
    };

    CuckooHashTable():
      entries_(nullptr),
      bucketCount_(detail::DEFAULT_CUCKOO_BUCKETS),
      bucketSize_(detail::DEFAULT_CUCKOO_BUCKET_SIZE),
      size_(0),
      firstSeed_(0x46694355434b3031ULL),
      secondSeed_(0x46694355434b3032ULL),
      hash_(),
      equal_()
    {
      entries_ = new Entry[physicalCapacity()];
    }

    explicit CuckooHashTable(std::size_t buckets, std::size_t bucketSize = detail::DEFAULT_CUCKOO_BUCKET_SIZE):
      entries_(nullptr),
      bucketCount_(buckets),
      bucketSize_(bucketSize),
      size_(0),
      firstSeed_(0x46694355434b3031ULL),
      secondSeed_(0x46694355434b3032ULL),
      hash_(),
      equal_()
    {
      if (bucketCount_ == 0 || bucketSize_ == 0)
      {
        throw std::invalid_argument("invalid hash table capacity");
      }
      entries_ = new Entry[physicalCapacity()];
    }

    ~CuckooHashTable()
    {
      delete[] entries_;
    }

    CuckooHashTable(const CuckooHashTable& other):
      entries_(new Entry[other.physicalCapacity()]),
      bucketCount_(other.bucketCount_),
      bucketSize_(other.bucketSize_),
      size_(other.size_),
      firstSeed_(other.firstSeed_),
      secondSeed_(other.secondSeed_),
      hash_(other.hash_),
      equal_(other.equal_)
    {
      for (std::size_t i = 0; i < physicalCapacity(); ++i)
      {
        entries_[i] = other.entries_[i];
      }
    }

    CuckooHashTable(CuckooHashTable&& other) noexcept:
      entries_(other.entries_),
      bucketCount_(other.bucketCount_),
      bucketSize_(other.bucketSize_),
      size_(other.size_),
      firstSeed_(other.firstSeed_),
      secondSeed_(other.secondSeed_),
      hash_(std::move(other.hash_)),
      equal_(std::move(other.equal_))
    {
      other.entries_ = nullptr;
      other.bucketCount_ = 0;
      other.bucketSize_ = 0;
      other.size_ = 0;
      other.firstSeed_ = 0;
      other.secondSeed_ = 0;
    }

    CuckooHashTable& operator=(const CuckooHashTable& other)
    {
      if (this != &other)
      {
        CuckooHashTable tmp(other);
        swap(tmp);
      }
      return *this;
    }

    CuckooHashTable& operator=(CuckooHashTable&& other) noexcept
    {
      if (this != &other)
      {
        delete[] entries_;
        entries_ = other.entries_;
        bucketCount_ = other.bucketCount_;
        bucketSize_ = other.bucketSize_;
        size_ = other.size_;
        firstSeed_ = other.firstSeed_;
        secondSeed_ = other.secondSeed_;
        hash_ = std::move(other.hash_);
        equal_ = std::move(other.equal_);
        other.entries_ = nullptr;
        other.bucketCount_ = 0;
        other.bucketSize_ = 0;
        other.size_ = 0;
        other.firstSeed_ = 0;
        other.secondSeed_ = 0;
      }
      return *this;
    }

    void swap(CuckooHashTable& other) noexcept
    {
      Entry* tmpEntries = entries_;
      entries_ = other.entries_;
      other.entries_ = tmpEntries;

      std::size_t tmpBucketCount = bucketCount_;
      bucketCount_ = other.bucketCount_;
      other.bucketCount_ = tmpBucketCount;

      std::size_t tmpBucketSize = bucketSize_;
      bucketSize_ = other.bucketSize_;
      other.bucketSize_ = tmpBucketSize;

      std::size_t tmpSize = size_;
      size_ = other.size_;
      other.size_ = tmpSize;

      std::size_t tmpFirstSeed = firstSeed_;
      firstSeed_ = other.firstSeed_;
      other.firstSeed_ = tmpFirstSeed;

      std::size_t tmpSecondSeed = secondSeed_;
      secondSeed_ = other.secondSeed_;
      other.secondSeed_ = tmpSecondSeed;
    }

    void clear() noexcept
    {
      for (std::size_t i = 0; i < physicalCapacity(); ++i)
      {
        entries_[i].occupied = false;
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
      return bucketCount_;
    }

    std::size_t bucketSize() const noexcept
    {
      return bucketSize_;
    }

    std::size_t capacity() const noexcept
    {
      return 2 * bucketCount_ * bucketSize_;
    }

    iterator begin() noexcept
    {
      return iterator(this, 0);
    }

    iterator end() noexcept
    {
      return iterator(this, physicalCapacity());
    }

    const_iterator begin() const noexcept
    {
      return const_iterator(this, 0);
    }

    const_iterator end() const noexcept
    {
      return const_iterator(this, physicalCapacity());
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
      std::vector< Entry > values = collectEntries();
      Entry entry;
      entry.key = key;
      entry.value = value;
      entry.occupied = true;
      values.push_back(entry);
      rebuildFor(values, bucketCount_);
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

    void rehash(std::size_t buckets)
    {
      if (buckets == 0)
      {
        throw std::invalid_argument("invalid hash table capacity");
      }
      rebuildFor(collectEntries(), buckets);
    }

  private:
    Entry* entries_;
    std::size_t bucketCount_;
    std::size_t bucketSize_;
    std::size_t size_;
    std::size_t firstSeed_;
    std::size_t secondSeed_;
    Hash hash_;
    Equal equal_;

    std::size_t tableCapacity() const noexcept
    {
      return bucketCount_ * bucketSize_;
    }

    std::size_t physicalCapacity() const noexcept
    {
      return 2 * tableCapacity();
    }

    std::size_t tableOffset(std::size_t table) const noexcept
    {
      return table * tableCapacity();
    }

    std::size_t bucketStart(std::size_t table, std::size_t bucket) const noexcept
    {
      return tableOffset(table) + bucket * bucketSize_;
    }

    std::size_t bucketFor(const Key& key, std::size_t table) const
    {
      const std::size_t seed = table == 0 ? firstSeed_ : secondSeed_;
      return detail::mixCuckooHash(hash_(key), seed) % bucketCount_;
    }

    Entry* findInBucket(const Key& key, std::size_t table, std::size_t bucket) noexcept
    {
      const std::size_t start = bucketStart(table, bucket);
      for (std::size_t i = 0; i < bucketSize_; ++i)
      {
        Entry& entry = entries_[start + i];
        if (entry.occupied && equal_(entry.key, key))
        {
          return &entry;
        }
      }
      return nullptr;
    }

    const Entry* findInBucket(const Key& key, std::size_t table, std::size_t bucket) const noexcept
    {
      const std::size_t start = bucketStart(table, bucket);
      for (std::size_t i = 0; i < bucketSize_; ++i)
      {
        const Entry& entry = entries_[start + i];
        if (entry.occupied && equal_(entry.key, key))
        {
          return &entry;
        }
      }
      return nullptr;
    }

    Entry* findEntry(const Key& key) noexcept
    {
      if (!entries_ || bucketCount_ == 0)
      {
        return nullptr;
      }
      Entry* entry = findInBucket(key, 0, bucketFor(key, 0));
      return entry ? entry : findInBucket(key, 1, bucketFor(key, 1));
    }

    const Entry* findEntry(const Key& key) const noexcept
    {
      if (!entries_ || bucketCount_ == 0)
      {
        return nullptr;
      }
      const Entry* entry = findInBucket(key, 0, bucketFor(key, 0));
      return entry ? entry : findInBucket(key, 1, bucketFor(key, 1));
    }

    bool placeInFreeSlot(const Entry& value, std::size_t table)
    {
      const std::size_t start = bucketStart(table, bucketFor(value.key, table));
      for (std::size_t i = 0; i < bucketSize_; ++i)
      {
        Entry& entry = entries_[start + i];
        if (!entry.occupied)
        {
          entry = value;
          entry.occupied = true;
          ++size_;
          return true;
        }
      }
      return false;
    }

    bool insertDirect(const Entry& value)
    {
      return placeInFreeSlot(value, 0) || placeInFreeSlot(value, 1);
    }

    bool insertWave(const Entry& value, std::size_t startTable)
    {
      Entry current = value;
      std::size_t table = startTable;
      const std::size_t maxKickCount = physicalCapacity() * 2 + bucketSize_;
      for (std::size_t kick = 0; kick < maxKickCount; ++kick)
      {
        if (placeInFreeSlot(current, table))
        {
          return true;
        }

        const std::size_t start = bucketStart(table, bucketFor(current.key, table));
        const std::size_t victimIndex = start + (kick % bucketSize_);
        Entry victim = entries_[victimIndex];
        entries_[victimIndex] = current;
        entries_[victimIndex].occupied = true;
        current = victim;
        table = 1 - table;
      }
      return false;
    }

    bool insertWithWaves(const Entry& value)
    {
      if (insertDirect(value))
      {
        return true;
      }

      CuckooHashTable firstWave(*this);
      if (firstWave.insertWave(value, 0))
      {
        swap(firstWave);
        return true;
      }

      CuckooHashTable secondWave(*this);
      if (secondWave.insertWave(value, 1))
      {
        swap(secondWave);
        return true;
      }
      return false;
    }

    bool insertAll(const std::vector< Entry >& values)
    {
      clear();
      for (std::size_t i = 0; i < values.size(); ++i)
      {
        if (!insertWithWaves(values[i]))
        {
          return false;
        }
      }
      return true;
    }

    std::vector< Entry > collectEntries() const
    {
      std::vector< Entry > values;
      values.reserve(size_);
      for (const_iterator it = begin(); it != end(); ++it)
      {
        values.push_back(*it);
      }
      return values;
    }

    void advanceSeeds() noexcept
    {
      firstSeed_ = detail::mixCuckooHash(firstSeed_, 0x46694355434b5241ULL);
      secondSeed_ = detail::mixCuckooHash(secondSeed_, 0x46694355434b5242ULL);
      if (firstSeed_ == secondSeed_)
      {
        secondSeed_ = detail::mixCuckooHash(secondSeed_, 0x46694355434b5243ULL);
      }
    }

    void rebuildFor(const std::vector< Entry >& values, std::size_t buckets)
    {
      if (buckets == 0)
      {
        throw std::invalid_argument("invalid hash table capacity");
      }
      std::size_t nextBuckets = buckets;
      while (values.size() > 2 * nextBuckets * bucketSize_)
      {
        nextBuckets *= 2;
      }

      for (;;)
      {
        CuckooHashTable tmp(nextBuckets, bucketSize_);
        tmp.hash_ = hash_;
        tmp.equal_ = equal_;
        tmp.firstSeed_ = detail::mixCuckooHash(firstSeed_, nextBuckets);
        tmp.secondSeed_ = detail::mixCuckooHash(secondSeed_, nextBuckets + 1);

        for (std::size_t attempt = 0; attempt < detail::CUCKOO_REHASH_ATTEMPTS; ++attempt)
        {
          if (tmp.insertAll(values))
          {
            swap(tmp);
            return;
          }
          tmp.advanceSeeds();
        }
        nextBuckets *= 2;
      }
    }
  };
}

#endif
