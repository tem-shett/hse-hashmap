#pragma once

#include <cstdint>
#include <vector>
#include <list>
#include <stdexcept>
#include <iostream>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    // CONSTS
    static const uint32_t PRIME = 1791791791;
    static const uint32_t MAX_NB_SIZE = 27; // max size of bucket's neighbourhood
    static constexpr double MAX_LOAD_FACTOR = 0.87;
    static const size_t SIZE_GROWTH_FACTOR = 2;
    static const size_t START_CAPACITY = 3;
    static const size_t START_NB_SIZE = 2;

    //SIZE VARIABLES
    size_t nb_size; // size of bucket's neighbourhood
    size_t capacity;

    Hash hash_func_;

    class KeyValue;

    class Bucket;

    class OverflowElement;

    Bucket* buckets_;
    std::list<KeyValue> all_elements_;
    std::list<OverflowElement> overflow_list_;

    class KeyValue {
    public:
        std::pair<const KeyType, ValueType> map_pair;
        size_t hash;

        KeyValue(const KeyType key, ValueType value, size_t hash_) : map_pair({key, value}), hash(hash_) {}
    };

    class Bucket {
    public:
        uint32_t mask;
        typename std::list<KeyValue>::iterator key_value_iter;

        Bucket() : mask(0), key_value_iter(typename std::list<KeyValue>::iterator()) {}
    };

    class OverflowElement {
    public:
        typename std::list<KeyValue>::iterator key_value_iter;

        explicit OverflowElement(typename std::list<KeyValue>::iterator key_val_iter) : key_value_iter(key_val_iter) {}
    };


public:
    // CONSTRUCTORS
    explicit HashMap(Hash hash_func = Hash());

    template<class Iterator>
    HashMap(Iterator iter_begin, Iterator iter_end, Hash hash_func = Hash());

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init_list, Hash hash_func = Hash());

    HashMap(const HashMap& hash_map);

    HashMap& operator=(const HashMap& hash_map);

public:
    ~HashMap();

private:
    size_t key_hash(KeyType key) const;

    [[nodiscard]] size_t add_indexes(size_t index1, size_t index2) const;

    [[nodiscard]] size_t sub_indexes(size_t index1, size_t index2) const;

public:
    class iterator {
    public:
        explicit iterator(typename std::list<KeyValue>::iterator iter = typename std::list<KeyValue>::iterator());

        std::pair<const KeyType, ValueType>& operator*() const;

        std::pair<const KeyType, ValueType>* operator->() const;

        iterator& operator++();

        iterator operator++(int);

        bool operator!=(const iterator other);

        bool operator==(const iterator other);

    private:
        typename std::list<KeyValue>::iterator key_value_iter_;
    };

    class const_iterator {
    public:
        explicit const_iterator(
                typename std::list<KeyValue>::const_iterator iter = typename std::list<KeyValue>::const_iterator());

        const std::pair<const KeyType, ValueType>& operator*() const;

        const std::pair<const KeyType, ValueType>* operator->() const;

        const_iterator& operator++();

        const_iterator operator++(int);

        bool operator!=(const const_iterator other);

        bool operator==(const const_iterator other);

    private:
        typename std::list<KeyValue>::const_iterator key_value_iter_;
    };

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

public:
    [[nodiscard]] size_t size() const;

    [[nodiscard]] bool empty() const;

    Hash hash_function() const;

private:
    class FindResult {
    public:
        typename std::list<KeyValue>::iterator key_value_iter;
        size_t bucket_index;
        typename std::list<OverflowElement>::iterator overflow_iter;

        FindResult(typename std::list<KeyValue>::iterator key_val_iter, size_t index,
                   typename std::list<OverflowElement>::iterator over_iter) : key_value_iter(key_val_iter),
                                                                              bucket_index(index),
                                                                              overflow_iter(over_iter) {}
    };

    class ConstFindResult {
    public:
        typename std::list<KeyValue>::const_iterator key_value_iter;
        size_t bucket_index;
        typename std::list<OverflowElement>::const_iterator overflow_iter;

        ConstFindResult(typename std::list<KeyValue>::const_iterator key_val_iter, size_t index,
                        typename std::list<OverflowElement>::const_iterator over_iter) : key_value_iter(key_val_iter),
                                                                                         bucket_index(index),
                                                                                         overflow_iter(over_iter) {}
    };

    size_t find_within_neighbourhood(KeyType& key, size_t bucket_index) const;

    FindResult find_(KeyType key, size_t hash);

    ConstFindResult const_find_(KeyType key, size_t hash) const;

