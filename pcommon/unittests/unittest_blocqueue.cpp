/*-*- tab-width:4;indent-tabs-mode:nil;c-file-style:"ellemtel";c-basic-offset:4;c-file-offsets:((innamespace . 0)(inlambda . 0)) -*-*/
/*******************************************************************************
 FILE         :   unittest_semaphore.cpp
 COPYRIGHT    :   Yakov Markovitch, 2020. All rights reserved.
                  See LICENSE for information on usage/redistribution.

 DESCRIPTION  :   Unittests for blocking concurrent queue

 PROGRAMMED BY:   Yakov Markovitch
 CREATION DATE:   25 Feb 2020
*******************************************************************************/
#include "unittest_semaphore.h"

#include <pcomn_blocqueue.h>
#include <pcomn_stopwatch.h>
#include <pcomn_iterator.h>
#include <pcomn_atomic.h>

#include <thread>
#include <chrono>

#include <stdlib.h>

using namespace pcomn ;
using namespace std::chrono ;

class counting_quasi_queue ;
typedef blocking_queue<unsigned, counting_quasi_queue> counting_blocqueue ;


namespace CppUnit {
template<typename T>
struct assertion_traits<count_iterator<T>> {

      static bool equal(const count_iterator<T> &x, const count_iterator<T> &y)
      {
          return x == y ;
      }

      static std::string toString(const count_iterator<T> &x)
      {
          return assertion_traits<T>::toString(x.count()) ;
      }
} ;

template<typename T>
struct assertion_traits<fwd::optional<T>> {

      static bool equal(const fwd::optional<T> &x, const fwd::optional<T> &y)
      {
          return x == y ;
      }

      __noinline static std::string toString(const fwd::optional<T> &x)
      {
          return x ? "{" + assertion_traits<T>::toString(*x) + "}" : std::string("{}") ;
      }
} ;
}

class counting_quasi_queue {
public:
    typedef uint32_t value_type ;
    typedef count_iterator<value_type> iterator ;

    explicit counting_quasi_queue(unsigned current_capacity) :
        _max_size(blocqueue_controller::max_capacity()),
        _capacity(current_capacity)
    {
        PCOMN_VERIFY(_capacity) ;
        PCOMN_VERIFY(_capacity <= _max_size) ;
        last_constructed = this ;
    }

    explicit counting_quasi_queue(const unipair<unsigned> &capacities) :
        _max_size(capacities.second),
        _capacity(capacities.first)
    {
        PCOMN_VERIFY(capacities.second <= blocqueue_controller::max_capacity()) ;
        PCOMN_VERIFY(capacities.first <= capacities.second) ;
        last_constructed = this ;
    }

    ~counting_quasi_queue()
    {
        if (last_constructed == this)
            last_constructed = nullptr ;
    }

    void push(const value_type &)
    {
        const queue_data result (atomic_op::add(&_qdata, queue_data(1, 0)._value)) ;
        PCOMN_VERIFY((uint32_t)result._occupied_count <= _capacity) ;
    }
    void push(value_type &&v) { push(const_cast<const value_type &>(v)) ; }

    template<typename FwdIterator>
    FwdIterator push_many(FwdIterator begin, FwdIterator end) ;

    value_type pop()
    {
        const queue_data result (atomic_op::add(&_qdata, queue_data(-1, 1)._value)) ;
        PCOMN_VERIFY(result._occupied_count >= 0) ;
        PCOMN_VERIFY(result._popped_count > 0) ;
        return result._popped_count - 1 ;
    }

    unipair<iterator> pop_many(unsigned count)
    {
        PCOMN_VERIFY(count) ;
        PCOMN_VERIFY(count <= current_capacity()) ;

        const queue_data result (atomic_op::add(&_qdata, queue_data(-(int)count, count)._value)) ;

        PCOMN_VERIFY(result._occupied_count >= 0) ;
        PCOMN_VERIFY(result._popped_count >= count) ;
        return {iterator(result._popped_count - count), iterator(result._popped_count)} ;
    }

