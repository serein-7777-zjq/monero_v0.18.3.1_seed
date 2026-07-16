// Copyright (c) 2014-2024, The Monero Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Double spend logger for experiment analysis.

#include "double_spend_logger.h"
#include <boost/filesystem.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace cryptonote
{

double_spend_logger::~double_spend_logger()
{
}

void double_spend_logger::init(const std::string& log_path)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_initialized)
    return;
  m_log_path = log_path;
  try
  {
    boost::filesystem::path p(log_path);
    boost::filesystem::path parent = p.parent_path();
    if (!parent.empty())
      boost::filesystem::create_directories(parent);
    std::ofstream f(m_log_path, std::ios::app);
    if (f)
    {
      auto now = std::chrono::system_clock::now();
      auto sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
      f << "[" << sec << ".000] LOG_STARTED double_spend\n";
    }
  }
  catch (...) {}
  m_initialized = true;
}

void double_spend_logger::log_double_spend(const std::string& tx_id_hex, uint64_t timestamp, bool kept_by_block, const std::vector<std::string>& key_images_hex)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_initialized || m_log_path.empty())
    return;

  std::ostringstream line;
  line << "[DOUBLE_SPEND_DETECTED] tx_id=" << tx_id_hex
       << " timestamp=" << timestamp
       << " kept_by_block=" << (kept_by_block ? "1" : "0");
  for (size_t i = 0; i < key_images_hex.size(); ++i)
    line << " key_image[" << i << "]=" << key_images_hex[i];
  line << "\n";

  std::ofstream f(m_log_path, std::ios::app);
  if (f)
    f << line.str();
}

} // namespace cryptonote
