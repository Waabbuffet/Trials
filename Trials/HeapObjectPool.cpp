#include "HeapObjectPool.h"


namespace Trials
{
	OpenCVMatrixRequirement::OpenCVMatrixRequirement(int sizeX, int sizeY, int sizeZ, int type):
		sizeX(sizeX),
		sizeY(sizeY),
		sizeZ(sizeZ),
		type(type)
	{

	}

	bool OpenCVMatrixRequirement::satisfy(cv::Mat* elementptr)
	{
		if (elementptr->rows >= sizeY)
		{
			if (elementptr->cols >= sizeX)
			{
				if (elementptr->type() == type)
				{
					return true;
				}
			}
		}
		return false;
	}

	cv::Mat* OpenCVMatrixRequirement::initialize()
	{
		cv::Mat*  elementptr = new cv::Mat(sizeY, sizeX, type);
		std::cout << "had to create new one lol";
		return elementptr;
	}

	//Right now this is equivalent to creating a new matrix,
	//Need to find better implementation
	void OpenCVMatrixRequirement::reuse(cv::Mat* elementptr)
	{
		std::cout << "Reused the function";
		elementptr->create(sizeY, sizeX, type);
	}
}
