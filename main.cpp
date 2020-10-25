#include <iostream>
#include "LinkedBlockingQueue.h"
#include <mutex>
#include <thread>
#include <chrono>

void putTest(LinkedBlockingQueue& queue) {
    for(int i = 0; i < 100; i++) {
        queue.put(i+3);
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
}

void takeTest(LinkedBlockingQueue& queue, int x) {
    int val;
    for(int i = 0; i < 50; i++) {
        queue.take(val);
        std::cout << x << ":" << val << std::endl;
    }
}

int main() {
    LinkedBlockingQueue queue{10};

//    for(int i = 0; i < 10; i++) {
//        queue.put(i);
//    }
//
//    std::cout << queue.offer(100, std::chrono::seconds(10)) << std::endl;
//    std::cout << queue.offer(100) << std::endl;
//
//    int val;
//
//    std::cout << "dequeue:" << std::endl;
//    std::cout << queue.poll(val) << std::endl;
//    std::cout << queue.poll(val,std::chrono::seconds(10)) << std::endl;
//
//    std::cout << "enqueue:" << std::endl;
//    std::cout << queue.offer(101, std::chrono::seconds(10)) << std::endl;
//    std::cout << queue.offer(102) << std::endl;
//
//    for(int i = 0; i < 10; i++) {
//        queue.take(val);
//        std::cout << val << " ";
//    }
//    std::cout << std::endl;
//
//    std::cout << "dequeue:" << std::endl;
//    std::cout << queue.poll(val) << std::endl;
//    std::cout << queue.poll(val,std::chrono::seconds(10)) << std::endl;


    std::cout << "==========Begin==========" << std::endl;

    std::thread t1(putTest, std::ref(queue));
    std::thread t2(takeTest, std::ref(queue),1);
    std::thread t3(takeTest, std::ref(queue),2);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "===========End===========" << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
