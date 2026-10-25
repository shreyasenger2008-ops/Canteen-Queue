#ifndef CANTEEN_QUEUE_HPP
#define CANTEEN_QUEUE_HPP

#include "canteen/ds/SinglyLinkedList.hpp"
#include <stdexcept>
#include <iostream>

namespace canteen {

/**
 * @brief Custom FIFO (First-In, First-Out) Queue implemented using the custom
 * Singly Linked List foundation.
 * 
 * Front and rear pointers in the underlying linked list guarantee:
 *   - enqueue: O(1) time complexity (appends at rear)
 *   - dequeue: O(1) time complexity (removes from front)
 *   - front: O(1) time complexity
 *   - isEmpty: O(1) time complexity
 *   - size: O(1) time complexity
 * 
 * Enforces strict FIFO ordering: elements are removed in the exact order
 * they arrived.
 * 
 * @tparam T Element type
 */
template <typename T>
class Queue {
private:
    SinglyLinkedList<T> list;

public:
    Queue() = default;

    /**
     * @brief Inserts an element at the rear of the queue.
     * Complexity: O(1)
     */
    void enqueue(const T& item) {
        list.insertBack(item);
    }

    /**
     * @brief Inserts an element at the rear using move semantics.
     * Complexity: O(1)
     */
    void enqueue(T&& item) {
        list.insertBack(std::move(item));
    }

    /**
     * @brief Removes and returns the element at the front of the queue.
     * Complexity: O(1)
     * @throws std::underflow_error if queue is empty
     */
    T dequeue() {
        if (isEmpty()) {
            throw std::underflow_error("Cannot dequeue from an empty queue.");
        }
        T frontItem = list.front();
        list.deleteFront();
        return frontItem;
    }

    /**
     * @brief Accesses the front element without removing it.
     * Complexity: O(1)
     * @throws std::underflow_error if queue is empty
     */
    T& front() {
        if (isEmpty()) {
            throw std::underflow_error("Cannot inspect front of an empty queue.");
        }
        return list.front();
    }

    /**
     * @brief Const access to the front element.
     * Complexity: O(1)
     */
    const T& front() const {
        if (isEmpty()) {
            throw std::underflow_error("Cannot inspect front of an empty queue.");
        }
        return list.front();
    }

    /**
     * @brief Accesses the rear element without removing it.
     * Complexity: O(1)
     */
    const T& rear() const {
        if (isEmpty()) {
            throw std::underflow_error("Cannot inspect rear of an empty queue.");
        }
        return list.back();
    }

    /**
     * @brief Checks if queue has no elements.
     * Complexity: O(1)
     */
    bool isEmpty() const noexcept {
        return list.isEmpty();
    }

    /**
     * @brief Current number of elements in the queue.
     * Complexity: O(1)
     */
    size_t size() const noexcept {
        return list.size();
    }

    /**
     * @brief Removes an item matching the predicate (e.g. for order cancellation).
     * Maintains FIFO order of all remaining elements.
     * Complexity: O(n)
     * @return true if an item was removed, false otherwise
     */
    template <typename Predicate>
    bool removeIf(Predicate&& pred) {
        return list.deleteBy(std::forward<Predicate>(pred));
    }

    /**
     * @brief Searches for an item matching predicate in the queue.
     * Complexity: O(n)
     */
    template <typename Predicate>
    const T* find(Predicate&& pred) const {
        return list.find(std::forward<Predicate>(pred));
    }

    /**
     * @brief Non-const find.
     * Complexity: O(n)
     */
    template <typename Predicate>
    T* find(Predicate&& pred) {
        return list.find(std::forward<Predicate>(pred));
    }

    /**
     * @brief Traverses all elements in arrival order.
     * Complexity: O(n)
     */
    template <typename Visitor>
    void traverse(Visitor&& visitor) const {
        list.traverse(std::forward<Visitor>(visitor));
    }

    /**
     * @brief Non-const traversal.
     * Complexity: O(n)
     */
    template <typename Visitor>
    void traverse(Visitor&& visitor) {
        list.traverse(std::forward<Visitor>(visitor));
    }

    /**
     * @brief Displays queue contents from front (earliest) to rear (latest).
     */
    void displayQueue() const {
        if (isEmpty()) {
            std::cout << "  [Queue is empty]\n";
            return;
        }
        size_t pos = 1;
        list.traverse([&pos](const T& item) {
            std::cout << "  " << pos++ << ". " << item.toString() << "\n";
        });
    }

    /**
     * @brief Clears all elements from the queue.
     */
    void clear() noexcept {
        list.clear();
    }
};

} // namespace canteen

#endif // CANTEEN_QUEUE_HPP
