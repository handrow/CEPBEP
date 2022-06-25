#ifndef COMMON_FAST_VECTOR_H_
#define COMMON_FAST_VECTOR_H_

#include <vector>
#include <cstring>

namespace mut_std {

template <typename T>
class vector : public ::std::vector<T> {
 public:
    typedef typename ::std::vector<T>::value_type               value_type;
    typedef typename ::std::vector<T>::allocator_type           allocator_type;
    typedef typename ::std::vector<T>::reference                reference;
    typedef typename ::std::vector<T>::const_reference          const_reference;
    typedef typename ::std::vector<T>::iterator                 iterator;
    typedef typename ::std::vector<T>::const_iterator           const_iterator;
    typedef typename ::std::vector<T>::size_type                size_type;
    typedef typename ::std::vector<T>::difference_type          difference_type;
    typedef typename ::std::vector<T>::pointer                  pointer;
    typedef typename ::std::vector<T>::const_pointer            const_pointer;
    typedef typename ::std::vector<T>::reverse_iterator         reverse_iterator;
    typedef typename ::std::vector<T>::const_reverse_iterator   const_reverse_iterator;

 private:
    inline static
    void FastSwapElems(pointer el1, pointer el2) {
        static const size_t VALUE_SIZE = sizeof(value_type);
        uint8_t tmpBuff[VALUE_SIZE];  // ignore

        if (el1 != el2) {
            ::memmove(tmpBuff, el1, VALUE_SIZE);
            ::memmove(el1, el2, VALUE_SIZE);
            ::memmove(el2, tmpBuff, VALUE_SIZE);
        }
    }

 public:
    iterator
    erase(iterator position) {
        pointer p = position.base();
        pointer last = this->end().base() - 1;

        FastSwapElems(p, last);
        this->pop_back();
        return position;
    }

};

}  // namespace mut_std

#endif  // COMMON_FAST_VECTOR_H_