public:
    iterator find(KeyType key);

    const_iterator find(KeyType key) const;

public:
    void erase(KeyType key);

private:
    void insert_(typename std::list<KeyValue>::iterator iter);

private:
    [[nodiscard]] bool is_overload() const;

    void size_increase();

    void full_rebuild();

public:
    void insert(std::pair<KeyType, ValueType> insert_pair);

public:
    ValueType& operator[](const KeyType key);

    const ValueType& at(const KeyType key) const;

public:
    void clear();
};

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(Hash hash_func) : nb_size(START_NB_SIZE), capacity(START_CAPACITY),
                                                             hash_func_(hash_func) {
  buckets_ = new Bucket[capacity];
}

template<class KeyType, class ValueType, class Hash>
template<class Iterator>
HashMap<KeyType, ValueType, Hash>::HashMap(Iterator iter_begin, Iterator iter_end, Hash hash_func): HashMap(hash_func) {
  for (; iter_begin != iter_end; ++iter_begin) {
    insert(*iter_begin);
  }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init_list,
                                           Hash hash_func) : HashMap(
        hash_func) {
  for (auto key_value : init_list) {
    insert(key_value);
  }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const HashMap& hash_map) : HashMap() {
  *this = hash_map;
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>& HashMap<KeyType, ValueType, Hash>::operator=(const HashMap& hash_map) {
  if (this == &hash_map) {
    return *this;
  }
  capacity = hash_map.capacity;
  hash_func_ = hash_map.hash_func_;
  all_elements_.clear();
  overflow_list_.clear();
  delete[] buckets_;
  buckets_ = new Bucket[capacity];
  for (auto key_value : hash_map) {
    insert(key_value);
  }
  return *this;
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::~HashMap() {
  overflow_list_.clear();
  all_elements_.clear();
  delete[] buckets_;
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::key_hash(KeyType key) const {
  return static_cast<uint64_t>(hash_func_(key) ^ 17969228) * PRIME % capacity;
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::add_indexes(size_t index1, size_t index2) const {
  size_t res = index1 + index2;
  return res >= capacity ? (res - capacity) : res;
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::sub_indexes(size_t index1, size_t index2) const {
  return index1 < index2 ? (index1 + capacity - index2) : (index1 - index2);
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::iterator::iterator(typename std::list<KeyValue>::iterator iter)
        : key_value_iter_(iter) {}

template<class KeyType, class ValueType, class Hash>
std::pair<const KeyType, ValueType>& HashMap<KeyType, ValueType, Hash>::iterator::operator*() const {
  return key_value_iter_->map_pair;
}

template<class KeyType, class ValueType, class Hash>
std::pair<const KeyType, ValueType>* HashMap<KeyType, ValueType, Hash>::iterator::operator->() const {
  return &key_value_iter_->map_pair;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator& HashMap<KeyType, ValueType, Hash>::iterator::operator++() {
  ++key_value_iter_;
  return *this;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::iterator::operator++(int) {
  auto prev_iter(*this);
  ++(*this);
  return prev_iter;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::iterator::operator!=(const iterator other) {
  return key_value_iter_ != other.key_value_iter_;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::iterator::operator==(const iterator other) {
  return key_value_iter_ == other.key_value_iter_;
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::const_iterator::const_iterator(
        typename std::list<KeyValue>::const_iterator iter)
        : key_value_iter_(iter) {}

template<class KeyType, class ValueType, class Hash>
const std::pair<const KeyType, ValueType>& HashMap<KeyType, ValueType, Hash>::const_iterator::operator*() const {
  return key_value_iter_->map_pair;
}

template<class KeyType, class ValueType, class Hash>
const std::pair<const KeyType, ValueType>* HashMap<KeyType, ValueType, Hash>::const_iterator::operator->() const {
  return &key_value_iter_->map_pair;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator&
HashMap<KeyType, ValueType, Hash>::const_iterator::operator++() {
  ++key_value_iter_;
  return *this;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator
HashMap<KeyType, ValueType, Hash>::const_iterator::operator++(int) {
  auto prev_iter(*this);
  ++(*this);
  return prev_iter;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::const_iterator::operator!=(const const_iterator other) {
  return key_value_iter_ != other.key_value_iter_;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::const_iterator::operator==(const const_iterator other) {
  return key_value_iter_ == other.key_value_iter_;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin() {
  return iterator(all_elements_.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::begin() const {
  return const_iterator(all_elements_.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::end() {
  return iterator(all_elements_.end());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::end() const {
  return const_iterator(all_elements_.end());
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::size() const {
  return all_elements_.size();
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
  return all_elements_.empty();
}

template<class KeyType, class ValueType, class Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
  return hash_func_;
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::find_within_neighbourhood(KeyType& key, size_t bucket_index) const {
  auto bucket_mask = buckets_[bucket_index].mask & ((1 << nb_size) - 1);
  for (size_t bit = 0; bit < nb_size && bucket_mask != 0; ++bit) {
    size_t index = add_indexes(bucket_index, bit);
    if (bucket_mask & (1 << bit)) {
      bucket_mask ^= (1 << bit);
      if (buckets_[index].key_value_iter->map_pair.first == key) {
        return index;
      }
    }
  }
  return capacity;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::FindResult
HashMap<KeyType, ValueType, Hash>::find_(KeyType key, size_t hash) {
  auto bucket_index = hash;
  auto neigh_index = find_within_neighbourhood(key, bucket_index);
  if (neigh_index < capacity) {
    return {buckets_[neigh_index].key_value_iter, neigh_index, overflow_list_.end()};
  }
  auto bucket_mask = buckets_[bucket_index].mask;
  if (bucket_mask & (1 << (nb_size))) {
    for (auto overflow_iter = overflow_list_.begin(); overflow_iter != overflow_list_.end(); ++overflow_iter) {
      if (overflow_iter->key_value_iter->hash == hash &&
          overflow_iter->key_value_iter->map_pair.first == key) {
        return {overflow_iter->key_value_iter, capacity, overflow_iter};
      }
    }
  }
  return {all_elements_.end(), capacity, overflow_list_.end()};
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::ConstFindResult
HashMap<KeyType, ValueType, Hash>::const_find_(KeyType key, size_t hash) const {
  auto bucket_index = hash;
  auto neigh_index = find_within_neighbourhood(key, bucket_index);
  if (neigh_index < capacity) {
    return {buckets_[neigh_index].key_value_iter, neigh_index, overflow_list_.end()};
  }
  auto bucket_mask = buckets_[bucket_index].mask;
  if (bucket_mask & (1 << (nb_size))) {
    for (auto overflow_iter = overflow_list_.begin(); overflow_iter != overflow_list_.end(); ++overflow_iter) {
      if (overflow_iter->key_value_iter->hash == hash &&
          overflow_iter->key_value_iter->map_pair.first == key) {
        return {overflow_iter->key_value_iter, capacity, overflow_iter};
      }
    }
  }
  return {all_elements_.end(), capacity, overflow_list_.end()};
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(KeyType key) {
  return iterator(find_(key, key_hash(key)).key_value_iter);
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::find(KeyType key) const {
  return const_iterator(const_find_(key, key_hash(key)).key_value_iter);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(KeyType key) {
  int hash = key_hash(key);
  auto position = find_(key, hash);
  if (position.key_value_iter == all_elements_.end()) {
    return;
  }
  all_elements_.erase(position.key_value_iter);
  if (position.overflow_iter != overflow_list_.end()) {
    overflow_list_.erase(position.overflow_iter);
    return;
  }
  int index = position.bucket_index;
  buckets_[hash].mask ^= 1 << sub_indexes(index, hash);
  buckets_[index].mask ^= 1 << (nb_size + 1);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert_(typename std::list<KeyValue>::iterator iter) {
  auto hash = iter->hash;
  size_t index = hash;
  while (buckets_[index].mask & (1 << (nb_size + 1))) {
    index = add_indexes(index, 1);
  }
  size_t swap_index = index;
  while (sub_indexes(index, hash) >= nb_size && swap_index != hash) {
    swap_index = sub_indexes(swap_index, 1);
    size_t swap_hash = buckets_[swap_index].key_value_iter->hash;
    size_t dist = sub_indexes(index, swap_hash);
    if (dist < nb_size) {
      buckets_[swap_hash].mask ^= (1 << (sub_indexes(swap_index, swap_hash)));
      buckets_[swap_hash].mask ^= (1 << dist);
      std::swap(buckets_[swap_index].key_value_iter, buckets_[index].key_value_iter);
      buckets_[index].mask ^= (1 << (nb_size + 1));
      buckets_[swap_index].mask ^= (1 << (nb_size + 1));
      index = swap_index;
    }
  }
  if (sub_indexes(index, hash) < nb_size) {
    buckets_[index].key_value_iter = iter;
    buckets_[index].mask ^= (1 << (nb_size + 1));
    buckets_[hash].mask ^= (1 << (sub_indexes(index, hash)));
  } else {
    buckets_[hash].mask |= (1 << nb_size);
    overflow_list_.push_back(OverflowElement(iter));
  }
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::is_overload() const {
  return size() > capacity * MAX_LOAD_FACTOR;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::size_increase() {
  capacity *= SIZE_GROWTH_FACTOR;
  nb_size = std::min(nb_size * SIZE_GROWTH_FACTOR, static_cast<size_t>(MAX_NB_SIZE));
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::full_rebuild() {
  delete[] buckets_;
  buckets_ = new Bucket[capacity];
  overflow_list_.clear();
  for (auto key_value_iter = all_elements_.begin(); key_value_iter != all_elements_.end(); ++key_value_iter) {
    key_value_iter->hash = key_hash(key_value_iter->map_pair.first);
    insert_(key_value_iter);
  }
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(std::pair<KeyType, ValueType> insert_pair) {
  if (is_overload()) {
    size_increase();
    full_rebuild();
  }
  size_t hash = key_hash(insert_pair.first);
  if (find_(insert_pair.first, hash).key_value_iter != all_elements_.end()) {
    return;
  }
  all_elements_.push_back({insert_pair.first, insert_pair.second, hash});
  auto iter = --all_elements_.end();
  insert_(iter);
}

template<class KeyType, class ValueType, class Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](const KeyType key) {
  auto iter = find(key);
  if (iter == end()) {
    insert({key, ValueType()});
    iter = iterator(--all_elements_.end());
  }
  return iter->second;
}

template<class KeyType, class ValueType, class Hash>
const ValueType& HashMap<KeyType, ValueType, Hash>::at(const KeyType key) const {
  auto iter = find(key);
  if (iter == end()) {
    throw std::out_of_range("Key does not belong to the hashmap");
  }
  return iter->second;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
  overflow_list_.clear();
  for (auto key_value : all_elements_) {
    if (buckets_[key_value.hash].mask == 0) {
      continue;
    }
    auto bucket_mask = buckets_[key_value.hash].mask & ((1 << nb_size) - 1);
    for (size_t bit = 0; bit < nb_size && bucket_mask != 0; ++bit) {
      if (bucket_mask & (1 << bit)) {
        bucket_mask ^= (1 << bit);
        size_t index = add_indexes(key_value.hash, bit);
        buckets_[index].mask &= ~(1 << (nb_size + 1));
      }
    }
    buckets_[key_value.hash].mask = 0;
  }
  all_elements_.clear();
}