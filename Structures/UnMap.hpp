#pragma once

#include "List_header.hpp"
#include "MyArray.hpp"
#include "MyFList.hpp"


template<typename Key, typename Value>
struct HashNode{
    Key key;
    Value value;
    HashNode(Key k, Value v) : key(k), value(v){}
    HashNode() = default;
    
    bool operator==(const HashNode& other) const {
        return (key == other.key && value == other.value);
    }
};

template<typename Key, typename Value>
ostream& operator<<(ostream& os,HashNode<Key, Value> hn){
    os << hn.key << ' ' << hn.value << "  ";
    return os;
}

template<typename Key, typename Value>
class UnMap{
private:
    MyArray<ForwardList<HashNode<Key, Value>>> buckets;
    size_t countItems = 0;
    size_t bucketsThersSmtng = 0;
    const double loadFactor = 0.75;
    
    size_t Hash(int key) const {
        size_t hash = 0;
        while (key > 0) {
            hash ^= (key & 0xFF); // берём младший байт и XOR с хэшем
            key >>= 8;            // сдвигаем на байт вправо
        }
        return hash % buckets.capacity;
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
        MyArray<ForwardList<HashNode<Key, Value>>> newBuckets{buckets.get_cap() * 2};
        
        for (int i = 0; i < buckets.get_cap(); i++) {
            for (int j = 0; j < buckets[i].fsize(); j++){
                HashNode<Key, Value> element = buckets[i].FGET_key(j);
                size_t newIndex = Hash(element.key) % newBuckets.get_cap();
                newBuckets[newIndex].FPUSH_back(element);
            }
        }
        buckets.resize();
        for (int i = 0; i < buckets.get_cap(); i++){
            buckets[i] = newBuckets[i];
        }
    }
    
public:
    UnMap(int initcap) : buckets(MyArray<ForwardList<HashNode<Key, Value>>>(initcap)) {};
    UnMap() : buckets(MyArray<ForwardList<HashNode<Key, Value>>>(10)) {};

    Value& operator[](Key k){
        size_t index = get_bucket_index(k);
        for (int i = 0; i < buckets[index].fsize(); i++){
            HashNode<Key, Value> node = buckets[index].FGET_key(i);
            if (node.key == k){
                return node.value;
            }
        }
        insert(k, Value());
        return Value();
    }

    const Value& operator[](Key k) const {
        size_t index = get_bucket_index(k);
        for (int i = 0; i < buckets[index].fsize(); i++){
            HashNode<Key, Value> node = buckets[index].FGET_key(i);
            if (node.key == k){
                return node.value;
            }
        }
        throw invalid_argument("Const operator");
    }
    
    bool find(const Key& key) const {
        if (buckets.msize() == 0) return false;
        
        size_t index = get_bucket_index(key);
        for (int i = 0; i < buckets[index].fsize(); i++){
            if (buckets[index].FGET_key(i).key == key){return true;}
        }

        return false;
    }
    
    void insert(const Key& key, const Value& value) {
        if (find(key)) {
            return;
        }
        
        if (static_cast<double>(countItems) / static_cast<double>(buckets.get_cap()) >= loadFactor){
            rehash();
        }
        
        int index = get_bucket_index(key);
        
        if (buckets[index].fsize() == 0){bucketsThersSmtng++;}
        
        HashNode<Key, Value> node{key, value};
        buckets[index].FPUSH_back(node);
        countItems++;
       // buckets.size++;
        
        if (static_cast<double>(bucketsThersSmtng) / static_cast<double>(buckets.get_cap()) >= 0.9){
            rehash();   
        }
    }
    
    Value get(const Key& key) const {
        size_t index = get_bucket_index(key);
        for (int i = 0; i < buckets[index].fsize(); i++) {
            HashNode<Key, Value> element = buckets[index].FGET_key(i);
            if (element.key == key) {
                return element.value;
            }
        }
        return Value();
    }
    
    void erase(const Key& key) {
        if (countItems == 0) return;
        
        size_t index = get_bucket_index(key);
        if (buckets[index].fsize() == 0) {return;}
        if (buckets[index].fsize() == 1) {bucketsThersSmtng--;}
        
        Value val = Value();
        
        for (int i = 0; i < buckets[index].fsize(); i++) {
            HashNode<Key, Value> element = buckets[index].FGET_key(i);
            if (element.key == key) {
                val = element.value;
            }
        }
        if (val == Value()){
            return;
        }
        HashNode<Key, Value> nd{key, val};
        buckets[index].FDEL_val(nd);
    }
    
    
    size_t size() const {
        return countItems;
    }
    
    bool empty() const {
        return countItems == 0;
    }
    
    void PRINT() const {
        for (int i = 0; i < buckets.get_cap(); i++){
            buckets[i].PRINT();
        }
    }
};

