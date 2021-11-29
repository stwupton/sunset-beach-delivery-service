#pragma once 

#include "types/array.hpp"
#include "types/core.hpp"

// It's common for us to identitfy when an object in an array is redundant while
// iterating through. Instead of re-organising the array so that everything is
// contigious in a sperate process, we can do this while iterating through using
// the `Reducer`. 
//
// Example:
// 
//     struct Foo { u8 bar; };
//
//     Array<Foo, 3> foos;
//     foos.push({ 0 });     
//     foos.push({ 1 });     
//     foos.push({ 2 });     
//
//     // Array before:
//     //	    length: 3
//     //	    data: { 0, 1, 2 }
//     
//     Reducer reducer(&foos);
// 
//     for (const Foo &foo : foos) {
//         reducer.next(&foo);
//         if (foo.bar == 1) {
//             reducer.remove();
//         }
//     }
//     
//     reducer.finish();
// 
//     // Array after:
//     //	    length: 2
//     //	    data: { 0, 2, 2 }
//
template<typename T, size_t Size>
struct Reducer {
	Array<T, Size> *array;
	T *current;
	s64 offset = 0;

	Reducer(Array<T, Size> *array) : array(array) {}

	void next(T *x) {
		this->relocateCurrent();
		this->current = x;
	}

	void remove() {
		this->offset--;
		this->current = nullptr;
	}

	void finish() {
		this->relocateCurrent();

		this->array->length += this->offset;

		this->array = nullptr;
		this->offset = 0;
		this->current = nullptr;
	}

protected:
	void relocateCurrent() {
		if (this->current != nullptr && this->offset != 0) {
			T *to = this->current + this->offset;
			*to = *this->current;
		}
	}
};