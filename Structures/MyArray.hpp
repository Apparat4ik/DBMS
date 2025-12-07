#pragma once

#include "List_header.hpp"

template<typename T>
struct ArNode {
    T key = T();
};



template<typename T>
class MyArray {
private:
    ArNode<T>* data;
    int size;
    int capacity;

public:
    MyArray() : data(new ArNode<T>[10]), size(0), capacity(10){}

    MyArray(initializer_list<T> list) 
        : data(new ArNode<T>[list.size()])
        , size(list.size())
        , capacity(list.size() * 2) {
            int i = 0;
            for (T it : list){
                data[i++].key = it;
            }
        } 


    T& operator[](int index) {
        if (index < 0 || index >= capacity) {
            throw std::out_of_range("Index out of array bounds");
        }
        return data[index].key;
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= capacity) {
            throw std::out_of_range("Index out of array bounds");
        }
        return data[index].key;
    }
    
    MyArray(const MyArray& other) {
        capacity = other.capacity;
        size = other.size;
        data = new ArNode<T>[capacity];
        for (int i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
    }

    MyArray& operator=(const MyArray& other) {
        if (this == &other) return *this;
        delete[] data;
        capacity = other.capacity;
        size = other.size;
        data = new ArNode<T>[capacity];
        for (int i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
        return *this;
    }

    bool operator!=(const MyArray<T>& other){
        if (size != other.msize()){
            return true;
        }
        for (int i = 0; i < size; i++){
            if (data[i].key != other[i]){
                return true;
            }
        }
        return false;
    }


    
    MyArray(int init_cap){
        data = new ArNode<T>[init_cap];
        size = 0;
        capacity = init_cap;
    }
    
    ~MyArray(){
        delete[] data;
        data = nullptr;
        size = 0;
        capacity = 1;
    }

    void resize(){
        int newCap = capacity * 2;
        if (newCap == 0) newCap = 1;
        ArNode<T>* newData = new ArNode<T>[newCap];
        
        // Копируем только если есть что копировать
        if (size > 0) {
            for (int i = 0; i < capacity; i++){
                newData[i] = data[i];
            }
        }
        
        if (!(data == nullptr)){
            delete[] data;
        }
        data = newData;
        capacity = newCap;  
    }

    void MPUSH_back(T key) {                 // O(1)
        if (size >= capacity){resize();}
        data[size].key = key;
        size++;
    }

    void MPOP_back(){
        if (empty()){
            return;
        }
        data[size].key = T();
        size--;
    }


    void MPUSH_index(int index, T key){        // O(N)
        if (index >= capacity || index < 0){
            throw invalid_argument("Array index out od bounds");
        }
        
        for (int i = capacity - 1; i >= index; i--){
            data[i + 1] = data[i];
        }
        data[index].key = key;
        size++;
    }

    T MGET(int index) const {             // O(1)
        if (index >= capacity || index < 0){
            throw invalid_argument("Array index out od bounds");
        }
        return data[index].key;
    }

    void MDEL(int index){       // O(N)
        if (index >= capacity || index < 0){
            throw invalid_argument("Array index out od bounds");
        }
        for (int i = index; i < capacity - 1; i++){
            data[i] = data[i + 1];
        }
        data[capacity - 1].key = T();
        size--;
    }

    void MSWAP(int index, T swapkey){
        if (index >= capacity || index < 0){
            throw invalid_argument("Array index out od bounds");
        }
        data[index].key = swapkey;
    }

    void PRINT() const {
        for (int i = 0; i < capacity; i++) {
            cout << data[i].key << ' ';
        }
        cout << endl;
    }

    int msize() const {
        return size;
    }

    bool empty() const {
        if (size == 0){
            return true;
        } else {
            return false;
        }
    }

    T back() const {
        return data[size - 1].key;
    }

    int get_cap() const{
        return capacity;
    }
};

template<typename T>
ostream& operator<<(ostream& os, MyArray<T> mr){
    for (int i = 0; i < mr.msize(); i++){
        os << mr[i] << ' '; 
    }
    os << endl;
    return os;
}