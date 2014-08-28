/*-*- tab-width:3; indent-tabs-mode:nil; c-file-style:"ellemtel"; c-file-offsets:((innamespace . 0)(inclass . ++)) -*-*/
/*******************************************************************************
 FILE         :   test_scheduler1.cpp
 COPYRIGHT    :   Yakov Markovitch, 2009-2014. All rights reserved.
                  See LICENSE for information on usage/redistribution.

 DESCRIPTION  :   The test of a trivial scheduler class.

 CREATION DATE:   11 Jun 2009
*******************************************************************************/
#include <pcomn_scheduler.h>
#include <pcomn_timespec.h>
#include <pcomn_fileutils.h>
#include <pcomn_unistd.h>

#include <functional>

#include <stdio.h>

using namespace pcomn ;

static bool stop_all ;

static void worker_fn(const std::string &name, unsigned sleep_usec)
{
   const int sout = fileno(stdout) ;

   pcomn::fdprintf<32>(sout, "%p\n", &sleep_usec) ;

   const std::string &start_text = name + " started at " + time_point::now().string() + "\n" ;
   ::write(sout, start_text.c_str(), start_text.size()) ;
   usleep(sleep_usec) ;
   const std::string &end_text = name + " ended at " + time_point::now().string() + "\n" ;
   ::write(sout, end_text.c_str(), end_text.size()) ;
}

int main(int argc, char *argv[])
{
   DIAG_INITTRACE("pcomn.scheduler.trace.ini") ;

   try
   {
      Scheduler scheduler (0, 4096) ;

      std::cout << "The synchronous scheduler has been created." << std::endl ;

      const Scheduler::taskid_t First =
         scheduler.schedule(std::bind(worker_fn, "First", 0), 1000000, 2000000, 0) ;
      const Scheduler::taskid_t Second =
         scheduler.schedule(std::bind(worker_fn, "Second", 100000), 0, 1000000, 0) ;

      std::cout << "All tasks has been sent." << std::endl ;
      char c ;

      std::cerr << "Please hit <AnyKey><ENTER> to cancel 'First' scheduler." << std::endl ; std::cin >> c ;
      std::cout << "Cancelling 'First'" << std::endl ;
      scheduler.cancel(First, true) ;
      std::cout << "'First' canceled" << std::endl ;

      std::cerr << "Please hit <AnyKey><ENTER> to stop scheduler." << std::endl ; std::cin >> c ;
      std::cout << "Stopping scheduler" << std::endl ;
      stop_all = true ;
   }
   catch(const std::exception &x)
   {
      std::cout << "Exception: " << x.what() << std::endl ;
      return 1 ;
   }
   std::cout << "Stopped" << std::endl ;

   std::cerr << "Please hit <AnyKey><ENTER> to exit." << std::endl ;
   char c ;
   std::cin >> c ;
   std::cout << "Finished" << std::endl ;

   return 0 ;
}
