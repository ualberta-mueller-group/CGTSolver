#ifndef H_HASHMAP
#define H_HASHMAP

#include <cstdlib>
#include <cstdint>
#include <cstring>

typedef uint64_t    Entry;

class HashMap
{
public:
    HashMap(uint64_t count, uint64_t entry_size);
    ~HashMap() { std::free(m_pool); };

    Entry get(uint64_t idx);
    void set(uint64_t idx, Entry entry);

private:
    unsigned char* m_pool;

    uint64_t m_capacity;    // # of entries
    uint64_t m_entry_size;
};

inline HashMap::HashMap(uint64_t count, uint64_t entry_size)
    : m_capacity(count), m_entry_size(entry_size)
{
    m_pool = (unsigned char*)std::calloc(m_capacity, m_entry_size);
}

inline Entry HashMap::get(uint64_t idx)
{
    unsigned char* ptr = m_pool + idx * m_entry_size;
    Entry entry = 0;
    std::memcpy(&entry, ptr, m_entry_size);
    return entry;
}

inline void HashMap::set(uint64_t idx, Entry entry)
{
    unsigned char* ptr = m_pool + idx * m_entry_size;
    std::memcpy(ptr, &entry, m_entry_size);
    return;
}

#endif
