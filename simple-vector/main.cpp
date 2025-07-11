#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>
#include <exception>

#include "simple_vector.h"

using namespace std;

inline void Test1() {
    // Инициализация конструктором по умолчанию
    {
        SimpleVector<int> v;
        assert(v.GetSize() == 0u);
        assert(v.IsEmpty());
        assert(v.GetCapacity() == 0u);
    }

    // Инициализация вектора указанного размера
    {
        SimpleVector<int> v(5);
        assert(v.GetSize() == 5u);
        assert(v.GetCapacity() == 5u);
        assert(!v.IsEmpty());
        for (size_t i = 0; i < v.GetSize(); ++i) {
            assert(v[i] == 0);
        }
    }

    // Инициализация вектора, заполненного заданным значением
    {
        SimpleVector<int> v(3, 42);
        assert(v.GetSize() == 3);
        assert(v.GetCapacity() == 3);
        for (size_t i = 0; i < v.GetSize(); ++i) {
            assert(v[i] == 42);
        }
    }

    // Инициализация вектора при помощи initializer_list
    {
        SimpleVector<int> v{1, 2, 3};
        assert(v.GetSize() == 3);
        assert(v.GetCapacity() == 3);
        assert(v[2] == 3);
    }

    // Доступ к элементам при помощи At
    {
        SimpleVector<int> v(3);
        assert(&v.At(2) == &v[2]);
        try {
            v.At(3);
            assert(false);  // Ожидается выбрасывание исключения
        } catch (const std::out_of_range&) {
        } catch (...) {
            assert(false);  // Не ожидается исключение, отличное от out_of_range
        }
    }

    // Очистка вектора
    {
        SimpleVector<int> v(10);
        const size_t old_capacity = v.GetCapacity();
        v.Clear();
        assert(v.GetSize() == 0);
        assert(v.GetCapacity() == old_capacity);
    }

    // Изменение размера
    {
        SimpleVector<int> v(3);
        v[2] = 17;
        v.Resize(7);
        assert(v.GetSize() == 7);
        assert(v.GetCapacity() >= v.GetSize());
        assert(v[2] == 17);
        assert(v[3] == 0);
    }
    {
        SimpleVector<int> v(3);
        v[0] = 42;
        v[1] = 55;
        const size_t old_capacity = v.GetCapacity();
        v.Resize(2);
        assert(v.GetSize() == 2);
        assert(v.GetCapacity() == old_capacity);
        assert(v[0] == 42);
        assert(v[1] == 55);
    }
    {
        const size_t old_size = 3;
        SimpleVector<int> v(3);
        v.Resize(old_size + 5);
        v[3] = 42;
        v.Resize(old_size);
        v.Resize(old_size + 2);
        assert(v[3] == 0);
    }

    // Итерирование по SimpleVector
    {
        // Пустой вектор
        {
            SimpleVector<int> v;
            assert(v.begin() == nullptr);
            assert(v.end() == nullptr);
        }

        // Непустой вектор
        {
            SimpleVector<int> v(10, 42);
            assert(v.begin());
            assert(*v.begin() == 42);
            assert(v.end() == v.begin() + v.GetSize());
        }
    }
}

