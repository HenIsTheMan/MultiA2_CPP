template <class T>
std::vector<T*> ObjPool<T>::activeObjs;

template <class T>
std::vector<T*> ObjPool<T>::inactiveObjs;

template <class T>
ObjPool<T>::~ObjPool(){
	for(T*& obj: activeObjs){
		if(obj != nullptr){
			delete obj;
			obj = nullptr;
		}
	}

	for(T*& obj: inactiveObjs){
		if(obj != nullptr){
			delete obj;
			obj = nullptr;
		}
	}
}

template <class T>
void ObjPool<T>::Init(const size_t& inactiveSize, const size_t& activeSize){
	activeObjs.reserve(activeSize);

	inactiveObjs.reserve(inactiveSize);
	for(size_t i = 0; i < inactiveSize; ++i){
		inactiveObjs.emplace_back(new T());
	}
}

template <class T>
T* ObjPool<T>::ActivateObj(){
	if(inactiveObjs.empty()){
		assert(false && "inactiveObjs is empty!");
		return nullptr;
	}

	T* const obj = inactiveObjs.front();
	activeObjs.emplace_back(obj);
	inactiveObjs.erase(inactiveObjs.begin());
	return obj;
}

template <class T>
void ObjPool<T>::DeactivateObj(const T* const obj){
	if(activeObjs.empty()){
		assert(false && "activeObjs is empty!");
		return;
	}

	typename std::vector<T*>::iterator iter = std::find(activeObjs.begin(), activeObjs.end(), obj);
	if(iter != activeObjs.end()){
		inactiveObjs.emplace_back(*iter);
		activeObjs.erase(iter);
	}
}