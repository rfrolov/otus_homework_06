#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <map>

/**
 * Класс матрицы.
 * @tparam T Тип элемента.
 * @tparam default_value Значение по умолчани.
 * @tparam N Размерность матрицы.
 */
template<typename T, T default_value, size_t N = 2>
class matrix {
    static_assert(N > 1, "");

    using vector_t          = std::vector<size_t>;
    using vector_pointer_t  = std::shared_ptr<vector_t>;
    using map_t             = std::map<vector_t, T>;
    using map_iterator_t    = typename std::map<vector_t, T>::iterator;
    using map_pointer_t     = std::shared_ptr<map_t>;

    template<size_t... Indices>
    static auto vector_to_tuple_impl(const vector_t &v, const T &value, std::index_sequence<Indices...>) {
        return std::make_tuple(v[Indices]..., value);
    }

    template<size_t Num>
    static auto vector_to_tuple(const vector_t &v, const T &value) {
        assert(v.size() >= Num);
        return vector_to_tuple_impl(v, value, std::make_index_sequence<Num>());
    }


    /**
     * Прокси-класс для эмуляции многомерного массива.
     * @tparam Index Уровень вложенности.
     */
    template<size_t Index, typename Fake = void>
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

    private:
        vector_pointer_t vector_{};
        map_pointer_t    map_{};
        T                default_value_{default_value};
    };

    /**
     * Прокси-класс для эмуляции многомерного массива (хвост рекурсии).
     */
    template<typename Fake>
    struct Proxy<N, Fake> {
        /**
         * Конструктор.
         * @param map Указатель на map.
         * @param vector Вектор индексов.
         */
        Proxy(map_pointer_t map, vector_pointer_t vector) : map_{std::move(map)}, vector_{std::move(vector)} {}

        /**
         * Реализация оператора &.
         * @return Значение элемента матрицы.
         */
        operator const T &() const {
            auto it = map_->find(*vector_);
            return (it == map_->cend()) ? default_value_ : it->second;
        }

        /**
         * Реализация оператора =.
         * @param value Значение.
         * @return Ссылка на элемент матрицы, куда помещено значение.
         */
        T &operator=(const T value) {
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
    class iterator : std::iterator<std::forward_iterator_tag, map_iterator_t> {
        template<size_t Num, typename Fake = void>
        struct iterator_tuple_t {
            using type = decltype(std::tuple_cat(std::declval<std::tuple<std::size_t>>(), std::declval<typename iterator_tuple_t<Num - 1, Fake>::type>()));
        };

        template<typename Fake>
        struct iterator_tuple_t<0, Fake> {
            using type = std::tuple<T>;
        };

        using tuple_t = typename iterator_tuple_t<N>::type;
    public:
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
            return vector_to_tuple<N>((*it_).first, (*it_).second);
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
