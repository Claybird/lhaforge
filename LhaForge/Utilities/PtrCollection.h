/*
 * Copyright (c) 2005-2012, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#pragma once

//---�o�^���ꂽ�|�C���^���f�X�g���N�g���Ɏ����I��delete����g���^�X�}�[�g�|�C���^
template <typename T>
class CSmartPtrCollection{
private:
	std::set<T*> m_Ptr;
public:
	CSmartPtrCollection(){}
	virtual ~CSmartPtrCollection(){DeleteAll();}
	void Add(T* lpT){	//�Ǘ��ɓ����
		ASSERT(lpT);
		if(lpT){
			m_Ptr.insert(lpT);
		}
	}
	void Remove(T* lpT){	//����|�C���^�����Ǘ�����O��
		m_Ptr.erase(lpT);
	}
	void RemoveAll(){	//�S�|�C���^���Ǘ�����O��
		m_Ptr.clear();
	}
	void Delete(T* lpT){	//����|�C���^����delete���ĊǗ�����O��
		ASSERT(lpT);
		if(lpT){
			m_Ptr.erase(lpT);
			delete lpT;
		}
	}
	void DeleteAll(){	//�S�|�C���^��delete���ĊǗ�����O��
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
protected:
	CSmartPtrCollection<T> m_Ptr;
	std::vector<T*> m_Array;
public:
	CSmartPtrCollectionArray(){}
	virtual ~CSmartPtrCollectionArray(){}
	void push_back(T* lpT){	//�Ǘ��ɓ����
		m_Ptr.Add(lpT);
		m_Array.push_back(lpT);
	}

	(T*)& operator[](size_t idx){return m_Array[idx];}
	size_t size()const{return m_Array.size();}
};
