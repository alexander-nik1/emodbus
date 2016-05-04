
#ifndef MY_BIT_VECTOR_HPP
#define MY_BIT_VECTOR_HPP

#include <vector>
#include <bitset>
#include <assert.h>

// это просто вектор с битсетом (std::vector<std::bitset<сколько_бит>>)

template<unsigned int _num_bits = (sizeof(int)*8)>
class bit_vector : public std::vector<std::bitset<_num_bits> >{
public:

	enum { num_bits = _num_bits };

    typedef typename std::bitset<num_bits> word_t;

    typedef typename std::vector<word_t> base_t;

	void resize(size_t sz) {
		base_t::resize(get_full_word_amount(sz));
	}

	size_t size()const {
		return base_t::size() * num_bits;
	}

	void set_all_bits(bool v){
        typename base_t::iterator mf;
        for(mf = typename base_t::begin(); mf != typename base_t::end(); ++mf){
			if(v == true)
				mf->set();
			else
				mf->reset();
		}
	}

	void set_range_bits(bool v, int begin, int end){
        assert(begin <= end);
		const bool has_word = (end-begin) >= num_bits;
		const int wi_begin = _get_word(begin)+1;
		const int wi_end = _get_word(end);
		const int f_end = min(end, wi_begin*num_bits);
		const int s_begin = max(begin, wi_end*num_bits);
		for(int i=begin; i < f_end; ++i)
			setBit(i, v);
        for(int i = wi_begin; (i < wi_end) && has_word; ++i){
			if(v == true)
                typename base_t::begin()[i].set();
//				_Myfirst[i].set();
			else
                typename base_t::begin()[i].reset();
//				_Myfirst[i].reset();
		}
        for(int i=s_begin; i < end; ++i)
			setBit(i, v);
	}

	inline bool operator [] (int i) const {
		return getBit(i);
	}

	typename word_t::reference operator [] (int i){
//#ifdef _DEBUG
		return (base_t::operator [] (_get_word(i))).word_t::operator [] (_get_bit(i));
//#else
//		return _Myfirst[_get_word(i)].word_t::operator [] (_get_bit(i));
//#endif
	}

	void setBit(int i, bool v){
//#ifdef _DEBUG
		(base_t::operator [] (_get_word(i))).word_t::set((_get_bit(i)),v);
//#else
//		_Myfirst[_get_word(i)].word_t::set((_get_bit(i)),v);
//#endif
	}

	bool getBit(int i)const {
//#ifdef _DEBUG
		return (base_t::operator [] (_get_word(i))).word_t::test((_get_bit(i)));
//#else
//		return _Myfirst[_get_word(i)].word_t::test((_get_bit(i)));
//#endif
	}
private:
	static inline int _get_word(int i){
		return (i/num_bits);
	}
	static inline int _get_bit(int i){
		return (i%num_bits);
	}
	static int get_full_word_amount(int _bits_amount){
		return ((_bits_amount+num_bits-1)/num_bits);
	}
};

#endif
