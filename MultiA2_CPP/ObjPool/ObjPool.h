#pragma once

#include <cassert>
#include <vector>

#include "Singleton.h"

template <class T>
class ObjPool final: public Singleton<ObjPool<T>>{
	friend Singleton<ObjPool<T>>;
public:
	~ObjPool<T>();

	void Init(const size_t& inactiveSize, const size_t& activeSize);

	const std::vector<T*>& GetActiveObjs() const{ //Hmmm
		return activeObjs;
	}

	T* ActivateObj();
	void DeactivateObj(const T* const obj);
private:
	static std::vector<T*> activeObjs;
	static std::vector<T*> inactiveObjs;

	ObjPool<T>() = default;
};

#include "ObjPool.inl"