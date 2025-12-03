#pragma once

#include "List_header.hpp"

template<typename T>
struct FLNode {
    T key = T();
    FLNode* next = nullptr;
    FLNode() : key(T()), next(nullptr){}
    FLNode(T value, FLNode* ptr) : key(value), next(ptr){}
};

template<typename T>
class ForwardList {
private:
    FLNode<T>* head = nullptr;
    int size = 0;

public:
    ForwardList() : head(nullptr){}

    ForwardList(const ForwardList& other) : head(nullptr), size(0) {
        FLNode<T>* curr = other.head;
        while (curr) {
            FPUSH_back(*this, curr->key);
            curr = curr->next;
        }
    }

    ForwardList& operator=(const ForwardList& other) {
        if (this == &other) return *this; // защита от самоприсваивания
        destroy_list(head);
        head = nullptr;
        size = 0;
        FLNode<T>* curr = other.head;
        while (curr) {
            FPUSH_back(curr->key);
            curr = curr->next;
        }
        return *this;
    }

    void destroy_list(FLNode<T>* head){
        if (size == 0){return;}
        FLNode<T>* current = head;
        while (current != nullptr) {
            FLNode<T>* next = current->next;
            delete current;
            current = next;
        }
        size = 0;
    }
    
    ~ForwardList(){
        destroy_list(head);
    }

    FLNode<T>* FGET(T key) const {        // O(N)
        FLNode<T>* target = head;
        while (target -> key != key){
            target = target -> next;
        }
        return target;
    }

    FLNode<T>* FGET_index(int index) const {   // O(N)
        FLNode<T>* ptr = head;
        if (index >= size || index < 0){
            throw out_of_range("forward list index out of bounds");
        }
        for (int i = 0; i < index; i++){
            ptr = ptr -> next;
        }
        return ptr;
    }

    T FGET_key(int index) const {
        try{
            return FGET_index(index) -> key;
        } catch(const exception& error){
            cerr << error.what() << endl;
        }
        return T();
    }
    
    void FPUSH_next(int index, T key){    // O(n)
        try{
            FLNode<T>* ptr = FGET_index(index);
            FLNode<T>* newNode = new FLNode<T>;
            newNode -> key = key;
            newNode -> next = ptr -> next;
            ptr -> next = newNode;
            size++;
        } catch(const exception& error){
            cerr << error.what() << endl;
        }
    }

    void FPUSH_prev(int index, T key){    // O(1)
        FLNode<T>* nextNode = head;
        FLNode<T>* prevNode;
        FLNode<T>* newNode = new FLNode<T>;
        for(int i = 0; i < index; i++){
            prevNode = nextNode;
            nextNode = nextNode -> next;
        }
        newNode -> key = key;
        prevNode -> next = newNode;
        newNode -> next = nextNode;
        size++;
    }

    void FPUSH_front(T key){        // O(1)
        FLNode<T>* newNode = new FLNode<T>;
        newNode -> key = key;
        newNode -> next = head -> next;
        head -> next = newNode;
        size++;
    }

    void FPUSH_back(T key){       // O(N)
        if (head == nullptr){
            head = new FLNode<T>{key, nullptr};
            size++;
            return;
        }
        FLNode<T>* ptr = head;
        while(ptr -> next){
            ptr = ptr -> next;
        }
        FLNode<T>* newNode = new FLNode<T>;
        newNode -> key = key;
        newNode -> next = nullptr;
        ptr -> next = newNode;
        size++;
    }

    void FDEL_next(int index){     // O(N)
        try{
            FLNode<T>* ptr = FGET_index(index);
            FLNode<T>* deleteNode = ptr -> next;
            ptr -> next = deleteNode -> next;
            delete deleteNode;
            size--;
        } catch(exception& error){
            cerr << error.what() << endl;
        }
    }

    void FDEL_prev(int index){
        try{
            FLNode<T>* nextNode = head;
            FLNode<T>* prevNode;
            for(int i = 0; i < index - 1; i++){
                prevNode = nextNode;
                nextNode = nextNode -> next;
            }
            prevNode -> next = nextNode -> next;
            size--;
            delete nextNode;
        } catch(exception& error){
            cerr << error.what() << endl;
        }
    }

    void FDEL_front(){              // O(1)
        FLNode<T>* deleteNode = head;
        head = head -> next;
        delete deleteNode;
        size--;
    }


    void FDEL_back(){       // O(N)
        FLNode<T>* ptr = head;
        while (ptr -> next -> next){
            ptr = ptr -> next;
        }
        FLNode<T>* deleteNode = ptr -> next;
        ptr -> next = nullptr;
        delete deleteNode;
        size--;
    }

    void FDEL_val(T key){      // O(N)
        FLNode<T>* delNode = head;
        FLNode<T>* prevNode = head;
        while (delNode -> key != key){
            prevNode = delNode;
            delNode = delNode -> next;
        }
        if (prevNode == delNode){
            head = delNode -> next;
            delete delNode;
            size--;
        }
        else {
            prevNode -> next = delNode -> next;
            delete delNode;
            size--;
        }
    }

    void PRINT() const {
        FLNode<T>* ptr = head;
        while (ptr) {
            cout << ptr -> key << ' ';
            ptr = ptr -> next;
        }
        cout << endl;
    }

    int fsize()const {
        return size;
    }
};