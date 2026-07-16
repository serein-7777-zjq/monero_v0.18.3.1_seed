// Copyright (c) 2014-2024, The Monero Project
//
// Outbound connection debug logger - writes to separate file only.

#include "outbound_connection_logger.h"
#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace nodetool
{

namespace
{
  std::string g_log_path;
  std::mutex g_mutex;
  bool g_initialized = false;
}

void outbound_debug_log_init(const std::string& log_path)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  if (g_initialized)
    return;
  g_log_path = log_path;
  try
  {
    boost::filesystem::path p(log_path);
    boost::filesystem::path parent = p.parent_path();
    if (!parent.empty())
      boost::filesystem::create_directories(parent);
    std::ofstream f(g_log_path, std::ios::app);
    if (f)
    {
      auto now = std::chrono::system_clock::now();
      auto sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
      f << "[" << sec << ".000] [OUTBOUND_DEBUG] LOG_STARTED\n";
    }
  }
  catch (...) {}
  g_initialized = true;
}

void outbound_debug_log(const std::string& msg)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  if (!g_initialized || g_log_path.empty())
    return;

  auto now = std::chrono::system_clock::now();
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

  std::ostringstream line;
  line << "[" << sec << "." << std::setfill('0') << std::setw(3) << ms << "] [OUTBOUND_DEBUG] " << msg << "\n";

  std::ofstream f(g_log_path, std::ios::app);
  if (f)
    f << line.str();
}

} // namespace nodetool