    void change_capacity(unsigned new_capacity)
    {
        PCOMN_VERIFY(new_capacity <= max_size()) ;
    }

    unsigned max_size() const { return _max_size ; }
    unsigned current_capacity() const { return _capacity ; }

    static thread_local const counting_quasi_queue *last_constructed ;

public:
    union queue_data {
        constexpr queue_data(uint64_t v = 0) noexcept : _value(v) {}

        constexpr queue_data(int32_t occupied, uint32_t popped) noexcept :
            _popped_count(popped),
            _occupied_count(occupied)
        {}

        uint64_t _value ;
        struct {
            uint32_t _popped_count ;
            int32_t  _occupied_count ;
        } ;
    } ;

    queue_data qdata() const { return queue_data(_qdata) ; }

private:
    const uint32_t _max_size ;
    uint32_t       _capacity ;
    mutable std::atomic<uint64_t> _qdata {} ;
} ;

thread_local const counting_quasi_queue *counting_quasi_queue::last_constructed = nullptr ;

/*******************************************************************************
                            class BlockingQueueTests
 *******************************************************************************/
class BlockingQueueTests : public CppUnit::TestFixture {

    void Test_BlockingQueue_TestFixture() ;
    void Test_BlockingQueue_Limits() ;
    void Test_BlockingQueue_SingleThreaded() ;

    CPPUNIT_TEST_SUITE(BlockingQueueTests) ;

    CPPUNIT_TEST(Test_BlockingQueue_TestFixture) ;
    CPPUNIT_TEST(Test_BlockingQueue_Limits) ;
    CPPUNIT_TEST(Test_BlockingQueue_SingleThreaded) ;

    CPPUNIT_TEST_SUITE_END() ;

private:
    unit::watchdog watchdog {5s} ;

    static const unsigned maxsize = counting_blocqueue::max_capacity() ;

public:
    void setUp()
    {
        watchdog.arm() ;
    }
    void tearDown()
    {
        watchdog.disarm() ;
    }
} ;

/*******************************************************************************
 BlockingQueueTests
*******************************************************************************/
const unsigned BlockingQueueTests::maxsize ;

// Who will test the testers?
void BlockingQueueTests::Test_BlockingQueue_TestFixture()
{
    typedef counting_quasi_queue::iterator citerator ;

    CPPUNIT_LOG_EQ(counting_quasi_queue(5).current_capacity(), 5) ;
    CPPUNIT_LOG_EQ(counting_quasi_queue(5).max_size(), blocqueue_controller::max_capacity()) ;
    CPPUNIT_LOG_EQ(counting_quasi_queue({10, 20}).current_capacity(), 10) ;
    CPPUNIT_LOG_EQ(counting_quasi_queue({10, 20}).max_size(), 20) ;

    CPPUNIT_LOG(std::endl) ;

    counting_quasi_queue q (10) ;

    CPPUNIT_LOG_EQ(q.qdata()._occupied_count, 0) ;
    CPPUNIT_LOG_EQ(q.qdata()._popped_count, 0) ;

    CPPUNIT_LOG_RUN(q.push(1)) ;
    CPPUNIT_LOG_EQ(q.qdata()._occupied_count, 1) ;
    CPPUNIT_LOG_EQ(q.qdata()._popped_count, 0) ;

    CPPUNIT_LOG_RUN(q.push(5)) ;
    CPPUNIT_LOG_EQ(q.qdata()._occupied_count, 2) ;
    CPPUNIT_LOG_EQ(q.qdata()._popped_count, 0) ;

    CPPUNIT_LOG_EQ(q.pop(), 0) ;
    CPPUNIT_LOG_EQ(q.pop(), 1) ;

    CPPUNIT_LOG_EQ(q.qdata()._occupied_count, 0) ;
    CPPUNIT_LOG_EQ(q.qdata()._popped_count, 2) ;

    CPPUNIT_LOG_RUN(q.push(1)) ;
    CPPUNIT_LOG_RUN(q.push(1)) ;
    CPPUNIT_LOG_RUN(q.push(1)) ;
    CPPUNIT_LOG_EQ(q.qdata()._occupied_count, 3) ;
    CPPUNIT_LOG_EQ(q.qdata()._popped_count, 2) ;


    CPPUNIT_LOG_EQ(q.pop(), 2) ;
    CPPUNIT_LOG_EQ(q.pop(), 3) ;

    CPPUNIT_LOG_EQ(q.qdata()._occupied_count, 1) ;
    CPPUNIT_LOG_EQ(q.qdata()._popped_count, 4) ;

    CPPUNIT_LOG_RUN(q.push(1)) ;
    CPPUNIT_LOG_RUN(q.push(1)) ;
    CPPUNIT_LOG_RUN(q.push(1)) ;

    CPPUNIT_LOG_EQUAL(q.pop_many(2), unipair<citerator>(citerator(4), citerator(6))) ;

    CPPUNIT_LOG_EQ(q.qdata()._occupied_count, 2) ;
    CPPUNIT_LOG_EQ(q.qdata()._popped_count, 6) ;

    CPPUNIT_LOG(std::endl) ;
}

