//
// Created by fenghl on 2020/10/13.
//

#ifndef BLOCK_QUEUE_LINKEDBLOCKINGQUEUE_H
#define BLOCK_QUEUE_LINKEDBLOCKINGQUEUE_H

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

template <typename T>
class LinkedBlockingQueue {
    struct Node {
        T item;
        struct Node *next;
        Node(T item):item(item), next(nullptr){}
        Node(): next(nullptr) {}
    };

public:
//    LinkedBlockingQueue() {
//        new (this) LinkedBlockingQueue(INT64_MAX);
//    }

    LinkedBlockingQueue() : LinkedBlockingQueue(INT64_MAX) {};

    LinkedBlockingQueue(int64_t capacity): count(0),capacity(capacity) {
        head = last = new Node();
    }

private:
    /* The capacity bound*/
    int64_t capacity;

    /*Current number of elements */
    std::atomic<int64_t> count;

    /**
     * Head of linked list.
     * Invariant: head.item unused
     */
    Node* head;

    /**
     * Tail of linked list.
     * Invariant: last.next == nullptr
     */
    Node* last;

    /** Lock held by take, pool, etc */
    std::mutex takeLock;

    /** Wait queue for waiting takes */
    std::condition_variable notEmpty;

    /** Lock held by put, offer, etc */
    std::mutex putLock;

    /** Wait queue for waiting puts */
    std::condition_variable notFull;

//public:

    /**
     * Links node at end of queue/
     * @param node the node
     */
    void enqueue(const T& item) {
        Node *node = new Node(item);
        last = last->next = node;
    }

    /**
     * Removes a node from head of queue.
     * @return the node
     */
    T& dequeue() {
        Node *h = head;
        Node *first = head->next;
        delete h;
        head = first;
        return first->item;
    }


public:
    /**
     * Returns the number of elements in the queue.
     * @return the number of elements in the queue.
     */
    int64_t size() { return count; }

    int64_t remainingCapacity() { return capacity - count; }

    void put(const T& item);

    template <class Rep, class Period>
    bool offer(const T& item, const std::chrono::duration<Rep, Period> rel_time);

    bool offer(const T& item);

    void take(T& returnVal);

    template <class Rep, class Period>
    bool poll(T& returnVal, const std::chrono::duration<Rep, Period> rel_time);

    bool poll(T& returnVal);

};




/**
 * Insert the specified element at the tail of this queue, waiting if
 * necessary for space to become available
 * @param item
 */
template <typename T>
void LinkedBlockingQueue<T>::put(const T& item){
    int c;
    {
        std::unique_lock<std::mutex> lck{putLock};
        if( count == capacity) {
            notFull.wait(lck, [this](){return count < capacity;});
        }
        enqueue(item);  //不应该把申请空间放在锁里面，耗时有点大
        c = count.fetch_add(1);
    }
    if(c + 1 < capacity) {
        notFull.notify_one();
    }

    if(0 == c) {
        notEmpty.notify_one();
    }
}

/**
 * Insert the specified element at the tail of this queue, waiting if
 * necessary up to the specified wait time for space to become available
 * @param item
 * @return {@code true} if successfully, or {@code false} if the
 *         the specified waiting time elapses before space is available.
 */
template <typename T>
template <class Rep, class Period>
bool LinkedBlockingQueue<T>::offer(const T& item, const std::chrono::duration<Rep, Period> rel_time) {
    int c;
    {
        std::unique_lock<std::mutex> lck{putLock};
        if( count == capacity) {
            if(!notFull.wait_for(lck, rel_time, [this](){return count < capacity;})){
                return false;
            }
        }
        enqueue(item);  //不应该把申请空间放在锁里面，耗时有点大
        c = count.fetch_add(1);
    }
    if(c + 1 < capacity) {
        notFull.notify_one();
    }

    if(0 == c) {
        notEmpty.notify_one();
    }

    return true;
}

/**
 * Insert the specified element at the tail of this queue if it is
 * possible to do so immediately without exceeding the queue's capacity,
 * returning {@code true} upon success and {@code false} if this queue
 * id full.
 * when using a capacity-restricted queue, this method is generally
 * preferable to method {@link BlockingQueue#add add}, which can fail to
 * insert an element only return {@code false}.
 * @param item
 * @return {@code true} if successfully, or {@code false} if
 * space isn't available.
 */
template <typename T>
bool LinkedBlockingQueue<T>::offer(const T& item){
    int c;
    {
        std::unique_lock<std::mutex> lck{putLock};
        if( count == capacity) {
            return false;
        }
        enqueue(item);  //不应该把申请空间放在锁里面，耗时有点大
        c = count.fetch_add(1);
    }
    if(c + 1 < capacity) {
        notFull.notify_one();
    }

    if(0 == c) {
        notEmpty.notify_one();
    }

    return true;
}

template <typename T>
void LinkedBlockingQueue<T>::take(T& returnVal) {
    int c;
    {
        std::unique_lock<std::mutex> lck{takeLock};
        if( 0 == count) {
            notEmpty.wait(lck, [this](){return count > 0 ;});
        }
        returnVal = dequeue();
        c = count.fetch_sub(1);
    }

    if( c > 1 ) {
        notEmpty.notify_one();
    }

    if(c == capacity) {
        notFull.notify_one();
    }
}

template <typename T>
template <class Rep, class Period>
bool LinkedBlockingQueue<T>::poll(T& returnVal, const std::chrono::duration<Rep, Period> rel_time) {
    int c;
    {
        std::unique_lock<std::mutex> lck{takeLock};
        if( 0 == count) {
            if(!notEmpty.wait_for(lck, rel_time, [this](){return count > 0;})){
                return false;
            }
        }
        returnVal = dequeue();
        c = count.fetch_sub(1);
    }

    if(c > 1) {
        notEmpty.notify_one();
    }

    if(c == capacity) {
        notFull.notify_one();
    }

    return true;
}

template <typename T>
bool LinkedBlockingQueue<T>::poll(T& returnVal) {
    int c;
    {
        std::unique_lock<std::mutex> lck{takeLock};
        if( 0 == count) {
            return false;
        }
        returnVal = dequeue();
        c = count.fetch_sub(1);
    }

    if(c > 1) {
        notEmpty.notify_one();
    }

    if(c == capacity) {
        notFull.notify_one();
    }

    return true;
}

#endif //BLOCK_QUEUE_LINKEDBLOCKINGQUEUE_H
