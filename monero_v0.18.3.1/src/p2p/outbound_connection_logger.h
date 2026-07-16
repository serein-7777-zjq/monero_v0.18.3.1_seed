// Copyright (c) 2014-2024, The Monero Project
//
// Outbound connection debug logger - writes to separate file only, not to main log.

#pragma once

#include <string>

namespace nodetool
{

/// Initialize outbound debug logger with log file path. Call once from node_server::init.
void outbound_debug_log_init(const std::string& log_path);

/// Write a single line to the outbound debug log. No-op if not initialized.
void outbound_debug_log(const std::string& msg);

} // namespace nodetool
