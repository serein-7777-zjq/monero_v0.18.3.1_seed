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
// Double spend logger for experiment analysis - logs to separate file only.

#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <vector>

namespace cryptonote
{

/// Lightweight logger for double spend detection events.
/// Writes to a separate file to avoid polluting main Monero logs.
/// Thread-safe, minimal lock hold time.
class double_spend_logger
{
public:
  double_spend_logger() = default;
  ~double_spend_logger();

  /// Initialize logger with log file path. Call once before use.
  void init(const std::string& log_path);

  /// Log a double spend detection event.
  /// @param tx_id_hex transaction hash in hex
  /// @param timestamp unix timestamp
  /// @param kept_by_block 0 or 1
  /// @param key_images_hex key images in hex, indexed by input order
  void log_double_spend(const std::string& tx_id_hex, uint64_t timestamp, bool kept_by_block, const std::vector<std::string>& key_images_hex);

  bool is_initialized() const { return m_initialized; }

private:
  std::string m_log_path;
  std::mutex m_mutex;
  bool m_initialized{false};
};

} // namespace cryptonote
