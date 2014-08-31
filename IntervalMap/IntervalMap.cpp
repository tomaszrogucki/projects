#include <assert.h>
#include <map>
#include <limits>
#include <random>
#include <memory>
#include <iterator>
#include <iostream>


// interval_map<K,V> is a data structure that efficiently associates intervals of keys of type K with values of type V. 
// Your task is to implement the assign member function of this data structure, which is outlined below. 

// interval_map<K, V> is implemented on top of std::map. In case you are not entirely sure which functions std::map provides,
// what they do and which guarantees they provide, we have attached an excerpt of the C++1x draft standard at the end of this
// file for your convenience. 

// Each key-value-pair (k,v) in the m_map member means that the value v is associated to the interval from k (including) to
// the next key (excluding) in m_map.
// Example: the std::map (0,'A'), (3,'B'), (5,'A') represents the mapping
// 0 -> 'A'
// 1 -> 'A'
// 2 -> 'A'
// 3 -> 'B'
// 4 -> 'B'
// 5 -> 'A' 
// 6 -> 'A'
// 7 -> 'A'
// ... all the way to numeric_limits<key>::max()

// The representation in m_map must be canonical, that is, consecutive map entries must not have the same value: 
// ..., (0,'A'), (3,'A'), ... is not allowed.
// Initially, the whole range of K is associated with a given initial value, passed to the constructor.

// Key type K
// - besides being copyable and assignable, is less-than comparable via operator< ;
// - is bounded below, with the lowest value being std::numeric_limits<K>::min();
// - does not implement any other operations, in particular no equality comparison or arithmetic operators.

// Value type V
// - besides being copyable and assignable, is equality-comparable via operator== ;
// - does not implement any other operations.

template<class K, class V>
class interval_map {
	friend void IntervalMapTest();
	
private:	
	std::map<K,V> m_map;

public:
	// constructor associates whole range of K with val by inserting (K_min, val) into the map
	interval_map( V const& val) {
		m_map.insert(m_map.begin(),std::make_pair(std::numeric_limits<K>::min(),val));
	};

