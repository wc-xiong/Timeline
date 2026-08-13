#pragma once
#include <string>
#include <sstream>
#include <functional>
#include <utility>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <map>
#include <type_traits>
#include <future>

#pragma warning(disable:4244)

// 与系统无关的常用 API

template <typename TChar>
struct LOG__
{
	~LOG__()
	{
		m_oss << "\n";
		ODS(m_oss.str().c_str());
	}

	void __stdcall ODS(char const *str)
	{
		OutputDebugStringA(str);
	}
	void __stdcall ODS(wchar_t const *str)
	{
		OutputDebugStringW(str);
	}

	template <typename T>
	LOG__ &operator<<(T const &v)
	{
		m_oss << v;
		return *this;
	}

	LOG__ &IPv4Addr(uint32_t addr)
	{
		m_oss << (int)((uint8_t*)&addr)[3] << "." << (int)((uint8_t*)&addr)[2] << "." << (int)((uint8_t*)&addr)[1] << "." << (int)((uint8_t*)&addr)[0];
		return *this;
	}

	std::basic_ostringstream<TChar> m_oss;
};
using LOGA = LOG__<char>;
using LOGW = LOG__<wchar_t>;

static inline std::wstring TranslateFilePath(const std::wstring &dir, std::wstring path)
{
	auto newPath = dir;
	if (dir.size() && dir.back() != L'\\')
		newPath += L'\\';
	if (path.size() && path.back() == L'\\')
		path.pop_back();
	for (auto c : path)
	{
		if (c == L':' || c == L'\\')
		{
			c = L'_';
		}
		newPath.push_back(c);
	}
	return newPath;
}

template<typename Map, typename K, typename V>
static inline V map_find_or(const Map &map, const K &k, const V &v)
{
	auto it = map.find(k);
	if (it == map.end())
		return v;
	return it->second;
}

static inline std::string WideStringToAscii(std::wstring const &ws)
{
	std::string as;
	for (auto c : ws)
	{
		as.push_back(c);
	}
	return as;
}

static inline std::wstring AsciiStringToWide(std::string const &as)
{
	std::wstring ws;
	for (auto c : as)
	{
		ws.push_back(c);
	}
	return ws;
}

namespace std
{
	template <typename TChar>
	static inline basic_string<TChar> trim_left(basic_string<TChar> const &str)
	{
		bool found = false;
		basic_string<TChar> ret;
		for (auto c : str)
		{
			if (!found && !isspace(c))
			{
				found = true;
			}
			if (found)
			{
				ret.push_back(c);
			}
		}
		return ret;
	}

	template <typename TChar>
	static inline basic_string<TChar> &trim_right(basic_string<TChar> &str)
	{
		while (str.size() && isspace(str.back()))
		{
			str.pop_back();
		}
		return str;
	}

	template <typename TChar>
	static inline basic_string<TChar> trim_lr(basic_string<TChar> const &str)
	{
		auto ret = trim_left(str);
		return trim_right(ret);
	}

	// 拥有单独线程的类
	class thread_owner
	{
	public:
		virtual ~thread_owner() = default;

		void start()
		{
			if (m_thread)
				return;
			m_thread = std::make_unique<std::thread>([&]()
			{
				m_running = true;
				thrd_proc();
				m_running = false;
			});
		}

		void stop()
		{
			m_running = false;
			if (m_thread && m_thread->joinable())
			{
				m_thread->join();
			}
			m_thread.reset();
		}

	protected:

		virtual void thrd_proc() = 0;

		// 线程退出指示变量
		std::atomic_bool m_running = false;
		std::unique_ptr<std::thread> m_thread;
	};

	template<typename Map>
	static inline auto map_to_vec(Map const &map)
	{
		using Key = std::remove_cv_t<decltype(map.begin()->first)>;
		using Value = std::remove_cv_t<decltype(map.begin()->second)>;
		std::vector<std::pair<Key, Value>> pairs;
		for (auto &pair : map)
		{
			pairs.push_back(std::make_pair(pair.first, pair.second));
		}
		return pairs;
	}
	template<typename Vec, typename Keyer, typename Valuer>
	static inline auto vec_to_map(Vec const &vec, Keyer &&keyer, Valuer &&valuer)
	{
		using Key = std::remove_cv_t<decltype(keyer(vec[0]))>;
		using Value = std::remove_cv_t<decltype(valuer(vec[0]))>;
		std::map<Key, Value> map;
		for (auto &v : vec)
		{
			map.insert({ keyer(v), valuer(v) });
		}
		return map;
	}
	template<typename Vec, typename Keyer>
	static inline auto vec_to_map(Vec const &vec, Keyer &&keyer)
	{
		return vec_to_map(vec, keyer, [](auto &v) { return v; });
	}

