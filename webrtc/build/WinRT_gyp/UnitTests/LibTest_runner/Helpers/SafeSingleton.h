//===========================================================================
//! \file SafeSingleton.h
//!
//! file contains implementation of CSafeSingleton
//!
//! 2014/02/13: created
//============================================================================

#pragma once

#include <assert.h>

namespace LibTest_runner
{

	//! declares the function prototype for callback of atexit CRT function
	typedef void (*PFnAtExit)();


	//=============================================================================
	//! \brief This is a creation policy class of CSafeSingletonT. 
	//! It allocates instance by new expression and calls default ctor().
	//!
	//=============================================================================
	template <class T>
	struct CStonCreateUsingNewT
	{
		//! creates new instance
		static T* Create()
		{
			return new T();
		}

		//! creates new instance
		static void Destroy(T* p)
		{
			delete p;
		}
	};

	//=============================================================================
	//! \brief This is a creation policy class of CSafeSingletonT. 
	//! It provides storage of variable in data segment. 
	//=============================================================================
	template <class T> struct CStonCreateStatic
	{
		//! creates new instance
		static T* Create()
		{
			static T stInstance;
			return &stInstance;
		}

		//! creates new instance
		static void Destroy(T* p)
		{
			ExplicitDtor(p);
		}
	};


	//! This macro is a helper for easy and fast definition of the singleton creation policy
	//! for cases when Constructor has more arguments
#define CPPF_DEFINE_STON_CREATE_POLICY_STATIC_V(__cls__, T, __args__) \
	struct __cls__ \
	{\
	static T* Create()\
	{\
	static T stInstance __args__;\
	return &stInstance;\
	}\
	\
	static void Destroy(T* p)\
	{\
	ExplicitDtor(p);\
	}\
	}


	//=============================================================================
	//! \brief This is a lifetime policy of class for CSafeSingletonT. 
	//! It registers CSafeSingletonT::DectroySingleton function by atexit(), so it is 
	//! called when program exits.
	//=============================================================================
	template <class T>
	struct CStonDefaultLifetimeT
	{
		//!Schedules destruction on exit
		static void ScheduleDestruction(T*, PFnAtExit pFun)
		{
			std::atexit(pFun);
		}

		//! dead reference handler
		static void OnDeadReference() throw()
		{
			
		}
	};

	//=============================================================================
	//! \brief This is a lifetime policy of class for CSafeSingletonT.
	//! It takes no action, so singleton is never destroyed. 
	//=============================================================================
	template <class T>
	struct CStonNoDestroyT
	{
		//!Schedules destruction on exit
		static void ScheduleDestruction(T*, PFnAtExit pFun)
		{}

		//! dead reference handler
		static void OnDeadReference()
		{}
	};

	//=============================================================================
	//! \brief This is a lifetime policy of class for CSafeSingletonT. 
	//! It maintains its own list, of instances. This is usefull for example in DLLs.  
	//=============================================================================
	struct CSingletonExplicitDestroy
	{
		//=============================================================================
		//! \brief This object is used for registration of call atexit() calls
		//=============================================================================
		class CDestroyItem
		{
			CDestroyItem() {}
		public:
			// The constructor links the object into the global list

			//! TODO: Mutex not implemented so not thread safe !!!!!!
			CDestroyItem(PFnAtExit fnDestroy)
			{
				//xpl::CMutexLock   lock(CXplLibrary::InitMutex());

				m_fnDestroy = fnDestroy;
				m_pNext = *GetHead();
				*GetHead() = this;
			}

			// The destructor removes the head of the list and calls the function.

			~CDestroyItem(void)
			{

				if (m_fnDestroy) m_fnDestroy();
				*GetHead() = this->m_pNext;
			}

			//! TODO: Mutex not implemented so not thread safe !!!!!!
			static void DestroyItems()
			{
				// lock this for whole list iteration, so there is not added Phoenix into list
				//xpl::CMutexLock   lock(CXplLibrary::InitMutex());

				// call termination handlers
				while (*GetHead() != NULL)
					// invoking the destructor causes the termination handler to be called
					delete (*GetHead());
			}

		private:
			// link to next item
			CDestroyItem* m_pNext;
			PFnAtExit     m_fnDestroy;

			// list of the items
			static CDestroyItem** GetHead()
			{
				static  CDestroyItem* m_pList = NULL;
				return &m_pList;
			}
		};

		//! Schedules destruction
		static void ScheduleDestruction(void*, PFnAtExit pFun)
		{
			new CDestroyItem(pFun);
		}

		//! dead reference handler
		static void OnDeadReference()
		{
		}

		//!destroy
		static void Destroy()
		{
			CDestroyItem::DestroyItems();
		}

	};




	//=============================================================================
	//! \brief Singleton class
	//!  This class must be used instead static variables inside 
	//!  (static) member functions. It ensures thread safe and only 
	//!  one initialization of the class instance wrapped by singleton. 
	//!  The construction/destruction and lifetime can be controled by policy.
	//!  Here are provided base policies, but user can create custom policy
	//!  to create (provide storage) and initialize wrapped instance.
	//!
	//! @param T - type which is wraped by singleton
	//! @param CreationPolicy - this class defines creation policy. See 
	//!               CStonCreateUsingNewT, or CStonCreateStaticT for sample which
	//!               interface it should have.
	//! @param LifetimePolicy - this class defines lifetime policy. It defines 
	//!               the end of the lifetime of the singleton.
	//=============================================================================
	template <class T,
	class CreationPolicy = CStonCreateUsingNewT<T>,
	class LifetimePolicy = CStonDefaultLifetimeT<T> >
	class CSafeSingletonT
	{
	public:

		CSafeSingletonT()
		{}

		//=============================================================================
		//! \brief Returns reference to instance of the encapsulated singleton.
		//! In case singleton is not created it creates singleton. 
		//! @return reference to single instance
		//=============================================================================
		static T& Instance()
		{
			if (!InternalInstance())
			{
				CreateSingleton();
			}
			return *InternalInstance();
		}
	protected:
		//! Internal instance
		static T*& InternalInstance()
		{
			static T*     s_pInstance;
			return s_pInstance;
		}

		//! indicates that instance destroyed
		static bool& Destroyed()
		{
			static bool stDestroyed = false;
			return stDestroyed;
		}

		//=============================================================================
		//! \brief creates singleton through create policy. 
		//! It registers DestroySingleton into lifetime policy. Creation is guarded by mutex 
		//!                 so it is thread safe.
		//! TODO: Mutex not implemented so not thread safe !!!!!!
		//! @return
		//=============================================================================
		static void CreateSingleton()
		{
			//xpl::CMutexLock   lock(CXplLibrary::InitMutex());
			if (!InternalInstance()) // use double-check pattern
			{
				if (Destroyed())
				{
					LifetimePolicy::OnDeadReference();
					Destroyed() = false;
				}
				InternalInstance() = CreationPolicy::Create();
				LifetimePolicy::ScheduleDestruction(InternalInstance(), &DestroySingleton);
			}
		}

		//=============================================================================
		//! \brief This static function is registered into lifetime policy.
		//! It could be called when lifetime of the singleton is finished.
		//! TODO: Mutex not implemented so not thread safe !!!!!!
		//!
		//! @return
		//=============================================================================
		static void DestroySingleton()
		{
			//TODO: implement locking to be thread safe
			assert(!Destroyed());
			CreationPolicy::Destroy(InternalInstance());
			InternalInstance() = NULL;
			Destroyed() = true;
		}
	};
} 
