#ifndef CANTEEN_STACK_HPP
#define CANTEEN_STACK_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <iostream>

namespace canteen {

/**
 * @brief Node for the linked-list based stack.
 */
template <typename T>
struct StackNode {
    T data;
    StackNode<T>* next{nullptr};

    explicit StackNode(const T& val) : data(val), next(nullptr) {}
    explicit StackNode(T&& val) : data(std::move(val)), next(nullptr) {}
};

/**
 * @brief Custom LIFO (Last-In, First-Out) Stack implemented using singly linked
 * nodes. Used to track administrative actions and support undo operations.
 * 
 * Time Complexities:
 *   - push: O(1)
 *   - pop: O(1)
 *   - peek: O(1)
 *   - isEmpty: O(1)
 *   - size: O(1)
 * Space Complexity:
 *   - O(n) dynamically allocated nodes
 * 
 * @tparam T Element type
 */
template <typename T>
class Stack {
private:
    StackNode<T>* topNode{nullptr};
    size_t stackSize{0};

    void copyFrom(const Stack& other) {
        if (other.topNode == nullptr) return;
        
        // Reconstruct in correct order:
        // Use an array or reversed link pass to preserve LIFO order
        size_t n = other.stackSize;
        if (n == 0) return;
        T* buffer = new T[n];
        StackNode<T>* curr = other.topNode;
        size_t idx = 0;
        while (curr != nullptr && idx < n) {
            buffer[idx++] = curr->data;
            curr = curr->next;
        }
        // Push from back to front of buffer to match original stack order
        for (size_t i = idx; i > 0; --i) {
            push(buffer[i - 1]);
        }
        delete[] buffer;
    }

public:
    Stack() = default;

    // Destructor: prevents memory leaks
    ~Stack() {
        clear();
    }

    // Copy Constructor
    Stack(const Stack& other) {
        copyFrom(other);
    }

    // Copy Assignment Operator
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // Move Constructor
    Stack(Stack&& other) noexcept
        : topNode(other.topNode), stackSize(other.stackSize) {
        other.topNode = nullptr;
        other.stackSize = 0;
    }

    // Move Assignment Operator
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            clear();
            topNode = other.topNode;
            stackSize = other.stackSize;
            other.topNode = nullptr;
            other.stackSize = 0;
        }
        return *this;
    }

    /**
     * @brief Pushes an item onto the top of the stack.
     * Complexity: O(1)
     */
    void push(const T& item) {
        auto* newNode = new StackNode<T>(item);
        newNode->next = topNode;
        topNode = newNode;
        ++stackSize;
    }

    /**
     * @brief Pushes an item using move semantics.
     * Complexity: O(1)
     */
    void push(T&& item) {
        auto* newNode = new StackNode<T>(std::move(item));
        newNode->next = topNode;
        topNode = newNode;
        ++stackSize;
    }

    /**
     * @brief Removes and returns the top item from the stack.
     * Complexity: O(1)
     * @throws std::underflow_error if stack is empty
     */
    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Cannot pop from an empty stack.");
        }
        StackNode<T>* temp = topNode;
        T poppedVal = std::move(temp->data);
        topNode = topNode->next;
        delete temp;
        --stackSize;
        return poppedVal;
    }

    /**
     * @brief Returns a reference to the top item without removing it.
     * Complexity: O(1)
     * @throws std::underflow_error if stack is empty
     */
    T& peek() {
        if (isEmpty()) {
            throw std::underflow_error("Cannot peek into an empty stack.");
        }
        return topNode->data;
    }

    /**
     * @brief Const reference to the top item.
     * Complexity: O(1)
     */
    const T& peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Cannot peek into an empty stack.");
        }
        return topNode->data;
    }

    /**
     * @brief Checks if stack contains no elements.
     * Complexity: O(1)
     */
    bool isEmpty() const noexcept {
        return topNode == nullptr;
    }

    /**
     * @brief Current count of items in the stack.
     * Complexity: O(1)
     */
    size_t size() const noexcept {
        return stackSize;
    }

    /**
     * @brief Deallocates all nodes in the stack.
     * Complexity: O(n)
     */
    void clear() noexcept {
        while (topNode != nullptr) {
            StackNode<T>* temp = topNode;
            topNode = topNode->next;
            delete temp;
        }
        stackSize = 0;
    }
};

} // namespace canteen

#endif // CANTEEN_STACK_HPP