	template<typename TFirst, typename TSecond, typename TThird>
	static inline void tupleVec_to_pairVec(const vector<tuple<TFirst, TSecond, TThird>>& vecT, vector<pair<TFirst, TSecond>>& vecP)
	{
		for (const auto& v : vecT)
			vecP.emplace_back(std::get<0>(v), std::get<1>(v));
	}

	template <typename It>
	class range_impl
	{
		It m_begin;
		It m_end;
	public:
		range_impl(It begin, It end) : m_begin(begin), m_end(end) {}

		It begin() const
		{
			return m_begin;
		}
		It end() const
		{
			return m_end;
		}
	};
	template <typename It>
	static inline range_impl<It> range(It begin, It end)
	{
		return range_impl<It>(begin, end);
	}

	// 异步的 for_each 实现
	template <typename It, typename Fn>
	static inline void async_for_each(It begin, It end, bool useCurrentThread, Fn fn)
	{
		//assert(std::is_void_v<decltype(fn(*begin))>);
		if (begin == end)
			return;
		auto first = begin;
		if (useCurrentThread)
		{
			++begin;
		}
		std::vector<std::future<void>> futures;
		for (auto &element : std::range(begin, end))
		{
			futures.push_back(std::async(std::launch::async, [&]() { fn(element); }));
		}
		if (useCurrentThread)
		{
			fn(*first);
		}
		std::for_each(futures.begin(), futures.end(), [](auto &f) { f.get(); });
	}

	class read_write_mutex
	{
	private:
		std::mutex readMtx;		// 锁住读锁操作
		std::mutex writeMtx;
		int readCnt;			// 已加读锁个数
		bool bWrite;			// 标识当前写是否被占用，存在读者/写者都为true
		std::condition_variable cv;
	public:
		explicit read_write_mutex() : readCnt(0), bWrite(false) {}

		void lock_shared()
		{
			std::lock_guard<std::mutex> lk(readMtx);
			if (++readCnt == 1)
			{
				std::unique_lock<std::mutex> ul(writeMtx);
				cv.wait(ul, [this]() { return bWrite == false; });
				bWrite = true;
			}
		}
		void unlock_shared()
		{
			std::lock_guard<std::mutex> lk(readMtx);
			if (--readCnt == 0)
			{
				std::unique_lock<std::mutex> ul(writeMtx);
				bWrite = false;
				cv.notify_one();
			}
		}
		void lock()
		{
			std::unique_lock<std::mutex> ul(writeMtx);
			cv.wait(ul, [this]() { return bWrite == false; });
			bWrite = true;
		}
		void unlock()
		{
			std::unique_lock<std::mutex> ul(writeMtx);
			bWrite = false;
			cv.notify_one();
		}
	};
}

static void Base16Encode(const void *pBytes, size_t len, std::string &x16Str)
{
	if (pBytes == nullptr || len == 0)
		return;

	static const char X16_CHARS[] = "0123456789ABCDEF";

	BYTE *ptr = (BYTE*)pBytes;
	x16Str.resize(len * 2);
	for (size_t i = 0; i < len; i++)
	{
		uint8_t low = ptr[i] & 0x0F;
		uint8_t high = (ptr[i] & 0xF0) >> 4;
		x16Str[i << 1] = X16_CHARS[high];
		x16Str[(i << 1) + 1] = X16_CHARS[low];
	}
}

static inline void Base16Encode(const std::string &in, std::string &out)
{
	Base16Encode(in.data(), in.size(), out);
}

template<typename T>
static inline std::string Base16Encode(T const &t)
{
	std::string out;
	Base16Encode(&t, sizeof(t), out);
	return out;
}

static inline std::string Base16Decode(const std::string &in)
{
	if (in.length() % 2 != 0)
		return "";

	std::string out;
	for (std::size_t i = 0; i < in.length();)
	{
		char buf[3] = "";
		buf[0] = in[i++];
		buf[1] = in[i++];
		int c = 0;
		if (sscanf_s(buf, "%x", &c) != 1)
		{
			return "";
		}
		out.push_back(c);
	}
	return out;
}

