//-----------------------------------------------------------------------------
// luna2d engine
// Copyright 2014-2015 Stepan Prokofjev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#include "sqobject.h"
#include "sqproxy.h"
#include "squtils.h"
#include "lunalog.h"

namespace luna2d{

class SqFunction : public SqObject
{
	friend struct SqStack<SqFunction>;

public:
	SqFunction();
	SqFunction(SqVm* vm);
	SqFunction(HSQUIRRELVM vm);
	SqFunction(const SqFunction& fn);

private:
	SqFunction(const std::shared_ptr<SqRef>& ref);

private:
	// Push arguments list to stack
	void PushArgs(HSQUIRRELVM) const {}

	template<typename T>
	void PushArgs(HSQUIRRELVM vm, const T& t) const
	{
		SqStack<T>::Push(vm, t);
	}

	template<typename T, typename ... Args>
	void PushArgs(HSQUIRRELVM vm, const T& t, const Args& ... args) const
	{
		SqStack<T>::Push(vm, t);
		PushArgs(vm, args...);
	}

public:
	// Call function and get return value
	template<typename Ret, typename ... Args>
	Ret Call(const Args& ... args) const
	{
		if(IsNull()) return Ret();

		HSQUIRRELVM vm = ref->GetVm();

		SqStack<SqObject>::Push(vm, *this);
		sq_pushroottable(vm);
		PushArgs(vm, args...);

		sq_call(vm, sizeof...(args) + 1, true, true);

		SqScopedPop pop(vm, 2); // Pop function object and return value from stack after return
		return SqStack<Ret>::Get(vm, -1);
	}

	// Call function without getting return value
	template<typename ... Args>
	void CallVoid(const Args& ... args) const
	{
		if(IsNull()) return;

		HSQUIRRELVM vm = ref->GetVm();

		SqStack<SqObject>::Push(vm, *this);
		sq_pushroottable(vm);
		PushArgs(vm, args...);

		sq_call(vm, sizeof...(args) + 1, false, true);
		sq_pop(vm, 1); // Pop function object from stack
	}

	SqFunction& operator=(const SqFunction& fn);
};


template<>
struct SqStack<SqFunction>
{
	inline static void Push(HSQUIRRELVM vm, const SqFunction& fn)
	{
		SqStack<std::shared_ptr<SqRef>>::Push(vm, fn.GetRef());
	}

	inline static SqFunction Get(HSQUIRRELVM vm, int index = -1)
	{
		SQObjectType type = sq_gettype(vm, index);
		if(type != OT_CLOSURE && type != OT_NATIVECLOSURE) return SqFunction(vm);
		return SqFunction(SqStack<std::shared_ptr<SqRef>>::Get(vm, index));
	}
};

}