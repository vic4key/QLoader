#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/fifo_map.hpp>

// using json = nlohmann::json; // ordered

template<class K, class V, class dummy_compare, class A>
using unordered_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<unordered_fifo_map>;