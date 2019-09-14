/*
* MIT License

* Copyright (c) 2005- Claybird

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

//---登録されたポインタをデストラクト時に自動的にdeleteする拡張型スマートポインタ
template <typename T>
class CSmartPtrCollection{
private:
	std::set<T*> m_Ptr;
public:
	CSmartPtrCollection(){}
	virtual ~CSmartPtrCollection(){DeleteAll();}
	void Add(T* lpT){	//管理に入れる
		ASSERT(lpT);
		if(lpT){
			m_Ptr.insert(lpT);
		}
	}
	void Remove(T* lpT){	//特定ポインタだけ管理から外す
		m_Ptr.erase(lpT);
	}
	void RemoveAll(){	//全ポインタを管理から外す
		m_Ptr.clear();
	}
	void Delete(T* lpT){	//特定ポインタだけdeleteして管理から外す
		ASSERT(lpT);
		if(lpT){
			m_Ptr.erase(lpT);
			delete lpT;
		}
	}
	void DeleteAll(){	//全ポインタをdeleteして管理から外す
		std::set<T*>::iterator ite=m_Ptr.begin();
		const std::set<T*>::iterator end=m_Ptr.end();
		for(;ite!=end;++ite){
			delete *ite;
		}
		m_Ptr.clear();
	}
};


template <typename T>
class CSmartPtrCollectionArray
{
	typedef T* PT;
protected:
	CSmartPtrCollection<T> m_Ptr;
	std::vector<T*> m_Array;
public:
	CSmartPtrCollectionArray(){}
	virtual ~CSmartPtrCollectionArray(){}
	void push_back(T* lpT){	//管理に入れる
		m_Ptr.Add(lpT);
		m_Array.push_back(lpT);
	}

	PT & operator[](size_t idx){return m_Array[idx];}
	size_t size()const{return m_Array.size();}
};
