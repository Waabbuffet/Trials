#pragma once

#ifndef trials_heapObjectPool00
#define trials_heapObjectPool00

#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <opencv2/core/mat.hpp>

namespace Trials
{
	template<class T> class Requirement
	{
	public:
		virtual bool satisfy(T* elementptr) { return true; }

		virtual T* initialize() = 0;

		virtual void reuse(T* elementptr) = 0;
	};

	class OpenCVMatrixRequirement : public Requirement<cv::Mat>
	{
	public:
		OpenCVMatrixRequirement(int sizeX, int sizeY, int sizeZ, int type);

		virtual bool satisfy(cv::Mat* elementptr);

		virtual cv::Mat* initialize();

		virtual void reuse(cv::Mat* elementptr);

		int sizeX, sizeY, sizeZ;
		int type;
	};

	/* This clas will create a simple object Pool. The object pool makes a distinction between
	   Disown and destroying
	   The goals of this object pool.
		- A way to return objects based on requirements
		- Expand the pool if you cannot return an object
		- Allow objects to live until the destory function is called
		- Only disown objects if the pointer goes out of scope
	*/
	template<class T, int maxSize>class HeapObjectPool
	{
	public:
		//We must acquire based on requirements
		T* acquire(Requirement<T> *r)
		{
			int lastFreeIndex = -1;
			mapLock.lock();
			std::cout << "Pool Size: " << objectOwner.size() << std::endl;
			for (int i = 0; i < objectOwner.size(); i++)
			{
				if (!objectOwner[i])
				{
					if (theObjectPtrs[i] == nullptr)
					{
						if (lastFreeIndex == -1)
						{
							
							T* tmp = lockAndCreateNew(i, r);
							ptrToIndex.insert(std::pair<T*, int>(tmp, i));
							mapLock.unlock();
							return tmp;
						}
						else
						{
							T* temp = lockAndUseOld(lastFreeIndex, r);
							r->reuse(temp);
							ptrToIndex.insert(std::pair<T*, int>(temp, lastFreeIndex));
							std::cout << "Pool Size: " << objectOwner.size() << std::endl;
							mapLock.unlock();
							return temp;
						}
					}
					else
					{
						if (r->satisfy(theObjectPtrs[i]))
						{
							T* tmp = lockAndUseOld(i, r);
							ptrToIndex.insert(std::pair<T*, int>(tmp, i));
							//std::cout << "Pool Size: " << objectOwner.size();
							mapLock.unlock();
							return tmp;
						}else
						{
							lastFreeIndex = i;
						}
					}
				}
			}

			if (lastFreeIndex == -1)
			{
				if (objectOwner.size()+1 < maxSize)
				{
					T* tempPtr = r->initialize();
					ptrToIndex.insert(std::pair<T*, int>(tempPtr, objectOwner.size()));
					objectOwner.push_back(true);
					
					theObjectPtrs.push_back(tempPtr);
					mapLock.unlock();
					return tempPtr;
				}
				else
				{
					mapLock.unlock();
					throw std::out_of_range("Pool exceeded its size");	
				}
			}
			else
			{
				T* temp = lockAndUseOld(lastFreeIndex, r);
				r->reuse(temp);
				
				ptrToIndex.insert(std::pair<T*, int>(temp, lastFreeIndex));
				mapLock.unlock();
				return temp;
			}
		}

		void destroyAll()
		{
			mapLock.lock();
			for (int i = 0; i < theObjectPtrs.size(); i++)
			{
				if (theObjectPtrs[i] != nullptr)
				{
					objectOwner[i] = false;
					delete theObjectPtrs[i];
					theObjectPtrs[i] = nullptr;
				}
			}
			ptrToIndex.clear();
			mapLock.unlock();
		}

		//This function will release the object inside the pool and will DESTROY the object
		void destroy(T* ptr)
		{
			mapLock.lock();
			int index = indexOf(ptr);
			if (index > 0 && index < theObjectPtrs.size())
			{
				if (theObjectPtrs[index] != nullptr)
				{
					std::cout << "Deleted size of pool before" << theObjectPtrs.size() << std::endl;
					objectOwner[index] = false;
					ptrToIndex.erase(ptr);
					delete theObjectPtrs[index];
					theObjectPtrs.erase(theObjectPtrs.begin()+index);
					std::cout << "Deleted size of pool after" << theObjectPtrs.size() << std::endl;
					//theObjectPtrs[index] = nullptr;
				}
			}
			mapLock.unlock();
		}

		void disownAll()
		{
			mapLock.lock();
			for (int i = 0; i < objectOwner.size(); i++)
			{
				objectOwner[i] = false;
			}
			ptrToIndex.clear();
			mapLock.unlock();
		}

		//This function will release the object inside the pool and will not destroy the object
		void disown(T* ptr)
		{
			mapLock.lock();
			int index = indexOf(ptr);
			std::cout << "Disowned the pool Size before: " << objectOwner.size() << std::endl;
			objectOwner.at(index) = false;
			ptrToIndex.erase(ptr);
			std::cout << "Disowned the pool Size after: " << objectOwner.size() << std::endl;
			
			mapLock.unlock();
		}

		bool isOwned(T* ptr)
		{
			mapLock.lock();
			int index = indexOf(ptr);
			mapLock.unlock();
			return objectOwner.at(index);
		}

	private:

		int indexOf(T* ptr)
		{
			return ptrToIndex.at(ptr);
		}

		T* lockAndCreateNew(int i, Requirement<T> *r)
		{
			std::cout << "Using a new matrix" << objectOwner.size() << std::endl;
			objectOwner[i] = true;
			theObjectPtrs[i] = r->initialize();
			return theObjectPtrs[i];
		}

		T* lockAndUseOld(int i, Requirement<T>* r)
		{
			std::cout << "Using old matrix" << objectOwner.size() << std::endl;
			objectOwner[i] = true;
			r->reuse(theObjectPtrs[i]);
			return theObjectPtrs[i];
		}
		//bool is if the object is owneed
		std::vector<bool> objectOwner;

		std::vector<T*> theObjectPtrs;
		std::map<T*, int> ptrToIndex;

		std::mutex mapLock;
	};
}
#endif // !trials_heapObjectPool00

