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
// Incoming connection logger for experiment analysis - logs to separate file only.

#pragma once

#include <string>
#include <mutex>
#include <fstream>

namespace nodetool
{

/// Lightweight logger for incoming P2P connection events.
/// Writes to a separate file to avoid polluting main Monero logs.
/// Thread-safe, minimal lock hold time.
class incoming_connection_logger
{
public:
  incoming_connection_logger() = default;
  ~incoming_connection_logger();

  /// Initialize logger with log file path. Call once before use.
  void init(const std::string& log_path);

  /// Log incoming connection attempt.
  void log_attempt(const std::string& address_str);

  /// Log incoming connection rejected with reason.
  void log_rejected(const std::string& address_str, const std::string& reason);

  /// Log incoming connection established.
  void log_established(const std::string& address_str);

  /// Log connection closed (optional).
  void log_closed(const std::string& address_str, const char* direction);

  bool is_initialized() const { return m_initialized; }

private:
  void write_line(const char* event, const std::string& address_str, const char* extra = nullptr);

  std::string m_log_path;
  std::mutex m_mutex;
  bool m_initialized{false};
};

} // namespace nodetool
