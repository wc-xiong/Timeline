#pragma once
#include "ThreadPool.h"
#include "common/Common/Timer.h"
#include "ThreadManagerGlobal.h"

#include <QtConcurrent>
#include <QThreadPool>
#include <QFuture>

#include <vector>
#include <memory>
#include <future>

class THREADMANAGER_API CThreadManager
{
public:
	CThreadManager();
	~CThreadManager();

	/**
	 * @brief 提交一个周期任务，开辟C++子线程周期执行
	 *
	 * @tparam F 可调用对象的类型（如函数、函数对象、lambda、类成员函数（用bind））。
	 * @tparam Args 可调用对象的参数类型。
	 * @param f 可调用对象，表示任务逻辑。
	 * @param args 可调用对象所需的参数,不支持右值。
	 *
	 * @note 因为模板的定义必须在实例化时可见，所以这里不分离声明和定义
	 */
	template<class F, class... Args>
	void AddPeriodCTask(double dInterval,F&& f, Args&&... args)
	{
		using namespace std;
		auto task = bind(forward<F>(f), ref(forward<Args>(args))...);
	
		auto timer = std::make_unique<CTimer>(task, dInterval);
		timer->Start();

		std::lock_guard<std::mutex> lock{ m_lockTask };
		m_vecThreads.push_back(std::move(timer));
	}

	/**
	 * @brief 提交一个临时任务到C++线程池，并返回任务的期望，
	 *		  调用期望.get()获取返回值会等待任务执行完,获取返回值，亦可用于判断任务是否完成
	 *
	 * @tparam F 可调用对象的类型（如函数、函数对象、lambda、类成员函数（用bind））。
	 * @tparam Args 可调用对象的参数类型。
	 * @param f 可调用对象，表示任务逻辑。
	 * @param args 可调用对象所需的参数。
	 * @param priority 任务优先级，默认为一般。
	 * @return std::future<decltype(f(args...))>
	 *         包含任务结果的 `std::future` 对象，用于获取任务的异步执行结果。
	 *
	 * @note 因为模板的定义必须在实例化时可见，所以这里不分离声明和定义
	 */
	template<class F, class... Args>
	auto AddTemporaryTask(F&& f, int priority = ETL_Normal, Args&&... args)->std::future<decltype(f(args...))>
	{
		using namespace std;
		using ReturnType = decltype(f(args...));

		auto task = make_shared<packaged_task<ReturnType()>>(
			bind(forward<F>(f), ref(forward<Args>(args))...)
			);

		future<ReturnType> fu = task->get_future();

		STask s;
		s.m_task = [task]() {(*task)(); };
		s.m_eLevel = (ETaskLevel)priority;

		std::lock_guard<std::mutex> lock{ m_lockTask };
		m_CThreadPool.CommitTask(s);

		return fu;
	}


	/**
	 * @brief 提交一个临时任务到Qt线程池，并返回任务的期望
	 *
	 * @tparam F 可调用对象的类型（如函数、函数对象、lambda、类成员函数（用bind））。
	 * @tparam Args 可调用对象的参数类型。
	 * @param f 可调用对象，表示任务逻辑。
	 * @param args 可调用对象所需的参数。
	 * @return QFuture<decltype(f(args...))>
	 *         包含任务结果的 QFuture 对象，用于获取任务的异步执行结果。
	 *
	 * @note 因为模板的定义必须在实例化时可见，所以这里不分离声明和定义
	 */
	template<class F, class... Args>
	auto AddQtTask(F&& f, Args&&... args)-> QFuture<decltype(f(args...))>
	{
		using namespace std;
		std::lock_guard<std::mutex> lock{ m_lockTask };
		return QtConcurrent::run(m_QtThreadPool, std::bind(std::forward<F>(f), ref(forward<Args>(args))...));

	}

private:
	std::mutex m_lockTask;											//多线程提交任务同步锁

	//c++线程管理
	std::vector<std::unique_ptr<CTimer>> m_vecThreads;
	CThreadPool m_CThreadPool;

	//qt线程管理
	QThreadPool* m_QtThreadPool;
};

 