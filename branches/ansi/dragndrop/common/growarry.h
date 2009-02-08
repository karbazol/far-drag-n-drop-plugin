/**
 * @file growarry.h
 * Contains declaration of GrowOnlyArray template class.
 * 
 * $Id$
 */

#ifndef __KARBAZOL_DRAGNDROP_2_0__GROWARRY_H__
#define __KARBAZOL_DRAGNDROP_2_0__GROWARRY_H__

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
    void grow()
    {
        if (_count == _capacity)
        {
            _capacity += 16;
            T* newValues = new T[_capacity];

            size_t i;
            for (i = 0; i < _count; i++)
            {
                newValues[i] = _values[i];
            }

            if (_values)
                delete [] _values;

            _values = newValues;
        }

        _count++;
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
            size_t i;
            for (i = 0; i < _count; i++)
            {
                _values[i] = r._values[i];
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
                delete [] _values;

            _values = new T[_capacity];
        }

        _count = r._count;
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
        }
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
    void size(size_t value)
    {
        while (value > size())
            grow();
        size_t i;
        for (i = value; i < size(); i++)
        {
            _values[i] = T();
        }
        _count = value;
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
    inline void clear(){_count = 0;}

    /**
     * Appends new element to the container.
     */
    T& append(const T& item)
    {
        size_t i = _count;
        grow();
        _values[i] = item;
        return _values[i];
    }
};

#endif // __KARBAZOL_DRAGNDROP_2_0__GROWARRY_H__