	// Assign value val to interval [keyBegin, keyEnd). 
	// Overwrite previous values in this interval. Do not change values outside this interval.
	// Conforming to the C++ Standard Library conventions, the interval includes keyBegin, but excludes keyEnd.
	// If !( keyBegin < keyEnd ), this designates an empty interval, and assign must do nothing.
	void assign( K const& keyBegin, K const& keyEnd, const V& val ) {
		// TODO:
		// Implement this function.
		// Your implementation is graded by these criteria in this order:
		// - correctness (of course): In particular, pay attention to the validity of iterators. It is illegal to dereference
		//   end iterators. Consider using a checking STL implementation such as the one shipped with Visual C++.
		// - simplicity: simple code is easy to understand and maintain, which is important in large projects. 
		//   To write a simple solution, you need to exploit the structure of the problem.
		//   Use functions of std::map wherever you can.
		// - running time: Imagine your implementation is part of a library, so it should be big-O optimal.
		//   In addition, 
		//   * do not make big-O more operations on K and V than necessary, because you 
		//     do not know how fast operations on K/V are; remember that constructions, destructions and assignments 
		//     are operations as well.
		//   * do not make more than two operations of amortized O(log N), in contrast to O(1), running time, where N is the number of elements in m_map.
		//     Any operation that needs to find a position in the map "from scratch", without being given a nearby position, is such an operation.
		//   Otherwise favor simplicity over minor speed improvements.
		// - time to turn in the solution: you should not take longer than a day, and you may be faster.
		//   But don't rush, I would not give you this assignment if it were trivial.
		//
		// You must develop the solution yourself. You may not let others help you or search for existing solutions. Of course
		// you may use any documentation of the C++ language or the C++ Standard Library.
		// Do not give your solution to others or make it public. It will entice others into
		// sending in plagiarized solutions.


		// Assign only if the interval boundaries are valid.
		if(keyBegin < keyEnd) {
			// Find map item with key greater or equal to the new interval's upper boundary.
			// Complexity: logarithmic
			auto end_next_it = m_map.lower_bound(keyEnd);
			auto next_interval_it = end_next_it;

			// If found item's key is equal to the new interval's upper boundary nothing has to be done as these intervals are contiguous,
			// otherwise create a new map item with a key equal to the new interval's upper boundary and value of the previous map item.
			// This way values outside of the new interval's upper boundary will not be changed.
			// In this if statement the iterator can be safely dereferenced, because if the second condition is reached that means
			// the first condition was not satisfied, thus the iterator does not point to the after-the-last item in the map.
			if(end_next_it == std::end(m_map) || keyEnd < end_next_it->first) {
				auto end_previous_it = end_next_it;
				--end_previous_it;
				// New map item will lay between end_previous and end_next items,
				// its value will be the one of the previous item.
				// Save the iterator to the new item for further use in case a range of items has to be erased.
				// Knowing that the new item is directly preceding the next item, iterator to the next item can be provided as a parameter to insert(), thus:
				// Complexity: amortized constant
				next_interval_it = m_map.insert(end_next_it,std::make_pair(keyEnd,end_previous_it->second));
			}

			// Insert a map item reflecting the new interval's lower boundary.
			// Complexity: logarithmic
			auto ins_begin_pair = m_map.insert(std::make_pair(keyBegin,val));

			// If the second parameter of the result pair is true, a new map item has been created.
			bool begin_inserted = ins_begin_pair.second;
			bool re_insert_begin = false; // Flag used later.

			// To overwrite previous values in this interval, all map items falling into it have to be erased.
			// Special care has to be taken to assure that the map stays canonical.
			// Let us temporarily assign the first element to be erased to the newly inserted map item reflecting
			// the beginning of the interval (or already existing item with the same key).
			auto erase_begin_it = ins_begin_pair.first;

			// Now the value of the new interval has to be compared to the value of the preceding interval.
			// If they are equal there lower boundary of the new interval should not be stored in the map.
			if(begin_inserted) {
				// Keep the lower boundary of the new interval in the map only if it is the first element in the map
				// or if the value of the preceding interval is different.
				// In this if statement the second condition safely dereferences a valid iterator as it is evaluated
				// only if the first condition is false.
				if(erase_begin_it == std::begin(m_map) || std::prev(erase_begin_it)->second != val)
					++erase_begin_it;
			}
			else {
				// If a map item reflecting the lower boundary of the new interval has not been created due to existence
				// of another item with the same key, this other item has to be erased and the lower boundary reinserted.
				re_insert_begin = true;
				// In case the previous interval has the same value, the lower boundary of the new interval should not be stored in the map.
				if(erase_begin_it != std::begin(m_map) && std::prev(erase_begin_it)->second == val)
					re_insert_begin = false;
			}

			// Assign the last element to be erased to the lower boundary of the next interval.
			auto erase_end_it = next_interval_it;

			// If the value of the next interval is the same as the new interval's value, the lower boundary
			// of the next interval should be erased from the map.
			if(erase_end_it->second == val) {
				++erase_end_it;
			}

			// Erase the unnecessary items from the map.
			// Complexity: amortized O(N) where N is the number of items erased
			m_map.erase(erase_begin_it,erase_end_it);

			// If the lower boundary has to be reinserted it will directly precede the first unerased item (indicated by
			// the erase_end iterator which stays valid after calling the erase() method).
			// Complexity: amortized constant
			if(re_insert_begin) {
				m_map.insert(erase_end_it,std::make_pair(keyBegin,val));
			}
		}
	}

	// look-up of the value associated with key
	V const& operator[]( K const& key ) const {
		return ( --m_map.upper_bound(key) )->second;
	}
};

// Provide a function IntervalMapTest() here that tests the functionality of the interval_map,
// for example using a map of unsigned int intervals to char.
// Many solutions we receive are incorrect. Consider using a randomized test to discover 
// the cases that your implementation does not handle correctly.

