#pragma once

#include <functional>
#include <memory>

namespace std
{
	using tr1::function;
}

namespace wpl
{
   namespace mt
   {
	   class thread
	   {
		   unsigned int _id;
		   void *_thread;

	   public:
		   typedef std::function<void()> action;

	   public:
		   explicit thread(const action &job);
		   virtual ~thread() throw();

		   static std::auto_ptr<thread> run(const action &initializer, const action &job);

		   unsigned int id() const throw();
	   };


	   // thread - inline definitions
	   inline unsigned int thread::id() const throw()
	   {	return _id;	}
   }
}