void BlockingQueueTests::Test_BlockingQueue_Limits()
{
    CPPUNIT_LOG_EXCEPTION(counting_blocqueue(maxsize + 1), std::out_of_range) ;
    CPPUNIT_LOG_EXCEPTION(counting_blocqueue(0), std::out_of_range) ;
    CPPUNIT_LOG_RUN(counting_blocqueue(maxsize + 0)) ;
    CPPUNIT_LOG_RUN(counting_blocqueue(1)) ;

    CPPUNIT_LOG_EXCEPTION(counting_blocqueue({0, 0}), std::out_of_range) ;
    CPPUNIT_LOG_EXCEPTION(counting_blocqueue({0, 1}), std::out_of_range) ;
    CPPUNIT_LOG_EXCEPTION(counting_blocqueue({1, maxsize + 1}), std::out_of_range) ;
    CPPUNIT_LOG_EXCEPTION(counting_blocqueue({5, 4}), std::invalid_argument) ;
    CPPUNIT_LOG_RUN(counting_blocqueue({1, 1})) ;

    CPPUNIT_LOG(std::endl) ;

    counting_blocqueue q1 (1) ;

    CPPUNIT_LOG_EQ(q1.capacity(), 1) ;
    CPPUNIT_LOG_RUN(q1.change_capacity(2)) ;
    CPPUNIT_LOG_EQ(q1.capacity(), 2) ;
    CPPUNIT_LOG_EXCEPTION(q1.change_capacity(0), std::out_of_range) ;

    counting_blocqueue q11 ({5, 100}) ;

    CPPUNIT_LOG_EQ(q11.capacity(), 5) ;
    CPPUNIT_LOG_EXCEPTION(q11.change_capacity(101), std::out_of_range) ;
    CPPUNIT_LOG_EQ(q11.capacity(), 5) ;
    CPPUNIT_LOG_RUN(q11.change_capacity(100)) ;
    CPPUNIT_LOG_EQ(q11.capacity(), 100) ;
    CPPUNIT_LOG_RUN(q11.change_capacity(1)) ;
    CPPUNIT_LOG_EQ(q11.capacity(), 1) ;
    CPPUNIT_LOG_EQ(q11.size(), 0) ;
}

