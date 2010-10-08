#include "ofxThread.h" 

//------------------------------------------------- 
ofxThread::ofxThread(){ 
   threadRunning = false; 
   verbose = false;
} 

//------------------------------------------------- 
ofxThread::~ofxThread(){ 
   stopThread(); 
} 

//------------------------------------------------- 
bool ofxThread::isThreadRunning(){ 
   //should be thread safe - no writing of vars here 
   return threadRunning; 
} 

//------------------------------------------------- 
void ofxThread::startThread(bool _blocking, bool _verbose){ 
   if( threadRunning ){ 
      if(verbose)printf("ofxThread: thread already running\n"); 
      return; 
   } 

   //have to put this here because the thread can be running 
   //before the call to create it returns 
   threadRunning   = true; 

   #ifdef TARGET_WIN32 
      //InitializeCriticalSection(&critSec); 
      myThread = (HANDLE)_beginthreadex(NULL, 0, this->thread,  (void *)this, 0, NULL); 
   #else 
      //pthread_mutex_init(&myMutex, NULL); 
      pthread_create(&myThread, NULL, thread, (void *)this); 
   #endif 

   blocking      =   _blocking; 
   verbose         = _verbose; 
} 

//------------------------------------------------- 
//returns false if it can't lock 
bool ofxThread::lock(){ 

	if ( blocking )
	{
		mutex.lock();
		if(verbose)printf("ofxThread: we are in -- mutex is now locked \n"); 
		return true;
	} 
	else
	{
		bool res = mutex.tryLock();
		if(verbose)
		{
			if ( res )
				printf("ofxThread: mutex is busy - already locked\n"); 
			else
				printf("ofxThread: we are in -- mutex is now locked \n"); 
		}
		return res;
	}
		
} 

//------------------------------------------------- 
bool ofxThread::unlock(){ 

	mutex.unlock();

   if(verbose)printf("ofxThread: we are out -- mutext is now unlocked \n"); 

   return true; 
} 

//------------------------------------------------- 
void ofxThread::stopThread(){ 
   if(threadRunning){ 
      #ifdef TARGET_WIN32 
         CloseHandle(myThread); 
      #else 
         //pthread_mutex_destroy(&myMutex); 
         pthread_detach(myThread); 
      #endif 
      if(verbose)printf("ofxThread: thread stopped\n"); 
      threadRunning = false; 
   }else{ 
      if(verbose)printf("ofxThread: thread already stopped\n"); 
   } 
}