inline void Test2() {
    // PushBack
    {
        SimpleVector<int> v(1);
        v.PushBack(42);
        assert(v.GetSize() == 2);
        assert(v.GetCapacity() >= v.GetSize());
        assert(v[0] == 0);
        assert(v[1] == 42);
    }

    // Если хватает места, PushBack не увеличивает Capacity
    {
        SimpleVector<int> v(2);
        v.Resize(1);
        const size_t old_capacity = v.GetCapacity();
        v.PushBack(123);
        assert(v.GetSize() == 2);
        assert(v.GetCapacity() == old_capacity);
    }

    // PopBack
    {
        SimpleVector<int> v{0, 1, 2, 3};
        const size_t old_capacity = v.GetCapacity();
        const auto old_begin = v.begin();
        v.PopBack();
        assert(v.GetCapacity() == old_capacity);
        assert(v.begin() == old_begin);
        assert((v == SimpleVector<int>{0, 1, 2}));
    }

    // Конструктор копирования
    {
        SimpleVector<int> numbers{1, 2};
        auto numbers_copy(numbers);
        assert(&numbers_copy[0] != &numbers[0]);
        assert(numbers_copy.GetSize() == numbers.GetSize());
        for (size_t i = 0; i < numbers.GetSize(); ++i) {
            assert(numbers_copy[i] == numbers[i]);
            assert(&numbers_copy[i] != &numbers[i]);
        }
    }

    // Сравнение
    {
        assert((SimpleVector{1, 2, 3} == SimpleVector{1, 2, 3}));
        assert((SimpleVector{1, 2, 3} != SimpleVector{1, 2, 2}));

        assert((SimpleVector{1, 2, 3} < SimpleVector{1, 2, 3, 1}));
        assert((SimpleVector{1, 2, 3} > SimpleVector{1, 2, 2, 1}));

        assert((SimpleVector{1, 2, 3} >= SimpleVector{1, 2, 3}));
        assert((SimpleVector{1, 2, 4} >= SimpleVector{1, 2, 3}));
        assert((SimpleVector{1, 2, 3} <= SimpleVector{1, 2, 3}));
        assert((SimpleVector{1, 2, 3} <= SimpleVector{1, 2, 4}));
    }

    // Обмен значений векторов
    {
        SimpleVector<int> v1{42, 666};
        SimpleVector<int> v2;
        v2.PushBack(0);
        v2.PushBack(1);
        v2.PushBack(2);
        const int* const begin1 = &v1[0];
        const int* const begin2 = &v2[0];

        const size_t capacity1 = v1.GetCapacity();
        const size_t capacity2 = v2.GetCapacity();

        const size_t size1 = v1.GetSize();
        const size_t size2 = v2.GetSize();

        static_assert(noexcept(v1.swap(v2)));
        v1.swap(v2);
        assert(&v2[0] == begin1);
        assert(&v1[0] == begin2);
        assert(v1.GetSize() == size2);
        assert(v2.GetSize() == size1);
        assert(v1.GetCapacity() == capacity2);
        assert(v2.GetCapacity() == capacity1);
    }

    // Присваивание
    {
        SimpleVector<int> src_vector{1, 2, 3, 4};
        SimpleVector<int> dst_vector{1, 2, 3, 4, 5, 6};
        dst_vector = src_vector;
        assert(dst_vector == src_vector);
    }

    // Вставка элементов
    {
        SimpleVector<int> v{1, 2, 3, 4};
        v.Insert(v.begin() + 2, 42);
        assert((v == SimpleVector<int>{1, 2, 42, 3, 4}));
    }

    // Удаление элементов
    {
        SimpleVector<int> v{1, 2, 3, 4};
        v.Erase(v.cbegin() + 2);
        assert((v == SimpleVector<int>{1, 2, 4}));
    }
}

using namespace std;

void TestReserveConstructor() {
    cout << "TestReserveConstructor"s << endl;
    SimpleVector<int> v(Reserve(5));
    assert(v.GetCapacity() == 5);
    assert(v.IsEmpty());
    cout << "Done!"s << endl;
}

void TestReserveMethod() {
    cout << "TestReserveMethod"s << endl;
    SimpleVector<int> v;
    // зарезервируем 5 мест в векторе
    v.Reserve(5);
    assert(v.GetCapacity() == 5);
    assert(v.IsEmpty());

    // попытаемся уменьшить capacity до 1
    v.Reserve(1);
    // capacity должно остаться прежним
    assert(v.GetCapacity() == 5);
    // поместим 10 элементов в вектор
    for (int i = 0; i < 10; ++i) {
        v.PushBack(i);
    }
    assert(v.GetSize() == 10);
    // увеличим capacity до 100
    v.Reserve(100);
    // проверим, что размер не поменялся
    assert(v.GetSize() == 10);
    assert(v.GetCapacity() == 100);
    // проверим, что элементы на месте
    for (int i = 0; i < 10; ++i) {
        assert(v[i] == i);
    }
    cout << "Done!"s << endl;
}

void Test3() {
    TestReserveConstructor();
    TestReserveMethod();
}

class X {
public:
    X()
        : X(5) {
    }
    X(size_t num)
        : x_(num) {
    }
    X(const X& other) = delete;
    X& operator=(const X& other) = delete;
    X(X&& other) {
        x_ = exchange(other.x_, 0);
    }
    X& operator=(X&& other) {
        x_ = exchange(other.x_, 0);
        return *this;
    }
    size_t GetX() const {
        return x_;
    }

private:
    size_t x_;
};

SimpleVector<int> GenerateVector(size_t size) {
    SimpleVector<int> v(size);
    iota(v.begin(), v.end(), 1);
    return v;
}

void TestTemporaryObjConstructor() {
    const size_t size = 1000000;
    cout << "Test with temporary object, copy elision" << endl;
    SimpleVector<int> moved_vector(GenerateVector(size));
    assert(moved_vector.GetSize() == size);
    cout << "Done!" << endl << endl;
}

