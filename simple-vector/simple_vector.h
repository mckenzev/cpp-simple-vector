#pragma once

#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>

#include "array_ptr.h"

// обертка для резерва памяти
struct ReserveProxyObj {
    explicit ReserveProxyObj(size_t capacity) : capacity_(capacity) {}
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}


template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // ArrayPtr сам очищает память
    ~SimpleVector() = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : SimpleVector(size, Type()) {}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : ptr_(size), size_(size), capacity_(size) {
        if (size > 0) {
            std::fill_n(ptr_.Get(), size, value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : ptr_(init.size()), size_(init.size()), capacity_(init.size()) {
        DataOverwrite(init.begin(), init.end(), ptr_.Get());
    }

    SimpleVector(const SimpleVector& other) 
        : ptr_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), ptr_.Get());
    }

    SimpleVector(SimpleVector&& other) : ptr_(std::exchange(other.ptr_, ArrayPtr<Type>(nullptr))),
                                         size_(std::exchange<size_t>(other.size_, 0)),
                                         capacity_(std::exchange<size_t>(other.capacity_, 0)) {}

    SimpleVector(ReserveProxyObj obj) : ptr_(ArrayPtr<Type>(obj.capacity_)),
                                        capacity_(obj.capacity_) {}

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            ptr_ = std::move(rhs.ptr_);
            size_ = std::exchange<size_t>(rhs.size_, 0);
            capacity_ = std::exchange<size_t>(rhs.capacity_, 0);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        UniversalPushBack(item);
    }

    void PushBack(Type&& item) {
        static_assert(std::is_move_constructible_v<Type>, "To call PushBack(const Type&&), the Type must be movable");
        UniversalPushBack(std::move(item));
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        return UniversalInsert(pos, value);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        return UniversalInsert(pos, std::move(value));
    }

    void PopBack() noexcept {
        if (size_ == 0) {
            return;
        }
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        // "cend() - 1" т.к. end не должен входит в диапазон
        IteratorChecking(pos, cbegin(), cend() - 1);

        Iterator non_const_it = const_cast<Iterator>(pos);
        DataOverwrite(non_const_it + 1, end(), non_const_it);
        --size_;
        return non_const_it;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }

        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        
        return ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        // Заполнение дефолтными значениями только для типов, имеющих такой конструктор
        static_assert(std::is_default_constructible_v<Type>, "The Resize() method only works for types that have a default constructor");

        // Перевыделение памяти не нужно
        if (new_size <= capacity_) {
            // Новый размер больше текущего. В новые ячейки надо добавить дефолтные элементы
            if (new_size > size_) {
                std::generate(ptr_.Get() + size_, ptr_.Get() + new_size, []{ return Type(); });
            }

            size_ = new_size;
            return;
        }

       // Случай с перевыделением памяти
        ArrayPtr<Type> array(new_size);
        DataOverwrite(begin(), end(), array.Get());
        
        std::generate(array.Get() + size_, array.Get() + new_size, []{ return Type(); });

        ptr_.swap(array);
        size_ = capacity_ = new_size;
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ >= new_capacity) {
            return;
        }

        ArrayPtr<Type> array(new_capacity);
        DataOverwrite(begin(), end(), array.Get());
        ptr_.swap(array);
        capacity_ = new_capacity;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ptr_.Get() + size_;
    }

    void swap(SimpleVector& other) noexcept {
        ptr_.swap(other.ptr_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    bool operator==(const SimpleVector<Type>& rhs) const noexcept {
        if (size_ != rhs.size_) {
            return false;
        }

        return std::equal(cbegin(), cend(), rhs.cbegin());
    }

    bool operator!=(const SimpleVector<Type>& rhs) const noexcept {
        return !(*this == rhs);
    }

    auto operator<=>(const SimpleVector<Type>& rhs) const noexcept {
        return std::lexicographical_compare_three_way(cbegin(), cend(), rhs.cbegin(), rhs.cend());
    }
    

private:
    // Вспомогательный метод для проверки принадлежности итератора it к диапазону итераторов произвольного доступа [ first : last ]
    void IteratorChecking(ConstIterator it, ConstIterator first, ConstIterator last) const {
        if (it < first || it > last) {
            throw std::out_of_range("Iterator out of range");
        }
    }

    // Метод для перезаписи данных в новую область памяти принимает константные итераторы, чтобы можно было передавать const и не const
    // но для работы преобразует все итераторы в не const
    void DataOverwrite(ConstIterator in_first, ConstIterator in_last, ConstIterator out_first) {
        Iterator in_f = const_cast<Iterator>(in_first);
        Iterator in_l = const_cast<Iterator>(in_last);
        Iterator ou_f = const_cast<Iterator>(out_first);
        // Если Type может перемещатсья без исключений перенос данных произойдет через move
        // Но если Type не может быть копирован, то кроме переноса с move нет другого способа переноса данных при реаллокациях
        // В таком случае при выпадении исключения при move SimpleVector сломается так же, как и оригинальный vector (Test5 в main.cpp)
        if constexpr(!std::is_copy_assignable_v<Type> || std::is_nothrow_move_assignable_v<Type>) {
            std::move(in_f, in_l, ou_f);
        } else {
            // В случае если без риска исключений нельзя переносить элементы, но можно копировать, в новый массив данные будут скопированы
            std::copy(in_f, in_l, ou_f);
        }
    }

    // Реализация для copy и move item одинаковая, поэтому решил, что проще будет этот метод вызывать внутри публичных методов
    template <typename U>
    void UniversalPushBack(U&& item) {
        if (size_ == capacity_) {
            // Если capacity_ = 0, то capacity *= 2 дало бы 0
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);

            ArrayPtr<Type> array(new_capacity);

            // DataOverwrite может через move в array перенести данные из ptr_, поэтому заполнение нового массива начинается с нового элемента
            // Если это действие выбросит исключение, то ptr_ останется неизменным
            array[size_] = std::forward<U>(item);

            // При копировании и nothrow move этот метод будет строго гарантировать безопасность исключений
            // Но если же здесь при переносе с move выпадет исключение, значит nothrow move у Type нет и Type не может быть копирован
            // Получается, что move с риском исключения был единственным возможным вариантом
            DataOverwrite(begin(), end(), array.Get());

            ptr_.swap(array);
            capacity_ = new_capacity;
        } else {
            ptr_[size_] = std::forward<U>(item);
        }
        ++size_;
    }

    template <typename U>
    Iterator UniversalInsert(ConstIterator pos, U&& value) {
        IteratorChecking(pos, cbegin(), cend());

        size_t idx = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            ArrayPtr<Type> array(new_capacity);
            // Вставка значения
            array[idx] = std::forward<U>(value);

            // Копирование первой половины
            DataOverwrite(begin(), begin() + idx, array.Get());
            
            // Копирование второй половины
            DataOverwrite(begin() + idx, end(), array.Get() + idx + 1);

            ptr_.swap(array);
            capacity_ = new_capacity;
        } else {
            // Проверка что и в DataOverwrite, но копирование/перенос производится с конца
            if constexpr (std::is_nothrow_move_assignable_v<Type> || !std::is_copy_assignable_v<Type>) {
                std::move_backward(begin() + idx, end(), end() + 1);
            } else {
                std::copy_backward(begin() + idx, end(), end() + 1);
            }
            ptr_[idx] = std::forward<U>(value);
        }

        ++size_;

        return begin() + idx;
    }

    ArrayPtr<Type> ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
void swap(SimpleVector<Type>& lhs, SimpleVector<Type>& rhs) noexcept {
    lhs.swap(rhs);
}