/**
 * @file growarry.h
 * Contains declaration of GrowOnlyArray template class.
 *
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__GROWARRY_H__
#define __KARBAZOL_DRAGNDROP_2_0__GROWARRY_H__

#include <utils.h>

/**
 * Class represants container of T type.
 */
template<class T>
class GrowOnlyArray
{
private:
    T* _values;
    size_t _count;
    size_t _capacity;
    bool grow()
    {
        if (_count == _capacity)
        {
            _capacity += 16;
            T* newValues = new T[_capacity];

            if (!newValues)
            {
                return false;
            }

            size_t i;
            for (i = 0; i < _count; i++)
            {
                newValues[i] = _values[i];
            }

            if (_values)
            {
                delete [] _values;
            }

            _values = newValues;
        }

        _count++;
        return true;
    }
public:
    /**
     * Default constructor.
     */
    GrowOnlyArray(): _values(0), _count(0), _capacity(0){}
    /**
     * Copy constructor.
     */
    GrowOnlyArray(const GrowOnlyArray& r): _values(0), _count(r._count), _capacity(r._capacity)
    {
        if (_capacity)
        {
            _values = new T[_capacity];
            if (_values)
            {
                size_t i;
                for (i = 0; i < _count; i++)
                {
                    _values[i] = r._values[i];
                }
            }
            else
            {
                _count = _capacity = 0;
            }
        }
    }
    /**
     * Asignment operator.
     */
    GrowOnlyArray& operator=(const GrowOnlyArray& r)
    {
        if (_capacity < r._capacity)
        {
            _capacity = r._capacity;
            if (_values)
            {
                delete [] _values;

                _values = new T[_capacity];

                if (!_values)
                {
                    _capacity = _count = 0;
                    return *this;
                }
            }
        }

        if (!r._values)
        {
            if (_values)
            {
                delete [] _values;
                _values = 0;
            }

            _count = 0;
            return *this;
        }

        _count = r._count;
        ASSERT(_capacity >= r._capacity);
        ASSERT(_values);

        size_t i;
        for (i = 0; i < _count; i++)
        {
            _values[i] = r._values[i];
        }
        return *this;
    }
    ~GrowOnlyArray()
    {
        if (_values)
        {
            delete [] _values;
            _values = 0;
        }

        _count = _capacity = 0;
    }
    /**
     * Returns number of elements stored in the container.
     */
    size_t size() const
    {
        return _count;
    }

    /**
     * Allows to modify a size of an array
     */
    size_t size(size_t value)
    {
        while (value > size())
        {
            if (!grow())
            {
                return _count;
            }
        }

        size_t i;
        for (i = value; i < size(); i++)
        {
            _values[i] = T();
        }
        _count = value;
        return _count;
    }

    /**
     * Returns const reference to i-th element in the container.
     */
    const T& operator[](size_t i) const
    {
        return _values[i];
    }

    /**
     * Returns reference to i-th element in the container.
     */
    T& operator[](size_t i)
    {
        return _values[i];
    }

    /**
     * Deletes i-th element from the container.
     */
    size_t deleteItem(size_t i)
    {
        if (i < _count)
        {
            for (; i < _count - 1; i++)
            {
                _values[i] = _values[i+1];
            }

            _values[--_count] = T();
        }

        return _count;
    }

    /**
     * Deletes all elements form the container.
     */
    inline void clear(){size(0);}

    /**
     * Appends new element to the container.
     */
    T& append(const T& item)
    {
        size_t i = _count;
        if (!grow())
        {
            /** @todo raise "Out of memory" exception */
            return _values[0];
        }
        _values[i] = item;
        return _values[i];
    }
};

#endif // __KARBAZOL_DRAGNDROP_2_0__GROWARRY_H__