void BlockingQueueTests::Test_BlockingQueue_SingleThreaded()
{
    {
        counting_blocqueue q1 (1) ;
        const counting_quasi_queue &cqq1 = *counting_quasi_queue::last_constructed ;

        CPPUNIT_LOG_RUN(q1.push(1)) ;
        CPPUNIT_LOG_EQ(q1.size(), 1) ;
        CPPUNIT_LOG_EQ(q1.pop(), 0) ;
        CPPUNIT_LOG_EQ(q1.size(), 0) ;

        CPPUNIT_LOG_RUN(q1.push(1)) ;
        CPPUNIT_LOG_EQ(q1.size(), 1) ;
        CPPUNIT_LOG_EQ(q1.pop(), 1) ;

        CPPUNIT_LOG_EQ(q1.size(), 0) ;
        CPPUNIT_LOG_EQ(cqq1.qdata()._popped_count, 2) ;
        CPPUNIT_LOG_EQ(cqq1.qdata()._occupied_count, 0) ;

        CPPUNIT_LOG_EQUAL(q1.try_pop(), counting_blocqueue::optional_value()) ;
        CPPUNIT_LOG_IS_FALSE(q1.try_pop()) ;

        CPPUNIT_LOG_RUN(q1.close()) ;
        CPPUNIT_LOG_EXCEPTION(q1.push(1), sequence_closed) ;
        CPPUNIT_LOG_EXCEPTION(q1.try_pop(), sequence_closed) ;
        CPPUNIT_LOG_EXCEPTION(q1.pop(), sequence_closed) ;

        CPPUNIT_LOG_EQ(cqq1.qdata()._popped_count, 2) ;
        CPPUNIT_LOG_EQ(cqq1.qdata()._occupied_count, 0) ;
    }

    CPPUNIT_LOG(std::endl) ;
    {
        counting_blocqueue q2 (5) ;
        const counting_quasi_queue &cqq2 = *counting_quasi_queue::last_constructed ;

        CPPUNIT_LOG_ASSERT(q2.try_push(1)) ;
        CPPUNIT_LOG_ASSERT(q2.try_push(1)) ;
        CPPUNIT_LOG_RUN(q2.push(1)) ;
        CPPUNIT_LOG_RUN(q2.push(1)) ;
        CPPUNIT_LOG_RUN(q2.push(1)) ;
        CPPUNIT_LOG_IS_FALSE(q2.try_push(1)) ;

        CPPUNIT_LOG_EQ(cqq2.qdata()._popped_count, 0) ;
        CPPUNIT_LOG_EQ(cqq2.qdata()._occupied_count, 5) ;

        CPPUNIT_LOG_EQUAL(q2.try_pop(), counting_blocqueue::optional_value(0)) ;
        CPPUNIT_LOG_EQUAL(q2.try_pop(), counting_blocqueue::optional_value(1)) ;

        CPPUNIT_LOG_EQ(cqq2.qdata()._popped_count, 2) ;
        CPPUNIT_LOG_EQ(cqq2.qdata()._occupied_count, 3) ;

        CPPUNIT_LOG_EQ(q2.pop(), 2) ;

        CPPUNIT_LOG_IS_FALSE(q2.close_push()) ;

        CPPUNIT_LOG_EXCEPTION(q2.push(1), sequence_closed) ;
        CPPUNIT_LOG_EXCEPTION(q2.try_push(1), sequence_closed) ;

        CPPUNIT_LOG_EQ(cqq2.qdata()._popped_count, 3) ;
        CPPUNIT_LOG_EQ(cqq2.qdata()._occupied_count, 2) ;

        CPPUNIT_LOG_ASSERT(q2.try_pop()) ;

        CPPUNIT_LOG_IS_FALSE(q2.close_push()) ;

        CPPUNIT_LOG_EQ(q2.pop(), 4) ;

        CPPUNIT_LOG_EQ(cqq2.qdata()._popped_count, 5) ;
        CPPUNIT_LOG_EQ(cqq2.qdata()._occupied_count, 0) ;

        CPPUNIT_LOG_EXCEPTION(q2.pop(), sequence_closed) ;
        CPPUNIT_LOG_EXCEPTION(q2.try_pop(), sequence_closed) ;

        CPPUNIT_LOG_ASSERT(q2.close_push()) ;
        CPPUNIT_LOG_RUN(q2.close()) ;
    }
}

/*******************************************************************************
                            class BlockingQueueFuzzyTests
*******************************************************************************/
class BlockingQueueFuzzyTests : public ProducerConsumerFixture {
    typedef ProducerConsumerFixture ancestor ;

