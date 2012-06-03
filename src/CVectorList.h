/*
 * CVectorList.h
 *
 *  Created on: Jan 21, 2012
 *      Author: bkloppenborg
 */
 
 /* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * The authors request, but do not require, that you acknowledge the
 * use of this software in any publications.  See 
 * https://github.com/bkloppenborg/liboi/wiki
 * for example citations
 *
 * This file is part of the OpenCL Interferometry Library (LIBOI).
 * 
 * LIBOI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CVECTORLIST_H_
#define CVECTORLIST_H_

#include <vector>
using namespace std;

template <class T>

class CVectorList
{
protected:
	vector<T> mList;

public:
	CVectorList() {};

	~CVectorList()
	{
		Clear();
	}

	void Append(T item)
	{
		mList.push_back(item);
	}

	void Clear()
	{
		for(int i = mList.size() - 1; i > -1; i--)
			Remove(i);

	}

	void Remove(int i)
	{
		T tmp;
		if(i < mList.size())
		{
			tmp = mList[i];
			mList.erase(mList.begin() + i);
			delete tmp;
		}
	}

	int size(void)
	{
		return mList.size();
	}

	T operator[](int i) { return mList[i]; }
};

#endif /* CVECTORLIST_H_ */
