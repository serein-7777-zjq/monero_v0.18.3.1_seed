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
// Incoming connection logger for experiment analysis.

#include "incoming_connection_logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace nodetool
{

incoming_connection_logger::~incoming_connection_logger()
{
}

void incoming_connection_logger::init(const std::string& log_path)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_initialized)
    return;
  m_log_path = log_path;
  m_initialized = true;
}

void incoming_connection_logger::write_line(const char* event, const std::string& address_str, const char* extra)
{
  if (!m_initialized || m_log_path.empty())
    return;

  auto now = std::chrono::system_clock::now();
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

  std::ostringstream line;
  line << "[" << sec << "." << std::setfill('0') << std::setw(3) << ms << "] "
       << event << " " << address_str;
  if (extra && extra[0] != '\0')
    line << " " << extra;
  line << "\n";

  std::ofstream f(m_log_path, std::ios::app);
  if (f)
    f << line.str();
}

void incoming_connection_logger::log_attempt(const std::string& address_str)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  write_line("INCOMING_ATTEMPT", address_str, nullptr);
}

void incoming_connection_logger::log_rejected(const std::string& address_str, const std::string& reason)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string extra = "reason=" + reason;
  write_line("INCOMING_REJECTED", address_str, extra.c_str());
}

void incoming_connection_logger::log_established(const std::string& address_str)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  write_line("INCOMING_ESTABLISHED", address_str, nullptr);
}

void incoming_connection_logger::log_closed(const std::string& address_str, const char* direction)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string extra = std::string("direction=") + direction;
  write_line("CONNECTION_CLOSED", address_str, extra.c_str());
}

} // namespace nodetool
