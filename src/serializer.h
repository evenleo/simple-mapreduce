/**
* 序列化反序列化
* 仅支持有实际类型的数据结构，不支持含有指针的数据结构
*/
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <vector>
#include <algorithm>
#include <memory>
#include <string.h>

namespace even {

// 存储数据流的容器
class StreamBuffer : public std::vector<char> {
public:
    StreamBuffer() : curpos_(0) {}
    StreamBuffer(const char* s, size_t len) : curpos_(0)
    {
        insert(begin(), s, s + len);
    }
    const char* data() { return &(*this)[0]; }
    const char* curdata() { return &(*this)[curpos_]; }
    size_t cursize() const { return size() - curpos_; }
    bool eof() const { return curpos_ >= size(); }
    void offset(int k) { curpos_ += k; }
    void input(const char* s, size_t len) { insert(end(), s, s + len); }
    void reset() { curpos_ = 0; }

private:
    size_t curpos_;
};

class Serializer {
public:
    enum ByteOrder 
    { 
        BigEndian,    // 大端
        LittleEndian  // 小端
    };

public:
    Serializer() : byteOrder_(LittleEndian) {}
    Serializer(StreamBuffer s, int byteOrder = LittleEndian)
        : streamBuffer_(s), byteOrder_(byteOrder) {}
    
    template <typename T>
    void input_type(T t);
    
    template <typename T>
    void output_type(T& t);

    	template<typename Tuple, std::size_t Id>
	void getv(Serializer& ds, Tuple& t) {
		ds >> std::get<Id>(t);
	}

    void reset(){
		streamBuffer_.reset();
	}

    void clear() 
    { 
        streamBuffer_.clear();
        reset();
    }
    void input(const char* data, int len)
    {
        streamBuffer_.input(data, len);
    }

    template<typename Tuple, std::size_t... I>
	Tuple get_tuple(std::index_sequence<I...>) {
		Tuple t;
		((getv<Tuple, I>(*this, t)), ...);
		return t;
	}

    template<typename T>
	Serializer& operator >> (T& i)
    {
		output_type(i); 
		return *this;
	}

	template<typename T>
	Serializer& operator << (T i)
    {
		input_type(i);
		return *this;
	}
    
    const char* data() { return streamBuffer_.curdata(); }
    size_t size() const { return streamBuffer_.cursize(); }

private:
    
    void byte_order(char* s, int len)
    {
        if (byteOrder_ == BigEndian)
            std::reverse(s, s + len);
    }

private:
    int byteOrder_;
    StreamBuffer streamBuffer_;
};

template <typename T>
inline void Serializer::input_type(T t)
{
    int len = sizeof(T);
    std::shared_ptr<char> ptr(new char[len], [](char* p) { delete[] p; }); 
    const char* p = reinterpret_cast<const char*>(&t);
    memcpy(ptr.get(), p, len);
    byte_order(ptr.get(), len);
    streamBuffer_.input(ptr.get(), len);
}

// 偏特化
template <>
inline void Serializer::input_type(std::string t)
{
    // 先存入字符串长度
    uint16_t len = t.size();
    char* p = reinterpret_cast<char*>(&len);
    byte_order(p, sizeof(uint16_t));
    streamBuffer_.input(p, sizeof(uint16_t));

    // 存入字符串
    if (len == 0) return;
    std::shared_ptr<char> ptr(new char[len], [](char* p) { delete[] p; }); 

    memcpy(ptr.get(), t.data(), len);
    byte_order(ptr.get(), len);
    streamBuffer_.input(ptr.get(), len);
}

template<>
inline void Serializer::input_type(const char* in)
{
	input_type<std::string>(std::string(in));
}

template <typename T>
inline void Serializer::output_type(T& t)
{
    int len = sizeof(T);
    std::shared_ptr<char> ptr(new char[len], [](char* p) { delete[] p; }); 

    if (!streamBuffer_.eof())
    {
        memcpy(ptr.get(), streamBuffer_.curdata(), len);
        streamBuffer_.offset(len);
        byte_order(ptr.get(), len);
        t = *reinterpret_cast<T*>(&ptr.get()[0]);
    }
}

// 偏特化
template <>
inline void Serializer::output_type(std::string& t)
{
    // 先取字符串长度
    int strlen = sizeof(uint16_t);
    std::shared_ptr<char> ptr(new char[strlen], [](char* p) { delete[] p; }); 

    memcpy(ptr.get(), streamBuffer_.curdata(), strlen);
    byte_order(ptr.get(), strlen);
    streamBuffer_.offset(strlen);
    int len = *reinterpret_cast<uint16_t*>(&ptr.get()[0]);

    // 再取字符串
    if (len == 0) return;
    t.insert(t.begin(), streamBuffer_.curdata(), streamBuffer_.curdata() + len);
    streamBuffer_.offset(len);
}

} // namespace even

#endif