// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------
//static int compareL2(Thread* t1, Thread* t2);
//11/29
//int Scheduler::compareL2(Thread *t1, Thread *t2)
static int compareL2(Thread *t1, Thread *t2)
{
    // cout<<"compareL2"<<endl;
    if(t1->getPriority()> t2->getPriority()){
        // cout<<"return-1"<<endl;
        return -1;
    }else if(t1->getPriority() < t2->getPriority()){
        // cout<<"return1"<<endl;
         return 1;
    } else {
        // cout<<"return?"<<endl;
        return t1->getID() < t2->getID() ? -1:1;
    }
    // cout<<"compareL3"<<endl;
    return 0;
}
//11/29
void Scheduler::updatePriority()
{

    // cout << "before update" << endl;
    // Print();
    //Print();
    ListIterator<Thread *> *iter2 = new ListIterator<Thread*>(L2ReadyList);
    //Print();
    //cout << "fuck1" << endl;

    Statistics *stats = kernel->stats;
    int oldPriority;
    int newPriority;

    for(;!iter2->IsDone();iter2->Next())
    {
        ASSERT(iter2->Item()->getStatus()==READY);
        // cout << "fuck2" << endl;

        iter2->Item()->setWaitingTime(iter2->Item()->getWaitingTime()+TimerTicks);
        if(iter2->Item()->getWaitingTime() >=1500 && iter2->Item()->getID()>0)
        {
            oldPriority = iter2->Item()->getPriority();
            DEBUG('z',"[C] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iter2->Item()->getID()<< "] changes its priority from [" << oldPriority << "] to ["<< newPriority << "]");
            newPriority = oldPriority + 10;

            if(newPriority>149)
            {
                newPriority = 149;
            }
            iter2->Item()->setPriority(newPriority);
            L2ReadyList->Remove(iter2->Item());
            ReadyToRun(iter2->Item());
        }

    }
    // cout << "after update" << endl;
    // Print();
}



