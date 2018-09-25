
#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <string>
#include <functional>

class VectorExtensions
{
public:
	/**
	 * Moves all items from the given set into a new vector
	 * @param elements The set to convert
	 * @return The created vector containing all elements of the set
	 */
	template<typename T>
	static std::vector<T> moveSetToVector(std::set<T>& elements)
	{
		std::vector<T> tmpVector;
		tmpVector.reserve(elements.size());
		std::move(elements.begin(), elements.end(), std::back_inserter(tmpVector));
		elements.clear();
		return tmpVector;
	}

	/**
	 * Copies all items from the given set into a new vector
	 * @param elements The set to convert
	 * @return The created vector containing all elements of the set
	 */
	template<typename T>
	static std::vector<T> copySetToVector(std::set<T>& elements)
	{
		std::vector<T> tmpVector;
		tmpVector.reserve(elements.size());
		std::copy(elements.begin(), elements.end(), std::back_inserter(tmpVector));
		return tmpVector;
	}

	/**
	 * Copies all items from the given vector into a new set
	 * @param elements The vector to convert
	 * @return The created set containing all elements of the vector
	 */
	template<typename T>
	static std::set<T> copyVectorToSet(const std::vector<T>& elements)
	{
		std::set<T> tmpSet;
		std::copy(elements.begin(), elements.end(), std::inserter(tmpSet, tmpSet.begin()));
		return tmpSet;
	}
	
	/**
	 * Check if all elements of the subset are contained in set
	 * @param completeSet The complete set
	 * @param subSet A subset of the complete set
	 * @return True if equals, false otherwise
	 */
	template<typename T>
	static bool contains(const std::vector<T>& completeSet, const std::vector<T>& subSet)
	{
		return std::all_of(subSet.begin(), subSet.end(), [&completeSet](const T& val) {
			return std::find(completeSet.begin(), completeSet.end(), val) != completeSet.end();
		});
	}
	
	
	/**
 * Check if all elements of the subset are contained in set
 * @param completeSet The complete set
 * @param needle A subset of the complete set
 * @return True if equals, false otherwise
 */
	template<typename T>
	static bool contains(const std::vector<T>& completeSet,const T& needle)
	{
		return std::find(completeSet.begin(), completeSet.end(), needle) != completeSet.end();
	}
	
	template<typename T>
	static std::string toString(const std::vector<T>& data)
	{
		if(data.size() == 0)
			return "[]";
		
		return "[" + std::accumulate(data.begin() + 1, data.end(), data[0], [](const std::string& first, const std::string& second)
		{
			return first + ", " + second;
		}) + "]";
	}
	
	template<typename T>
	static std::string toString(const T& data)
	{
		if(data.size() == 0)
			return "[]";
		
		return "[" + std::accumulate(data.begin() + 1, data.end(), data[0], [](const std::string& first, const std::string& second)
		{
			return first + ", " + second;
		}) + "]";
	}

    template<typename T, size_t N>
    static std::string toString(const std::array<T, N>& data, const std::string& (*convertFn)(const T&))
    {
        if(data.size() == 0)
            return "[]";

        return "[" + std::accumulate(data.begin() + 1, data.end(), convertFn(data[0]), [&](const std::string& first, const T& second)
        {
            return first + ", " + convertFn(second);
        }) + "]";
    }
	
	template<typename T>
	static std::string toString(const std::vector<T>& data, const std::string& (*convertFn)(const T&))
	{
		if(data.size() == 0)
			return "[]";

		return "[" + std::accumulate(data.begin() + 1, data.end(), convertFn(data[0]), [&](const std::string& first, const T& second)
		{
			return first + ", " + convertFn(second);
		}) + "]";
	}
	
	template<typename T>
	static std::string toStringForSettings(const T& data)
	{
		if(data.size() == 0)
			return "";
		
		return std::accumulate(data.begin() + 1, data.end(), data[0], [](const std::string& first, const std::string& second)
		{
			return first + ", " + second;
		});
	}
	
	template<typename T>
	static std::string toStringForSettings(const std::vector<T>& data, std::string(*convertFn)(const T))
	{
		if(data.size() == 0)
			return "";
		
		return std::accumulate(data.begin() + 1, data.end(), convertFn(data[0]), [&](const std::string& first, const T& second)
		{
			return first + ", " + convertFn(second);
		});
	}
	
	template<typename T>
	static std::string toStringForSettings(const std::initializer_list<T>&& data, std::string& (*convertFn)(const T&))
	{
		if(data.size() == 0)
			return "";

		return std::accumulate(data.begin() + 1, data.end(), convertFn(*data.begin()), [&](const std::string& first, const T& second)
		{
			return first + ", " + convertFn(second);
		});
	}
};
