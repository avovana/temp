// Sparse Matrix
// v 0.97
#include <iostream>
#include <memory>
#include <map>
#include <utility>

#include "proxy.h"

template <typename ElementType, std::size_t Size = 2>
class SparseMatrix {

private:
    using IndexType = typename generate_tuple_type<size_t, Size>::type;
    using Container = std::map<IndexType, ElementType>;
    
    using iterator = typename Container::iterator;
    using DataPointerType = std::shared_ptr<Container>;

    DataPointerType data;

public:
    SparseMatrix() : data(new Container)
    {
        //data->insert(std::pair<IndexType, ElementType>(IndexType(5,7), 100));
        std::cout << "Matrix ctor " << '\n';
    }

    auto operator [] (std::size_t index) const
    {
        return Proxy<1, Size, DataPointerType>(data, index);
    }

    auto size() const
    {
        return data->size();
    }

    iterator begin() const
    {
        return data->begin();
    }

    iterator end() const
    {
        return data->end();
    }
};