Scheduler::Scheduler()
{
    //11/29 
    //readyList = new List<Thread *>; 
    //11/29
    // cout<<"L2ReadyList"<<endl;

    L2ReadyList = new SortedList<Thread *>(compareL2);


    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    //11/29
    //delete readyList; 
    delete L2ReadyList;
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
	//cout << "Putting thread on ready list: " << thread->getName() << endl ;

    /*********************************************/    
    thread->setStatus(READY);
    // 此時設定 thread 的 status 由 JUST_CREATED to READY    
    /*********************************************/

    /*********************************************/
    //11/29
    //readyList->Append(thread);
    //11/29
    //11/29(Debug)
    //  DEBUG(dbgSch , "[A] Tick [" << kernel->stats->totalTicks << "]:Thread [" << thread->getID()<< "] is inserted into queue L[queue level]");

    if(thread->getPriority() >= 0 && thread->getPriority() <= 149)
    {
        if(!L2ReadyList->IsInList(thread))
        {
            DEBUG('z' , "[A] Tick [" << kernel->stats->totalTicks << "]:Thread [" << thread->getID()<< "] is inserted into queue");
            // cout << "before insert" << endl;
            // Print();
            L2ReadyList->Insert(thread);
            // cout << "after insert" << endl;
            // Print();
        }
    }
    // status 為 READY 的 thread 加入 readyList 中
    
    // cout << "In schduler::ReadyToRun \n\t";
    // cout << "Putting thread on ready list: " << thread->getName() << "\n\t" ;
    // Print();
    // cout << "\n";
    /*********************************************/
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    // updatePriority();
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    //11/29
    //if (readyList->IsEmpty()) 
    if (L2ReadyList->IsEmpty()) 
    {
		return NULL;
    } else {
        // cout << "In Scheduler::FindNextToRun()" << endl; 
        // cout << "\tNumInList : " << L2ReadyList->NumInList() << endl;
        // cout << "\t";
        // Print();
        //11/29
        //Thread *a = readyList->RemoveFront();
        //11/29
        Thread *a = L2ReadyList->RemoveFront();
        //11/29(debug)
        DEBUG('z',"[B] Tick [" << kernel->stats->totalTicks << "]:Thread [" << a->getID()<< "] is removed from queue");
        // cout << "\tNext = " << a->getName() << endl;
    	return a;
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    // 註 : 經由 finish() -> Sleep() -> Run() 的 finishing 為 "True"
    /********************************************/
    Thread *oldThread = kernel->currentThread;
    // 這邊的 currentThread 在上一層 Sleep() 已經將 status 設為 BLOCKED 了
    /********************************************/
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    /********************************************/
    if (finishing) {	// mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
	    toBeDestroyed = oldThread;
    }
    // finishing == True，即表示 process 真的完成了
    // 用 toBeDestroyed 指到他，等下會做 delete
    /********************************************/

    /********************************************/
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
    // 若 process 還沒完成，則會將 thread 資訊儲存起來
    /********************************************/

    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    /********************************************/
    kernel->currentThread = nextThread;  // switch to the next thread
    // 將下一個要跑 thread 放到 CPU
    nextThread->setStatus(RUNNING);      // nextThread is now running
    // 將 Thread 的 status 設為 RUNNING
    /********************************************/
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    /**************************************************************/
    // cout << "In scheduler::Run \n\t";
    // Print();
    // // cout << "operating thread = " << oldThread->getName() << endl;
    // cout << "Before SWITCH" << endl;
    // cout << "\toldThread = " << oldThread->getName() << endl;
    // cout << "\tnextThread = " << nextThread->getName() << endl;
    // cout << endl;
    /**************************************************************/
    // int rand_num = rand()%20;
    // cout << "Before SWITCH" << endl;
    // cout << "rand_num = " << rand_num << "  , thread = " << oldThread->getName() << endl;
    // cout << "rand_num = " << rand_num << "  , thread = " << nextThread->getName() << endl;
    // cout << endl;
    DEBUG('z', "[D] Tick [" << kernel->stats->totalTicks << "]:Thread [" << nextThread->getID()<< "] is now selected for execution, thread [" << oldThread->getID() << "] is replaced, and it has executed ["<< oldThread->getBurstTime() << "] ticks");
    SWITCH(oldThread, nextThread);
    //11/29
    // DEBUG('z', "[D] Tick [" << kernel->stats->totalTicks << "]:Thread [" << nextThread->getID()<< "] is now selected for execution, thread [" << oldThread->getID() << "] is replaced, and it has executed ["<< nextThread->getBurstTime() << "] ticks");

    /**************************************************************/
    // cout << "In scheduler::Run \n\t";
    // Print();
    // cout << "operating thread = " << nextThread->getName() << endl;
    // cout << "After SWITCH" << endl;
    // cout << "\texecute " << oldThread->getName() << endl;
    // cout << "rand_num = " << rand_num << endl;
    // cout << endl;

    // cout << "\toldThread = " << oldThread->getName() << endl;
    // cout << "\tnextThread = " << nextThread->getName() << endl;
    // cout << endl;
    /**************************************************************/

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    /**********************************************/
    CheckToBeDestroyed();		// check if thread we were running
					            // before this one has finished
					            // and needs to be cleaned up
    // 若 ToBeDestroyed 有指到東西則 delete 他
    /**********************************************/
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
	toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents : ";
    L2ReadyList->Apply(ThreadPrint);
    cout << "\n";
}
// 11/29
// static int
// compareL2(Thread* t1, Thread* t2)
// int
// Scheduler::compareL2(Thread *t1, Thread *t2)
// {
//     if(t1->getPriority()> t2->getPriority())return -1;
//     else if(t1->getPriority() < t2->getPriority()) return 1; 
//     else return t1->getID() < t2->getID() ? -1:1;

//     return 0;
// }