    template<unsigned producers, unsigned consumers,
             unsigned pcount,
             unsigned max_pause_nano = 0, unsigned before_closed_milli = 0>
    void RunTest()
    {
        run(producers, consumers, pcount,
            nanoseconds(max_pause_nano), milliseconds(before_closed_milli)) ;
    }

    CPPUNIT_TEST_SUITE(BlockingQueueFuzzyTests) ;

    //CPPUNIT_TEST(P_PASS(RunTest<1,1,1>)) ;
    //CPPUNIT_TEST(P_PASS(RunTest<1,1,1000>)) ;
    //CPPUNIT_TEST(P_PASS(RunTest<1,1,2'000'000>)) ;
    //CPPUNIT_TEST(P_PASS(RunTest<2,2,2'000'000>)) ;
    //CPPUNIT_TEST(P_PASS(RunTest<2,1,1'000'000,100>)) ;
    CPPUNIT_TEST(P_PASS(RunTest<2,2,2'000'000,10,500>)) ;
    //CPPUNIT_TEST(P_PASS(RunTest<2,5,10'000'000,20,500>)) ;
    //CPPUNIT_TEST(P_PASS(RunTest<7,5,1'000'000>)) ;
    //CPPUNIT_TEST(P_PASS(RunTest<5,2,1'000'000,10,500>)) ;

    CPPUNIT_TEST_SUITE_END() ;

private:
    void run(unsigned producers, unsigned consumers, unsigned pcount,
             nanoseconds max_pause, milliseconds before_close) ;

public:
    BlockingQueueFuzzyTests() : ancestor(2h) {}

    /***************************************************************************
     tester_thread
    ***************************************************************************/
    class tester_thread final : public ancestor::tester_thread {
        typedef ancestor::tester_thread base ;
    public:
        std::vector<unsigned>   _produced ;
        std::vector<unsigned>   _consumed ;
        const uint64_t          _volume = 0 ;
        uint64_t                _remains = _volume ;
        uint64_t                _total = 0 ;
        counting_blocqueue &    _queue ;
        std::thread             _thread ;

        tester_thread(TesterMode mode, counting_blocqueue &cbq, unsigned volume, double p,
                      nanoseconds max_pause = {}) ;

        std::thread &self() final { return _thread ; }

    private:
        void produce() final ;
        void consume() final ;
    } ;

    static tester_thread &tester(const tester_thread_ptr &pt)
    {
        PCOMN_VERIFY(pt) ;
        return *static_cast<tester_thread *>(pt.get()) ;
    }
} ;

/*******************************************************************************
 BlockingQueueFuzzyTests::tester_thread
*******************************************************************************/
BlockingQueueFuzzyTests::tester_thread::tester_thread(TesterMode mode, counting_blocqueue &cbq,
                                                      unsigned volume, double p, nanoseconds max_pause) :
    base(p, volume, max_pause),
    _volume(volume),
    _queue(cbq)
{
    init(mode) ;
}

void BlockingQueueFuzzyTests::tester_thread::produce()
{
    CPPUNIT_LOG_LINE("Start producer " << HEXOUT(_thread.get_id()) << ", must produce " << _remains << " items.") ;

    for(int i = 0 ; ++i <= 2 ;) try
    {
        while (_remains)
        {
            for (unsigned count = std::min<uint64_t>({_generate(), _queue.capacity(), _remains}) ; count-- ;)
            {
                _queue.push(_total) ;
                ++_total ;
                --_remains ;
            }
        }
    }
    catch (const sequence_closed &)
    {
        CPPUNIT_LOG_LINE("Queue closed in producer " << HEXOUT(_thread.get_id())
                         << ", attempt " << i << ", produced " << _total << " items, "
                         << _remains << " remains.") ;

        std::this_thread::sleep_for(generate_pause()) ;
    }

    CPPUNIT_LOG_LINE("Finish producer " << HEXOUT(_thread.get_id())
                     << ", produced " << _total << " items, " << _remains << " remains.") ;
}

