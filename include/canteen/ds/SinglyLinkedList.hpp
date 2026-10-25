#ifndef CANTEEN_SINGLY_LINKED_LIST_HPP
#define CANTEEN_SINGLY_LINKED_LIST_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <functional>
#include <iostream>

namespace canteen {

/**
 * @brief Node in a custom singly linked list.
 */
template <typename T>
struct ListNode {
    T data;
    ListNode<T>* next{nullptr};

    explicit ListNode(const T& val) : data(val), next(nullptr) {}
    explicit ListNode(T&& val) : data(std::move(val)), next(nullptr) {}
};

/**
 * @brief Custom Singly Linked List demonstrating dynamic memory allocation,
 * raw pointer manipulation, O(1) insertion at ends, O(n) search and deletion,
 * and robust memory leak prevention complying with the Rule of Five.
 * 
 * Time Complexities:
 *   - Insert Front: O(1)
 *   - Insert Back: O(1) (with tail pointer)
 *   - Delete Front: O(1)
 *   - Delete by predicate/ID: O(n)
 *   - Search: O(n)
 *   - Traversal: O(n)
 * Space Complexity:
 *   - O(n) dynamically allocated nodes
 */
template <typename T>
class SinglyLinkedList {
private:
    ListNode<T>* head{nullptr};
    ListNode<T>* tail{nullptr};
    size_t listSize{0};

    void copyFrom(const SinglyLinkedList& other) {
        ListNode<T>* curr = other.head;
        while (curr != nullptr) {
            insertBack(curr->data);
            curr = curr->next;
        }
    }

public:
    SinglyLinkedList() = default;

    // Destructor: Safely deallocates all nodes to prevent memory leaks
    ~SinglyLinkedList() {
        clear();
    }

    // Copy Constructor (Deep Copy)
    SinglyLinkedList(const SinglyLinkedList& other) {
        copyFrom(other);
    }

    // Copy Assignment Operator (Deep Copy with clean existing memory)
    SinglyLinkedList& operator=(const SinglyLinkedList& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // Move Constructor
    SinglyLinkedList(SinglyLinkedList&& other) noexcept
        : head(other.head), tail(other.tail), listSize(other.listSize) {
        other.head = nullptr;
        other.tail = nullptr;
        other.listSize = 0;
    }

    // Move Assignment Operator
    SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            listSize = other.listSize;
            other.head = nullptr;
            other.tail = nullptr;
            other.listSize = 0;
        }
        return *this;
    }

    /**
     * @brief Checks if list has no nodes.
     * Complexity: O(1)
     */
    bool isEmpty() const noexcept {
        return listSize == 0;
    }

    /**
     * @brief Current count of nodes.
     * Complexity: O(1)
     */
    size_t size() const noexcept {
        return listSize;
    }

    /**
     * @brief Inserts element at the head of the list.
     * Complexity: O(1)
     */
    void insertFront(const T& val) {
        auto* newNode = new ListNode<T>(val);
        newNode->next = head;
        head = newNode;
        if (tail == nullptr) {
            tail = head;
        }
        ++listSize;
    }

    /**
     * @brief Inserts element at the head of the list using move semantics.
     * Complexity: O(1)
     */
    void insertFront(T&& val) {
        auto* newNode = new ListNode<T>(std::move(val));
        newNode->next = head;
        head = newNode;
        if (tail == nullptr) {
            tail = head;
        }
        ++listSize;
    }