void IntervalMapTest() {

	// This hard-coded test is meant to verify basic cases of interval overlapping.

	// Initialise the interval_map with 'a' characters.
	interval_map<int,char> im('a');

	// Create first interval.
	// Expected behaviour:
	// - keys greater or equal 100 have 'a' value,
	// - keys lower than 0 have 'a' value,
	// - other keys have 'b' value.
	im.assign(0,100,'b');
	assert(im[0] == 'b');
	assert(im[50] == 'b');
	assert(im[-10] == 'a');
	assert(im[100] == 'a');
	assert(im[110] == 'a');

	// Create second interval with upper boundary equal to the upper boundary of the previous interval.
	// Expected behaviour:
	// - keys greater or equal 100 have not changed their value ('a'),
	// - keys greater or equal 50 and lower than 100 have 'c' value.
	im.assign(50,100,'c');
	assert(im[50] == 'c');
	assert(im[70] == 'c');
	assert(im[100] == 'a');

	// Create third interval with lower boundary equal to the lower boundary of the second interval and the upper boundary lower than the upper boundary of the second interval.
	// Expected behaviour:
	// - keys greater or equal 50 and lower than 60 have value 'd',
	// - keys greater or equal than 60 have 'c' value.
	im.assign(50,60,'d');
	assert(im[50] == 'd');
	assert(im[60] == 'c');
	assert(im[80] == 'c');

	// Create fourth interval encompassing the third interval.
	// Expected behaviour:
	// - keys from the third interval have value 'e'.
	im.assign(30,80,'e');
	assert(im[30] == 'e');
	assert(im[50] == 'e');
	assert(im[80] == 'c');

	// Create empty interval.
	// Expected behaviour:
	// - nothing changes.
	im.assign(100,0,'f');
	assert(im[100] == 'a');
	assert(im[0] == 'b');



	// This randomised test will create a map of sample int keys and pointers to the respective char values.
	// Following, it will randomly generate int intervals and assign them char values. For each generated interval
	// the test will assign it to the interval_map as well as it will check each item in the sample map if its key falls
	// into the interval and assign it the new value if it does.
	// As the last step, for each key in the sample map the test will compare if its value corresponds to the value from
	// the interval_map for the currently processed key.

	// These values can be modified:
	char ch_init = 'A'; // a character to initialise maps
	std::size_t max_samples = 100; // maximum number of samples
	std::size_t max_test_steps = 1000; // maximum number of generated intervals
	int key_min = std::numeric_limits<int>::min(); // minimum value of generated key
	int key_max = std::numeric_limits<int>::max(); // maximum value of generated key
	char value_min = 65; // minimum of generated value
	char value_max = 90; // maximum of generated value
	// To increase probability of cases where generated intervals would be contiguous, it is recommended
	// to set key_min and key_max value so that their difference is lower than the max_test_steps, e.g.:
	// key_min = 0 and key_max = 500
	// Values range has been limited to 26 characters to increase the probability of intervals with the same value,
	// thus assuring correctness in terms of the canonical property of the map.


	// Random number generators. Produce uniform distributed numbers in the range of +-numeric_limits for int and char.
	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_int_distribution<> dis_int(key_min, key_max);
	std::uniform_int_distribution<char> dis_char(value_min, value_max);

	// Initialise interval_map with a chosen character.
	interval_map<int,char> im2(ch_init);

	// Populate the samples map with the chosen character to mimic the initial state of interval_map.
	std::map<int,std::shared_ptr<char>> samples;
	for(std::size_t i=0; i < max_samples; i++) {
		std::shared_ptr<char> char_ptr(new char(ch_init));
		// If the generated key already exists in the samples map, it will simply be ignored, thus reducing the total number of samples.
		samples.insert(std::make_pair(dis_int(gen),char_ptr));
	}

	// Randomly generate intervals and their values.
	for(std::size_t i=0; i < max_test_steps; i++) {
		char val = dis_char(gen);
		int min = dis_int(gen);
		int max = dis_int(gen);
		// This also test empty interval cases (e.g. where max < min).

		// Assign the generated interval and its value to the interval_map.
		im2.assign(min,max,val);

		// Find all samples which keys fall into the generated interval and set their corresponding values.
		for(const auto& sample : samples) {
			if(!(sample.first < min) && sample.first < max) {
				*(sample.second) = val;
			}
		}
	}

	// Carry out the actual test by comparing values for each key from the samples map with the values for respective keys in the interval_map.
	for(const auto& sample : samples) {
		assert(*(sample.second) == im2[sample.first]);
	}

	// Make sure the map is canonical.
	if(im2.m_map.size() > 1) {
		for(auto it = std::begin(im2.m_map); std::next(it) != std::end(im2.m_map); ++it) {
			// Verify if the consecutive map items have different values.
			assert(it->second != std::next(it)->second);
		}
	}

	std::cout << "Test has completed successfully!" << std::endl;
}

int main(int argc, char* argv[]) {
	IntervalMapTest();
	return 0;
}