void BlockingQueueFuzzyTests::tester_thread::consume()
{
    CPPUNIT_LOG_LINE("Start consumer " << HEXOUT(_thread.get_id())) ;

    for(int i = 0 ; ++i <= 2 ;) try
    {
        for (;;)
        {
            const unsigned count = std::min((size_t)_generate(), std::max(_queue.capacity()/2, (size_t)1)) ;

            if (count == 1)
            {
                _consumed.push_back(_queue.pop()) ;
                ++_total ;
            }
            else
            {
                const auto &r = _queue.pop_some(count) ;
                const ptrdiff_t popped_count = std::distance(r.first, r.second) ;

                PCOMN_VERIFY(popped_count) ;
                PCOMN_VERIFY(popped_count <= count) ;

                _consumed.insert(_consumed.end(), r.first, r.second) ;
                _total += popped_count ;
            }

        }
    }
    catch (const sequence_closed &)
    {
        CPPUNIT_LOG_LINE("Queue closed in consumer " << HEXOUT(_thread.get_id())
                         << ", attempt " << i << ", consumed " << _total << " items") ;

        std::this_thread::sleep_for(generate_pause()) ;
    }

    CPPUNIT_LOG_LINE("Finish consumer " << HEXOUT(_thread.get_id())
                     << ", consumed " << _total << " items.") ;
}

/*******************************************************************************
 BlockingQueueFuzzyTests
*******************************************************************************/
void BlockingQueueFuzzyTests::run(unsigned producers, unsigned consumers,
                                  unsigned pcount,
                                  nanoseconds max_pause, milliseconds before_close)
{
    const uint64_t total_volume = pcount*producers ;

    const nanoseconds consumers_timeout (std::max(consumers*100*max_pause, nanoseconds(50ms))) ;

    CPPUNIT_LOG_LINE("Running " << producers << " producers, " << consumers << " consumers, "
                     << total_volume << " total items (" << pcount << " per producer), "
                     << "max pause "<< (duration<double,std::micro>(max_pause).count()) << "ms") ;

    PRealStopwatch wall_time ;
    PCpuStopwatch  cpu_time ;

    counting_blocqueue cbq (200) ;

    wall_time.start() ;
    cpu_time.start() ;

    const auto make_testers = [&](TesterMode mode, auto &testers, size_t count)
    {
        CPPUNIT_ASSERT(testers.empty()) ;
        for (testers.reserve(count) ; count ; --count)
        {
            testers.emplace_back(new tester_thread(mode, cbq, pcount, 0.01, max_pause)) ;
        }
    } ;

    make_testers(Consumer, _consumers, consumers) ;
    make_testers(Producer, _producers, producers) ;

    std::this_thread::sleep_for(before_close) ;

    if (before_close != nanoseconds())
        CPPUNIT_LOG_LINE("Closing the push end: " << cbq.close_push()) ;

    join_producers() ;

    if (before_close == nanoseconds())
        CPPUNIT_LOG_LINE("Closing the push end: " << cbq.close_push()) ;

    join_consumers() ;

    const auto eval_total = [](auto &testers, size_t init)
    {
        return std::accumulate(testers.begin(), testers.end(), init,
                               [](size_t total, const auto &p) { return total + tester(p)._total ; }) ;
    } ;

    cpu_time.stop() ;
    wall_time.stop() ;

    const size_t total_produced = eval_total(_producers, 0) ;
    const size_t total_consumed = eval_total(_consumers, 0) ;

    CPPUNIT_LOG_LINE("Finished in " << wall_time << " real time, " << cpu_time << " CPU time.") ;
    CPPUNIT_LOG_LINE(total_produced << " produced, " <<  total_consumed << " consumed") ;

    //CPPUNIT_LOG_EQUAL(total_produced, total_volume) ;
    CPPUNIT_LOG_EQUAL(total_consumed, total_produced) ;
}

/*******************************************************************************
 main
*******************************************************************************/
int main(int argc, char *argv[])
{
    return pcomn::unit::run_tests
        <
            //BlockingQueueTests,
            BlockingQueueFuzzyTests
        >
        (argc, argv) ;
}
