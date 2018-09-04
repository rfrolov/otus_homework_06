#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <map>

/**
 * Вспомогательная функция для vector_to_tuple.
 * @tparam T Тип вектора.
 * @tparam U Тип значения.
 * @tparam Indices Набор индексов от 0...N-1.
 * @param v Ссылка на вектор.
 * @param value Ссылка на значение.
 * @return Кортеж склееный из N элементов вектора и значения.
 */
template<typename T, typename U, size_t... Indices>
auto vector_to_tuple_impl(const std::vector<T> &v, const U &value, std::index_sequence<Indices...>) {
    return std::make_tuple(v[Indices]..., value);
}

/**
 * Преобразовать вектор типа T и значение типа U в кортеж.
 * @tparam N Длина вектора.
 * @tparam T Тип вектора.
 * @tparam U Тип значения.
 * @param v Ссылка на вектор.
 * @param value Ссылка на значение.
 * @return Кортеж склееный из N элементов вектора и значения.
 */
template<size_t N, typename T, typename U>
auto vector_to_tuple(const std::vector<T> &v, const U &value) {
    assert(v.size() >= N);
    return vector_to_tuple_impl(v, value, std::make_index_sequence<N>());
}

/**
 * Тип кортежа из N элементов типа T и одного элемента типа U.
 * @tparam T Тип элементов кортежа.
 * @tparam N Количество элементов.
 */
template<typename T, typename U, size_t N>
struct matrix_tuple_t {
    using type = decltype(std::tuple_cat(std::declval<std::tuple<T>>(), std::declval<typename matrix_tuple_t<T, U, N - 1>::type>()));
};

/// Хвост рекурсии.
template<typename T, typename U>
struct matrix_tuple_t<T, U, 0> {
    using type = std::tuple<U>;
};

/**
 * Класс матрицы.
 * @tparam T Тип элемента.
 * @tparam default_value Значение по умолчани.
 * @tparam N Размерность матрицы.
 */
template<typename T, T default_value, size_t N = 2>
class matrix {
    /// Тип вектора, в котором хронятся координаты элемента матрицы.
    using vector_t          = std::vector<size_t>;

    /// Указатель на вектор, в котором хронятся координаты элемента матрицы.
    using vector_pointer_t  = std::shared_ptr<vector_t>;


    using map_t             = std::map<vector_t, T>;
    using map_iterator_t    = typename std::map<vector_t, T>::iterator;
    using map_pointer_t     = std::shared_ptr<map_t>;
    using tuple_t           = typename matrix_tuple_t<size_t, T, N>::type;

    static_assert(N > 1, "");

    /**
     * Прокси-класс для эмуляции многомерного массива.
     * @tparam Index Уровень вложенности.
     */
    template<size_t Index>
    struct Proxy {

        /**
         * Конструктор.
         * @param map Указатель на map.
         * @param vector Вектор индексов.
         */
        Proxy(map_pointer_t map, vector_pointer_t vector) : map_{std::move(map)}, vector_{std::move(vector)} {}

        /**
         * Реализация оператора [].
         * @param index Значение внутри квадратных скобок.
         * @return Прокси-класс следующей влеженности.
         */
        Proxy<Index + 1> operator[](size_t index) {
            vector_->emplace_back(index);
            return Proxy<Index + 1>{map_, vector_};
        }

        /**
         * Реализация оператора &.
         * @return Значение элемента матрицы.
         */
        operator const T &() const {
            static_assert(Index == N, "");
            auto it = map_->find(*vector_);
            return (it == map_->cend()) ? default_value_ : it->second;
        }

        /**
         * Реализация оператора =.
         * @param value Значение.
         * @return Ссылка на элемент матрицы, куда помещено значение.
         */
        T &operator=(const T value) {
            static_assert(Index == N, "");
            if (value == default_value_) {
                auto it = map_->erase(*vector_);
                return default_value_;
            }

            auto it = map_->emplace(*vector_, value);
            return it.first->second;
        }

    private:
        vector_pointer_t vector_{};
        map_pointer_t    map_{};
        T                default_value_{default_value};
    };

public:

    /// Итератор по элементам матрицы.
    struct iterator : std::iterator<std::forward_iterator_tag, map_iterator_t> {
        /**
         * Конструктор.
         * @param map Указатель на map.
         * @param it Итератор map.
         */
        explicit iterator(map_pointer_t map, map_iterator_t it) : map_(std::move(map)), it_{it} {}

        /**
         * Реализация оператора ++.
         * @return ссылка на итератор.
         */
        iterator &operator++() {
            ++it_;
            return *this;
        }

        /**
         * Реализация оператора *.
         * @return Кортеж состоящий из координат элемента матрицы и его значения.
         */
        tuple_t operator*() {
            return vector_to_tuple<N, size_t, T>((*it_).first, (*it_).second);
        }

        /**
         * Реализация оператора ==.
         * @return true - итераторы совпадают, false - итераторы не совпадают.
         */
        bool operator==(iterator &other) {
            return it_ == other.it_;
        }

        /**
         * Реализация оператора !=.
         * @param other
         * @return true - итераторы не совпадают, false - итераторы совпадают.
         */
        bool operator!=(iterator &other) {
            return !(*this == other);
        }

    private:
        map_pointer_t  map_{};
        map_iterator_t it_{};
    };

    /// Конструктор.
    matrix() {
        map_    = std::make_shared<map_t>();
        vector_ = std::make_shared<vector_t>();
    }

    /**
     * Возвращает прокси-класс для эмуляции многомерного массива.
     * @param index Значение в квадратных скобках.
     * @return Прокси-класс для эмуляции многомерного массива.
     */
    Proxy<1> operator[](size_t index) {
        vector_->clear();
        vector_->emplace_back(index);
        return Proxy<1>{map_, vector_};
    }

    /**
     * Возвращает прокси-класс для эмуляции многомерного массива.
     * @param index Значение в квадратных скобках.
     * @return Прокси-класс для эмуляции многомерного массива.
     */
    const Proxy<1> operator[](size_t index) const {
        vector_->clear();
        vector_->emplace_back(index);
        return Proxy<1>{map_, vector_};
    }

    /**
     * Возвращает реальное количество элементов матрицы.
     * @return
     */
    size_t size() {
        return map_->size();
    }

    /// Возвращает итератор первого элемента матрицы.
    iterator begin() { return iterator(map_, map_->begin()); }

    /// Возвращает итератор следующего за последним элементом матрицы.
    iterator end() { return iterator(map_, map_->end()); }

private:
    map_pointer_t    map_{};
    vector_pointer_t vector_{};
};
