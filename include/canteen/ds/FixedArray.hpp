#ifndef CANTEEN_FIXED_ARRAY_HPP
#define CANTEEN_FIXED_ARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include <functional>
#include <string>

namespace canteen {

/**
 * @brief Fixed-size array data structure demonstrating contiguous memory allocation,
 * indexed O(1) access, bounds verification, and linear O(n) traversal.
 * 
 * Time Complexities:
 *   - Indexed Access: O(1)
 *   - Update at index: O(1)
 *   - Traversal: O(n)
 *   - Search: O(n)
 * Space Complexity:
 *   - O(Capacity) contiguous memory
 * 
 * @tparam T Element type
 * @tparam Capacity Maximum number of elements in the array
 */
template <typename T, size_t Capacity>
class FixedArray {
private:
    T elements[Capacity];
    size_t currentSize{0};

public:
    FixedArray() = default;

    /**
     * @brief Number of active elements currently stored.
     * Complexity: O(1)
     */
    size_t size() const noexcept {
        return currentSize;
    }

    /**
     * @brief Maximum capacity of the array.
     * Complexity: O(1)
     */
    constexpr size_t capacity() const noexcept {
        return Capacity;
    }

    /**
     * @brief Check if array is empty.
     * Complexity: O(1)
     */
    bool isEmpty() const noexcept {
        return currentSize == 0;
    }

    /**
     * @brief Check if array has reached maximum capacity.
     * Complexity: O(1)
     */
    bool isFull() const noexcept {
        return currentSize >= Capacity;
    }

    /**
     * @brief Clear all elements.
     * Complexity: O(1) logical clear
     */
    void clear() noexcept {
        currentSize = 0;
    }

    /**
     * @brief Appends an element to the array if space is available.
     * Complexity: O(1)
     * @throws std::overflow_error if array is full
     */
    void add(const T& item) {
        if (currentSize >= Capacity) {
            throw std::overflow_error("FixedArray capacity exceeded (Capacity: " + std::to_string(Capacity) + ")");
        }
        elements[currentSize++] = item;
    }

    /**
     * @brief Appends an element using move semantics.
     * Complexity: O(1)
     */
    void add(T&& item) {
        if (currentSize >= Capacity) {
            throw std::overflow_error("FixedArray capacity exceeded (Capacity: " + std::to_string(Capacity) + ")");
        }
        elements[currentSize++] = std::move(item);
    }

    /**
     * @brief Bounds-checked access to element at index.
     * Complexity: O(1)
     * @throws std::out_of_range if index >= currentSize
     */
    T& at(size_t index) {
        if (index >= currentSize) {
            throw std::out_of_range("FixedArray index out of bounds: index " +
                                    std::to_string(index) + " >= size " + std::to_string(currentSize));
        }
        return elements[index];
    }

    /**
     * @brief Const bounds-checked access to element at index.
     * Complexity: O(1)
     * @throws std::out_of_range if index >= currentSize
     */
    const T& at(size_t index) const {
        if (index >= currentSize) {
            throw std::out_of_range("FixedArray index out of bounds: index " +
                                    std::to_string(index) + " >= size " + std::to_string(currentSize));
        }
        return elements[index];
    }

    /**
     * @brief Direct indexed access operator (without throwing bounds check).
     * Complexity: O(1)
     */
    T& operator[](size_t index) noexcept {
        return elements[index];
    }

    /**
     * @brief Const direct indexed access operator.
     * Complexity: O(1)
     */
    const T& operator[](size_t index) const noexcept {
        return elements[index];
    }

    /**
     * @brief Updates element at the given index.
     * Complexity: O(1)
     * @throws std::out_of_range if index >= currentSize
     */
    void update(size_t index, const T& item) {
        if (index >= currentSize) {
            throw std::out_of_range("Cannot update: index out of bounds (" +
                                    std::to_string(index) + " >= " + std::to_string(currentSize) + ")");
        }
        elements[index] = item;
    }

    /**
     * @brief Removes element at the specified index, shifting trailing elements left.
     * Complexity: O(n)
     */
    void removeAt(size_t index) {
        if (index >= currentSize) {
            throw std::out_of_range("Cannot remove: index out of bounds.");
        }
        for (size_t i = index; i + 1 < currentSize; ++i) {
            elements[i] = std::move(elements[i + 1]);
        }
        --currentSize;
    }

    /**
     * @brief Traverses all active elements and executes the provided visitor function.
     * Complexity: O(n)
     */
    template <typename Func>
    void traverse(Func&& visitor) const {
        for (size_t i = 0; i < currentSize; ++i) {
            visitor(elements[i], i);
        }
    }

    /**
     * @brief Raw pointer to underlying data.
     */
    T* data() noexcept { return elements; }
    const T* data() const noexcept { return elements; }

    // Iterator support for range-based for loops and algorithms
    T* begin() noexcept { return elements; }
    const T* begin() const noexcept { return elements; }
    T* end() noexcept { return elements + currentSize; }
    const T* end() const noexcept { return elements + currentSize; }
};

} // namespace canteen

#endif // CANTEEN_FIXED_ARRAY_HPP
