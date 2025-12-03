#pragma once

#include "List_header.hpp"
#include "MyArray.hpp"


template<typename Key, typename Value>
struct OTNode{
    Key key;
    Value value;
    OTNode() = default;
    OTNode(Key k, Value v) : key(k), value(v){}
};


template<typename Key, typename Value>
class OpenTable{
private:
    MyArray<OTNode<Key, Value>> buckets{10};
    size_t countItems = 0;
    size_t bucketsThersSmtng = 0;
    
    size_t Hash(int key) const {
        size_t hash = 0;
        while (key > 0) {
            hash ^= (key & 0xFF); // берём младший байт и XOR с хэшем
            key >>= 8;            // сдвигаем на байт вправо
        }
        return hash % buckets.get_cap();
    }
    
    size_t Hash(const string& key) const {
        size_t sum = 0;
        for (char c : key) {
            sum += static_cast<uint8_t>(c);
        }

        uint32_t result = (sum % (buckets.get_cap() - 1)) + 1;

        //Делаем результат нечётным, если размер таблицы чётный
        if (buckets.get_cap() % 2 == 0 && result % 2 == 0) {
            result++;
        }

        return result;
    }
    
    size_t get_bucket_index(const Key& key) const {
        return Hash(key) % buckets.get_cap();
    }
    
    void rehash() {
        MyArray<OTNode<Key, Value>> oldBuckets = buckets;
        buckets = MyArray<OTNode<Key, Value>>(oldBuckets.get_cap() * 2);
        countItems = 0;

        for (int i = 0; i < oldBuckets.get_cap(); i++) {
            Key key = oldBuckets[i].key;
            Value val = oldBuckets[i].value;
            if (key != Key()) {
                insert(key, val); // повторно вставляем все существующие элементы
            }
        }
    }
    
public:
    OpenTable(int initcap) : buckets(MyArray<OTNode<Key, Value>>(initcap)) {};
    OpenTable() : buckets(MyArray<OTNode<Key, Value>>(10)) {};
    
    bool find(const Key& key) const {
        if (countItems == 0) return false;
        
        size_t index = get_bucket_index(key);
        size_t start = index;
        
        while (buckets[index].key != Key()){
            if (buckets[index].key == key) return true;
            index = (index + 1) % buckets.get_cap();
            if (index == start) break;
        }
        
        return false;
    }
    
    void insert(const Key& key, const Value& val) {
        if (find(key)) return;  // уже есть

        size_t index = get_bucket_index(key);
        size_t start = index;

        // Линейное пробирование
        while (buckets[index].key != Key()) {
            index = (index + 1) % buckets.get_cap();
            if (index == start) {
                // Таблица переполнена — расширяем
                rehash();
                insert(key, val);
                return;
            }
        }
        
        if (buckets[index].key == Key()){
            bucketsThersSmtng++;
        }
        buckets[index].key = key;
        buckets[index].value = val;
        countItems++;

        // если загрузка > 0.75 — увеличиваем таблицу
        if (countItems * 4 > buckets.get_cap() * 3) {
            rehash();
        }
        
        if (static_cast<double>(bucketsThersSmtng) / static_cast<double>(buckets.get_cap()) >= 0.9){
            rehash();
        }
    }
    
    void erase(const Key& key) {
        if (countItems == 0) return;
        
        size_t index = get_bucket_index(key);
        
        while (buckets[index].key != key){
            index = (index + 1) % buckets.get_cap();
        }
        
        buckets[index].key = Key();
        buckets[index].value = Value();
        countItems--;
    }
    
    
    size_t size() const {
        return countItems;
    }
    
    bool empty() const {
        return countItems == 0;
    }
    
    int get_cap() const {
        return buckets.get_cap();
    }
    
    Key get_key(int i) const {
        return buckets[i].key;
    }
    
    Value get_value(Key k) const {
        size_t index = get_bucket_index(k);
        while (buckets[index].key != k){
            index = (index + 1) % buckets.get_cap();
        }
        
        return buckets[index].value;
        
    }
    
    void SPRINT(){
        for (int i = 0; i < buckets.get_cap(); i++){
            if (buckets[i].key != Key()){
                cout << buckets[i].key << ' ' << buckets[i].value << endl;
            }
        }
        cout << endl;
    }
};


