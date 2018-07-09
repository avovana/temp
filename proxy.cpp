#include <iostream>
#include <string>
#include <tuple>
#include <boost/type_index.hpp>

//#include "proxyData.h"

namespace {
    template<typename T, unsigned N, typename... REST>
    struct generate_tuple_type
    {
        typedef typename generate_tuple_type<T, N-1, T, REST...>::type type;
    };

    template<typename T, typename... REST>
    struct generate_tuple_type<T, 0, REST...>
    {
    typedef std::tuple<REST...> type;
    };

//===================================================

    template <typename T>
    int print(T arg)
    {
        std::cout << arg << " ";
        return 1;
    }

    template <typename... Args>
    void print(Args... args)
    {
        bool arr[] = { print(args)...  };

        (void)arr;
    }

    template <typename... Args>
    void print(size_t CurrSize, const char* str, Args... args)
    {
        std::cout << str << ". CurrSize = " << CurrSize << '\n';
        std::cout << " indexes: ";
        print(args...);
        std::cout << '\n';
    }
}

template<size_t CurrSize, typename Data>
class ProxyData {
    using gen_tuple = typename generate_tuple_type<size_t, CurrSize>::type;

    public:

    template<typename... Args>
    ProxyData(Data data_, Args... args) : data(data_), indexes(args...) {
        print(CurrSize, "ProxyData ctor", args...);
    }

    friend std::ostream &operator<<(std::ostream &output, const ProxyData<CurrSize, Data>& instance) {
    
        if(instance.data->find(instance.indexes) != instance.data->end())
            {
                std::cout << "found" << '\n';
                output << instance.data->operator [](instance.indexes);
            }
        else
            std::cout << "not found" << '\n';
        
        return output;
    }

    auto operator = (std::size_t value)  { //(value_type value)

        (*data)[indexes] = value;
        
        //const auto idx_iter = findElement(*data, row, col);
//
        ////std::cout << "Proxy = " << '\n';
        //if(value == default_value) {
        //    if(idx_iter != data->end())
        //        data->erase(idx_iter);
        //} else {
        //    //std::cout << " In insert" << '\n';
        //    if(idx_iter != data->end())
        //    {
        //        data->erase(idx_iter);
        //        data->insert(cell_type(row, col, value));
        //    } else {
        //        data->insert(cell_type(row, col, value));
        //    }
        //}

        return *this;
    }

    private:

    Data data;
    gen_tuple indexes;
};

template<size_t CurrSize, size_t Size, typename Data, typename = void >
class Proxy {
    using gen_tuple = typename generate_tuple_type<size_t, CurrSize>::type;
    using gen_tuple_next = typename generate_tuple_type<size_t, CurrSize + 1>::type;
    
    public:
    
    template<typename... Args>
    Proxy(Data data_, Args... args) : data(data_), indexes(args...) {
        
        print(CurrSize, "basic template", args...);
    }
    
    auto operator[](const size_t index) const {
        auto new_indexes = std::tuple_cat (indexes, std::tuple<size_t>(index));
        auto seq = std::make_index_sequence<CurrSize + 1>();
        return prepare_for_constr(new_indexes, seq);
    }

    private:
    
    template<size_t... Is>
    auto prepare_for_constr(const gen_tuple_next& tuple, std::index_sequence<Is...>) const {
        return Proxy<CurrSize + 1, Size, Data>(data, std::get<Is>(tuple)...);
    }

    Data data;
    gen_tuple indexes;
};

template<size_t CurrSize, size_t Size, typename Data>
class Proxy<CurrSize, Size, Data, typename std::enable_if<CurrSize == Size - 1>::type> {
    using gen_tuple = typename generate_tuple_type<size_t, CurrSize>::type;
    using gen_tuple_next = typename generate_tuple_type<size_t, CurrSize + 1>::type;
    
    public:
    
    template<typename... Args>
    Proxy(Data data_, Args... args) : data(data_), indexes(args...) {
        print(CurrSize, "specialized template", args...);
        
        //for(auto el : *data)
            //std::cout << "index: " << el.first << ", " << "data: " << el.second << '\n';
    }
    
    auto operator[](const size_t index) const {
        auto new_indexes = std::tuple_cat(indexes, std::tuple<size_t>(index));
        auto seq = std::make_index_sequence<CurrSize + 1>();
        
        return prepare_for_constr(new_indexes, seq);
        
        //return (*data)[new_indexes];
    }

    private:

    template<size_t... Is>
    auto prepare_for_constr(const gen_tuple_next& tuple, std::index_sequence<Is...>) const {
        return ProxyData<CurrSize + 1, Data>(data, std::get<Is>(tuple)...);
    }
    
    Data data;
    gen_tuple indexes;
};