    /**
     * @brief Inserts element at the tail of the list.
     * Complexity: O(1) with maintained tail pointer
     */
    void insertBack(const T& val) {
        auto* newNode = new ListNode<T>(val);
        if (tail == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++listSize;
    }

    /**
     * @brief Inserts element at the tail of the list using move semantics.
     * Complexity: O(1)
     */
    void insertBack(T&& val) {
        auto* newNode = new ListNode<T>(std::move(val));
        if (tail == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++listSize;
    }

    /**
     * @brief Deletes the front (head) element of the list.
     * Complexity: O(1)
     * @throws std::underflow_error if the list is empty
     */
    void deleteFront() {
        if (head == nullptr) {
            throw std::underflow_error("Cannot delete from an empty linked list.");
        }
        ListNode<T>* temp = head;
        head = head->next;
        if (head == nullptr) {
            tail = nullptr;
        }
        delete temp;
        --listSize;
    }

    /**
     * @brief Deletes the first node satisfying the provided predicate.
     * Handles empty list, single-node list, head match, middle match, tail match.
     * Complexity: O(n)
     * @return true if an element was found and deleted, false otherwise
     */
    template <typename Predicate>
    bool deleteBy(Predicate&& pred) {
        if (head == nullptr) {
            return false;
        }

        // Case 1: Head node matches
        if (pred(head->data)) {
            ListNode<T>* temp = head;
            head = head->next;
            if (head == nullptr) {
                tail = nullptr;
            }
            delete temp;
            --listSize;
            return true;
        }

        // Case 2: Middle or Tail node matches
        ListNode<T>* prev = head;
        ListNode<T>* curr = head->next;
        while (curr != nullptr) {
            if (pred(curr->data)) {
                prev->next = curr->next;
                if (curr == tail) {
                    tail = prev; // Update tail if deleted node was the last one
                }
                delete curr;
                --listSize;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    /**
     * @brief Finds first element matching the predicate.
     * Complexity: O(n)
     * @return Pointer to element data if found, nullptr otherwise
     */
    template <typename Predicate>
    T* find(Predicate&& pred) {
        ListNode<T>* curr = head;
        while (curr != nullptr) {
            if (pred(curr->data)) {
                return &(curr->data);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    /**
     * @brief Const version of find.
     * Complexity: O(n)
     */
    template <typename Predicate>
    const T* find(Predicate&& pred) const {
        ListNode<T>* curr = head;
        while (curr != nullptr) {
            if (pred(curr->data)) {
                return &(curr->data);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    /**
     * @brief Reference to front element.
     * Complexity: O(1)
     * @throws std::underflow_error if empty
     */
    T& front() {
        if (head == nullptr) {
            throw std::underflow_error("Linked list is empty.");
        }
        return head->data;
    }

    /**
     * @brief Const reference to front element.
     * Complexity: O(1)
     */
    const T& front() const {
        if (head == nullptr) {
            throw std::underflow_error("Linked list is empty.");
        }
        return head->data;
    }

    /**
     * @brief Reference to back element.
     * Complexity: O(1)
     * @throws std::underflow_error if empty
     */
    T& back() {
        if (tail == nullptr) {
            throw std::underflow_error("Linked list is empty.");
        }
        return tail->data;
    }

    /**
     * @brief Const reference to back element.
     * Complexity: O(1)
     */
    const T& back() const {
        if (tail == nullptr) {
            throw std::underflow_error("Linked list is empty.");
        }
        return tail->data;
    }

    /**
     * @brief Deallocates all nodes and resets pointers.
     * Complexity: O(n)
     */
    void clear() noexcept {
        ListNode<T>* curr = head;
        while (curr != nullptr) {
            ListNode<T>* nextNode = curr->next;
            delete curr;
            curr = nextNode;
        }
        head = nullptr;
        tail = nullptr;
        listSize = 0;
    }

    /**
     * @brief Traverses the list from head to tail, invoking visitor(element).
     * Complexity: O(n)
     */
    template <typename Visitor>
    void traverse(Visitor&& visitor) const {
        ListNode<T>* curr = head;
        while (curr != nullptr) {
            visitor(curr->data);
            curr = curr->next;
        }
    }

    /**
     * @brief Non-const traversal allowing modification of elements.
     * Complexity: O(n)
     */
    template <typename Visitor>
    void traverse(Visitor&& visitor) {
        ListNode<T>* curr = head;
        while (curr != nullptr) {
            visitor(curr->data);
            curr = curr->next;
        }
    }

    /**
     * @brief Displays elements using standard output.
     */
    void display() const {
        if (isEmpty()) {
            std::cout << "[Empty List]\n";
            return;
        }
        ListNode<T>* curr = head;
        while (curr != nullptr) {
            std::cout << curr->data;
            if (curr->next != nullptr) {
                std::cout << " -> ";
            }
            curr = curr->next;
        }
        std::cout << " -> NULL\n";
    }
};

} // namespace canteen

#endif // CANTEEN_SINGLY_LINKED_LIST_HPP
