#pragma once

#ifndef trials_heapObjectPool00
#define trials_heapObjectPool00
#include <vector>
#include <map>
#include <iostream>
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

	private:
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
			for (int i = 0; i < objectOwner.size(); i++)
			{
				if (!objectOwner[i])
				{
					if (theObjectPtrs[i] == nullptr)
					{
						//std::cout << "nullptr!";
						if (lastFreeIndex == -1)
						{
							T* tmp = lockAndCreateNew(i, r);
							ptrToIndex.insert(std::pair<T*, int>(tmp, i));
							return tmp;
						}
						else
						{
							T* temp = lockAndUseOld(lastFreeIndex, r);
							r->reuse(temp);
							ptrToIndex.insert(std::pair<T*, int>(temp, lastFreeIndex));
							return temp;
						}
					}
					else
					{
						if (r->satisfy(theObjectPtrs[i]))
						{
							T* tmp = lockAndUseOld(i, r);
							ptrToIndex.insert(std::pair<T*, int>(tmp, i));
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
					//std::cout << "created new";
					T* tempPtr = r->initialize();
					ptrToIndex.insert(std::pair<T*, int>(tempPtr, objectOwner.size()));
					objectOwner.push_back(true);
				
					theObjectPtrs.push_back(tempPtr);
					return tempPtr;
				}
				else
				{
					throw std::out_of_range("Pool exceeded its size");
				}
			}
			else
			{
				//std::cout << "reused old!";
				T* temp = lockAndUseOld(lastFreeIndex, r);
				r->reuse(temp);
				ptrToIndex.insert(std::pair<T*, int>(temp, lastFreeIndex));
				return temp;
			}
		}

		void destroyAll()
		{
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
		}

		//This function will release the object inside the pool and will DESTROY the object
		void destroy(T* ptr)
		{
			int index = indexOf(ptr);
			if (index > 0 && index < theObjectPtrs.size())
			{
				if (theObjectPtrs[index] != nullptr)
				{
					//std::cout << "Deleted";
					objectOwner[index] = false;
					ptrToIndex.erase(ptr);
					delete theObjectPtrs[index];
					theObjectPtrs[index] = nullptr;
				}
			}
		}

		void disownAll()
		{
			for (int i = 0; i < objectOwner.size(); i++)
			{
				objectOwner[i] = false;
			}
			ptrToIndex.clear();
		}

		//This function will release the object inside the pool and will not destroy the object
		void disown(T* ptr)
		{
			int index = indexOf(ptr);
			objectOwner.at(index) = false;
			ptrToIndex.erase(ptr);
		}

		bool isOwned(T* ptr)
		{
			int index = indexOf(ptr);
			return objectOwner.at(index);
		}

	private:

		int indexOf(T* ptr)
		{
			return ptrToIndex.at(ptr);
		}

		T* lockAndCreateNew(int i, Requirement<T> *r)
		{
			objectOwner[i] = true;
			theObjectPtrs[i] = r->initialize();
			return theObjectPtrs[i];
		}

		T* lockAndUseOld(int i, Requirement<T>* r)
		{
			objectOwner[i] = true;
			r->reuse(theObjectPtrs[i]);
			return theObjectPtrs[i];
		}
		//bool is if the object is owneed
		std::vector<bool> objectOwner;

		std::vector<T*> theObjectPtrs;
		std::map<T*, int> ptrToIndex;
	};
}
#endif // !trials_heapObjectPool00

