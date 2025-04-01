#pragma once

template<class T>
struct Voxel
{
public:
	T value;

public:
	Voxel(T value = T());
	Voxel(const Voxel& other);
	~Voxel();
};

template<class T>
inline Voxel<T>::Voxel(T value) :
	value(value)
{

}
template<class T>
inline Voxel<T>::Voxel(const Voxel& other)
{
	value = other.value;
}
template<class T>
inline Voxel<T>::~Voxel()
{

}