/*
The following paragraphs from the final draft of the C++1x ISO standard describe the available 
operations on a std::map container, their effects and their complexity.

23.2.1 General container requirements 

�1	Containers are objects that store other objects. They control allocation and deallocation of 
these objects through constructors, destructors, insert and erase operations.

�6	begin() returns an iterator referring to the first element in the container. end() returns 
an iterator which is the past-the-end value for the container. If the container is empty, 
then begin() == end();

24.2.1 General Iterator Requirements

�1	Iterators are a generalization of pointers that allow a C++ program to work with different 
data structures.

�2	Since iterators are an abstraction of pointers, their semantics is a generalization of most 
of the semantics of pointers in C++. This ensures that every function template that takes 
iterators works as well with regular pointers.

�5	Just as a regular pointer to an array guarantees that there is a pointer value pointing past 
the last element of the array, so for any iterator type there is an iterator value that points 
past the last element of a corresponding sequence. These values are called past-the-end values. 
Values of an iterator i for which the expression *i is defined are called dereferenceable. 
The library never assumes that past-the-end values are dereferenceable. Iterators can also have 
singular values that are not associated with any sequence. [ Example: After the declaration of 
an uninitialized pointer x (as with int* x;), x  must always be assumed to have a singular 
value of a pointer. �end example ] Results of most expressions are undefined for singular 
values; the only exceptions are destroying an iterator that holds a singular value, the 
assignment of a non-singular value to an iterator that holds a singular value, and, for 
iterators that satisfy the DefaultConstructible requirements, using a value-initialized 
iterator as the source of a copy or move operation.

�10 An invalid iterator is an iterator that may be singular. (This definition applies to pointers, 
since pointers are iterators. The effect of dereferencing an iterator that has been invalidated 
is undefined.)

23.2.4 Associative containers

�1	Associative containers provide fast retrieval of data based on keys. The library provides four 
basic kinds of associative containers: set, multiset, map and multimap.

�4	An associative container supports unique keys if it may contain at most one element for each key. 
Otherwise, it supports equivalent keys. The set and map classes support unique keys; the multiset 
and multimap classes support equivalent keys.

�5	For map and multimap the value type is equal to std::pair<const Key, T>. Keys in an associative 
container are immutable.

�6	iterator of an associative container is of the bidirectional iterator category.
(i.e., an iterator i can be incremented and decremented: ++i; --i;)

�9	The insert member functions (see below) shall not affect the validity of iterators and references 
to the container, and the erase members shall invalidate only iterators and references to the erased 
elements.

�10	The fundamental property of iterators of associative containers is that they iterate through the 
containers in the non-descending order of keys where non-descending is defined by the comparison 
that was used to construct them.

Associative container requirements (in addition to general container requirements):

std::pair<iterator, bool> insert(std::pair<const key_type, T> const& t)
Effects: Inserts t if and only if there is no element in the container with key equivalent to the key of t. 
The bool component of the returned pair is true if and only if the insertion takes place, and the iterator 
component of the pair points to the element with key equivalent to the key of t.
Complexity: logarithmic

iterator insert(const_iterator p, std::pair<const key_type, T> const& t)
Effects: Inserts t if and only if there is no element with key equivalent to the key of t in containers with
unique keys. Always returns the iterator pointing to the element with key equivalent to the key of t.
Complexity: logarithmic in general, but amortized constant if t is inserted right before p.

size_type erase(key_type const& k)  
Effects: Erases all elements in the container with key equivalent to k. Returns the number of erased elements.
Complexity: log(size of container) + number of elements with key k

iterator erase(const_iterator q) 
Effects: Erases the element pointed to by q. Returns an iterator pointing to the element immediately following 
q prior to the element being erased. If no such element exists, returns end().
Complexity: Amortized constant

iterator erase(const_iterator q1, const_iterator q2)
Effects: Erases all the elements in the left-inclusive and right-exclusive range [q1,q2). Returns q2.
Complexity: Amortized O(N) where N has the value distance(q1, q2).

void clear() 
Effects: erase(begin(), end())
Post-Condition: empty() returns true
Complexity: linear in size().

iterator find(key_type const& k);
Effects: Returns an iterator pointing to an element with the key equivalent to k, or end() if such an element is not found
Complexity: logarithmic

size_type count(key_type const& k) 
Effects: Returns the number of elements with key equivalent to k
Complexity: log(size of map) + number of elements with key equivalent to k

iterator lower_bound(key_type const& k)
Effects: Returns an iterator pointing to the first element with key not less than k, or end() if such an element is not found.
Complexity: logarithmic

iterator upper_bound(key_type const& k)
Effects: Returns an iterator pointing to the first element with key greater than k, or end() if such an element is not found.
Complexity: logarithmic

23.4.1 Class template map

�1 	A map is an associative container that supports unique keys (contains at most one of each key value) and provides 
for fast retrieval of values of another type T based on the keys. The map class supports bidirectional iterators.

23.4.1.2 map element access

T& operator[](const key_type& x);
Effects: If there is no key equivalent to x in the map, inserts value_type(x, T()) into the map. 
Returns: A reference to the mapped_type corresponding to x in *this.
Complexity: logarithmic.

T& at(const key_type& x);
const T& at(const key_type& x) const;
Returns: A reference to the element whose key is equivalent to x.
Throws: An exception object of type out_of_range if no such element is present.
Complexity: logarithmic.
*/