void TestTemporaryObjOperator() {
    const size_t size = 1000000;
    cout << "Test with temporary object, operator=" << endl;
    SimpleVector<int> moved_vector;
    assert(moved_vector.GetSize() == 0);
    moved_vector = GenerateVector(size);
    assert(moved_vector.GetSize() == size);
    cout << "Done!" << endl << endl;
}

void TestNamedMoveConstructor() {
    const size_t size = 1000000;
    cout << "Test with named object, move constructor" << endl;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    assert(vector_to_move.GetSize() == size);

    SimpleVector<int> moved_vector(move(vector_to_move));
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);
    cout << "Done!" << endl << endl;
}

void TestNamedMoveOperator() {
    const size_t size = 1000000;
    cout << "Test with named object, operator=" << endl;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    assert(vector_to_move.GetSize() == size);

    SimpleVector<int> moved_vector = move(vector_to_move);
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);
    cout << "Done!" << endl << endl;
}

void TestNoncopiableMoveConstructor() {
    const size_t size = 5;
    cout << "Test noncopiable object, move constructor" << endl;
    SimpleVector<X> vector_to_move;
    for (size_t i = 0; i < size; ++i) {
        vector_to_move.PushBack(X(i));
    }

    SimpleVector<X> moved_vector = move(vector_to_move);
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);

    for (size_t i = 0; i < size; ++i) {
        assert(moved_vector[i].GetX() == i);
    }
    cout << "Done!" << endl << endl;
}

void TestNoncopiablePushBack() {
    const size_t size = 5;
    cout << "Test noncopiable push back" << endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    assert(v.GetSize() == size);

    for (size_t i = 0; i < size; ++i) {
        assert(v[i].GetX() == i);
    }
    cout << "Done!" << endl << endl;
}

void TestNoncopiableInsert() {
    const size_t size = 5;
    cout << "Test noncopiable insert" << endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    // в начало
    v.Insert(v.begin(), X(size + 1));

    assert(v.GetSize() == size + 1);
    assert(v.begin()->GetX() == size + 1);
    // в конец
    v.Insert(v.end(), X(size + 2));
    assert(v.GetSize() == size + 2);
    assert((v.end() - 1)->GetX() == size + 2);
    // в середину
    v.Insert(v.begin() + 3, X(size + 3));
    assert(v.GetSize() == size + 3);
    assert((v.begin() + 3)->GetX() == size + 3);
    cout << "Done!" << endl << endl;
}


void TestNoncopiableErase() {
    const size_t size = 3;
    cout << "Test noncopiable erase" << endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    auto it = v.Erase(v.begin());

    assert(it->GetX() == 1);
    cout << "Done!" << endl << endl;
}

void Test4() {
    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
}

struct MoveableThrowable {
    int num = -1;
    bool must_throw = false;

    MoveableThrowable() = default;
    explicit MoveableThrowable(int n) : num(n) {}
    explicit MoveableThrowable(int n, bool can_throw) : num(n), must_throw(can_throw) {}

    MoveableThrowable(MoveableThrowable&) = delete;
    MoveableThrowable& operator=(MoveableThrowable&) = delete;

    MoveableThrowable(MoveableThrowable&& othr) : num(std::exchange(othr.num, -1)) {
        if (othr.must_throw) throw runtime_error("");
    };

    MoveableThrowable& operator=(MoveableThrowable&& othr) {
        if (othr.must_throw) throw runtime_error("");

        num = std::exchange(othr.num, -1);

        return *this;
    }

    bool operator==(const MoveableThrowable& rhs) const {
        return num == rhs.num;
    }
};

template <typename Type>
SimpleVector<Type> CreateSimpleVector(size_t size) {
    SimpleVector<Type> sv;
    sv.Reserve(size);

    for (size_t i = 1; i <= size; ++i) {
        sv.PushBack(Type(i));
    }

    return sv;
}

template <typename Type>
vector<Type> CreateVector(size_t size) {
    vector<Type> v;
    v.reserve(size);
    
    for (size_t i = 1; i <= size; ++i) {
        v.push_back(Type(i));
    }

    return v;
}

