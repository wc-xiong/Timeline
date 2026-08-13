#pragma once
#include <map>
#include <set>

namespace std
{
	template <typename One, typename N>
	class one_n_link
	{
	public:

		void add_one(const N &n, const One &one)
		{
			m_one_n_map[one].insert(n);
			m_n_one_map[n] = one;
		}

		bool erase_n(const N &n, One *pOne = nullptr)
		{
			auto it = m_n_one_map.find(n);
			if (it == m_n_one_map.end())
				return false;
			if (pOne)
				*pOne = it->second;
			m_one_n_map[it->second].erase(n);
			if (m_one_n_map[it->second].size() == 0)
				m_one_n_map.erase(it->second);

			m_n_one_map.erase(it);
			return true;
		}

		bool erase_one(const One &one, std::set<N> *pNs = nullptr)
		{
			auto it = m_one_n_map.find(one);
			if (it == m_one_n_map.end())
				return false;
			if (pNs)
				*pNs = it->second;
			for (const auto n : it->second)
			{
				m_n_one_map.erase(n);
			}
			m_one_n_map.erase(it);
			return true;
		}

		void clear()
		{
			m_n_one_map.clear();
			m_one_n_map.clear();
		}

		std::size_t is_alive(const One &one) const
		{
			return m_one_n_map.count(one);
		}

		bool get_ns(const One &one, std::set<N> &ns) const
		{
			auto it = m_one_n_map.find(one);
			if (it == m_one_n_map.end())
				return false;
			ns = it->second;
			return true;
		}

		bool get_one(const N &n, One &one) const
		{
			auto it = m_n_one_map.find(n);
			if (it == m_n_one_map.end())
				return false;
			one = it->second;
			return true;
		}

		const std::map<One, std::set<N>> &get_one_n_map() const
		{
			return m_one_n_map;
		}

		const std::map<N, One> &get_n_one_map() const
		{
			return m_n_one_map;
		}

	private:
		std::map<One, std::set<N>> m_one_n_map;
		std::map<N, One> m_n_one_map;
	};
}
