#pragma once

#include "OpenTable.hpp"

template<typename Key, typename Value>
ostream& operator<<(ostream& os, OTNode<Key, Value> s){
    os << s.key;
    return os;
}

template<typename T>
class MySet{
private:
    OpenTable<T, bool> buckets;
    int itemcnt = 0;
    
public:
    MySet(int initcap) : buckets(OpenTable<T, bool>(initcap)) {};
    MySet() : buckets(OpenTable<T, bool>(10)) {};
    
    bool SET_AT(const T& key) const {
        return buckets.find(key);
    }
    
    void SETADD(const T& key) {
        buckets.insert(key, 0);
        itemcnt++;
    }
    
    void SETDEL(const T& key) {
        buckets.erase(key);
        itemcnt--;
    }
    
    
    size_t size() const {
        return itemcnt;
    }
    
    bool empty() const {
        return itemcnt == 0;
    }
    
    void SPRINT(){
        buckets.SPRINT();
    }
    
    void set_write_file(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            return;
        }
        

        file << buckets.get_cap() << ' ' << itemcnt << ' ' << endl;

        for (int i = 0; i < buckets.get_cap(); i++){
            file << buckets.get_key(i) << ' ';
        }

        file.close();
    }
    
    void set_read_file(const string& filename){
        ifstream file(filename);
        if (!file.is_open()) {
            return;
        }
        if (is_file_empty(filename)){return;}
        
        int sz, count;
        file >> sz >> count;
        
        buckets = OpenTable<T, bool>(sz);
        itemcnt = 0;
        
        T value;
        while (file >> value) {
            SETADD(value);
        }

        file.close();
    }
};