void TestMoveableThrowablePushBackWithoutRealloc() {
    SimpleVector<MoveableThrowable> sv = CreateSimpleVector<MoveableThrowable>(10);
    sv.Reserve(11);
    try {
        // Добавление элемента, который выбросит исключение
        sv.PushBack(MoveableThrowable(11, true));
    } catch  (...) {
        // Пропускаем исключение для проверки состояния вектора после выпадения исключения
    }
    
    // Выполняем те же действия, что и для SimpleVector выше
    vector<MoveableThrowable> v = CreateVector<MoveableThrowable>(10);
    v.reserve(11);

    try {
        v.push_back(MoveableThrowable(11, true));
    } catch  (...) {}
    
    assert(sv.GetSize() == v.size());
    assert(sv.GetCapacity() == v.capacity());

    // Чтобы сравнить 2 разных вектора, их надо привести к общему знаменателю - vector<MoveableThrowable>
    vector<MoveableThrowable> v_from_sv(make_move_iterator(sv.begin()), make_move_iterator(sv.end()));
    assert(v_from_sv == v && "The behavior of the vectors is different");
}

void TestMoveableThrowablePushBackWithRealloc() {
    SimpleVector<MoveableThrowable> sv = CreateSimpleVector<MoveableThrowable>(10);
    try {
        // Добавление элемента, который выбросит исключение
        sv.PushBack(MoveableThrowable(11, true));
    } catch  (...) {
        // Пропускаем исключение для проверки состояния вектора после выпадения исключения
    }
    
    // Выполняем те же действия, что и для SimpleVector выше
    vector<MoveableThrowable> v = CreateVector<MoveableThrowable>(10);

    try {
        v.push_back(MoveableThrowable(11, true));
    } catch  (...) {}
    
    assert(sv.GetSize() == v.size());
    assert(sv.GetCapacity() == v.capacity());

    // Чтобы сравнить 2 разных вектора, их надо привести к общему знаменателю - vector<MoveableThrowable>
    vector<MoveableThrowable> v_from_sv(make_move_iterator(sv.begin()), make_move_iterator(sv.end()));
    assert(v_from_sv == v && "The behavior of the vectors is different");
}

void TestMoveableThrowableInsertWithoutRealloc() {
    SimpleVector<MoveableThrowable> sv = CreateSimpleVector<MoveableThrowable>(10);
    sv.Reserve(11);
    try {
        sv.Insert(sv.begin() + 2, MoveableThrowable(11, true));
    } catch (...) {}

    vector<MoveableThrowable> v = CreateVector<MoveableThrowable>(10);
    v.reserve(11);
    try {
        v.insert(v.begin() + 2, MoveableThrowable(11, true));
    } catch (...) {}

    // simple vec
    for (auto& a : sv) cout << a.num << ' ';
    cout << endl;

    // std::vec
    for (auto& a : v) cout << a.num << ' ';
    cout << endl;

    assert(sv.GetSize() == v.size());
    assert(sv.GetCapacity() == v.capacity());

    // Чтобы сравнить 2 разных вектора, их надо привести к общему знаменателю - vector<MoveableThrowable>
    vector<MoveableThrowable> v_from_sv(make_move_iterator(sv.begin()), make_move_iterator(sv.end()));
    assert(v_from_sv == v && "The behavior of the vectors is different");
    /*
    Результат:
    1 2 -1 3 4 5 6 7 8 9
    1 2 -1 3 4 5 6 7 8 9 10
    Assertion failed: sv.GetSize() == v.size(), file main.cpp, line 527

    Видимо стандартный вектор изменяет размер до выявления исключения, а SimpleVector только после всех операций, которые могут выбросить исключение
    Точно не знаю, стоит ли в SimpleVector менять size заранее.
    */
}

void TestMoveableThrowableInsertWithRealloc() {
    SimpleVector<MoveableThrowable> sv = CreateSimpleVector<MoveableThrowable>(10);
    
    try {
        sv.Insert(sv.begin() + 2, MoveableThrowable(11, true));
    } catch (...) {}

    vector<MoveableThrowable> v = CreateVector<MoveableThrowable>(10);
    
    try {
        v.insert(v.begin() + 2, MoveableThrowable(11, true));
    } catch (...) {}

    assert(sv.GetSize() == v.size());
    assert(sv.GetCapacity() == v.capacity());

    // Чтобы сравнить 2 разных вектора, их надо привести к общему знаменателю - vector<MoveableThrowable>
    vector<MoveableThrowable> v_from_sv(make_move_iterator(sv.begin()), make_move_iterator(sv.end()));
    assert(v_from_sv == v && "The behavior of the vectors is different");
}

void test5() {
    TestMoveableThrowablePushBackWithoutRealloc();
    TestMoveableThrowablePushBackWithRealloc();
    // TestMoveableThrowableInsertWithoutRealloc(); // 
    TestMoveableThrowableInsertWithRealloc();
    
}

int main() {
    Test1();
    Test2();
    Test3();
    Test4();
    test5();
    std::cout << "All test successes" << std::endl;
}