# Monero 0.18.4.3_seed 修改迁移到 0.18.3.1 指南

本文档将 0.18.4.3_seed（基线 ee619855d）上的全部修改迁移到 Monero 0.18.3.1，逐项说明对应位置和可用的修改代码。

---

## 1. contrib/epee/include/net/abstract_tcp_server2.h

### 1.1 添加 `#include <functional>`
**0.18.3.1 位置**: 第 39 行后（`#include <vector>` 之后）

```cpp
#include <string>
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>
```

### 1.2 添加 i_connection_limit 接口及 plimit
**版本差异说明**: 0.18.3.1 中**不存在** `i_connection_limit` 接口和 `plimit`。0.18.4.3 在 TCP 接受连接时即检查连接限制，而 0.18.3.1 在 handshake 阶段才检查。为完整迁移功能，需在 0.18.3.1 中新增该接口。

**0.18.3.1 位置**: 在 `i_connection_filter` 结构体之后（约第 77 行后）添加：

```cpp
  struct i_connection_limit
  {
    virtual bool is_host_limit(const epee::net_utils::network_address &address, std::string* reject_reason = nullptr)=0;
  protected:
    virtual ~i_connection_limit(){}
  };
```

### 1.3 修改 shared_state 结构体
**0.18.3.1 位置**: 第 254-262 行的 `shared_state` 结构体

**原代码**:
```cpp
    struct shared_state : connection_basic_shared_state, t_protocol_handler::config_type
    {
      shared_state()
        : connection_basic_shared_state(), t_protocol_handler::config_type(), pfilter(nullptr), stop_signal_sent(false)
      {}

      i_connection_filter* pfilter;
      bool stop_signal_sent;
    };
```

**修改为**:
```cpp
    struct shared_state : connection_basic_shared_state, t_protocol_handler::config_type
    {
      shared_state()
        : connection_basic_shared_state(), t_protocol_handler::config_type(), pfilter(nullptr), plimit(nullptr), stop_signal_sent(false)
      {}

      i_connection_filter* pfilter;
      i_connection_limit* plimit;
      std::function<void(const network_address&, const char*)> incoming_connection_callback;
      bool stop_signal_sent;
    };
```

### 1.4 添加 set_connection_limit 和 set_incoming_connection_callback 声明
**0.18.3.1 位置**: 第 354 行 `set_connection_filter` 之后

**添加**:
```cpp
    void set_connection_limit(i_connection_limit* plimit);
    void set_incoming_connection_callback(std::function<void(const network_address&, const char*)> cb);
```

---

## 2. contrib/epee/include/net/abstract_tcp_server2.inl

### 2.1 修改 start_internal 中的 filter/limit 检查逻辑
**0.18.3.1 位置**: 第 601-607 行

**原代码**:
```cpp
    auto *filter = static_cast<shared_state&>(
      connection_basic::get_state()
    ).pfilter;
    if (filter && !filter->is_remote_host_allowed(*real_remote))
      return false;
    ec_t ec;
```

**修改为**:
```cpp
    auto &state = static_cast<shared_state&>(connection_basic::get_state());
    auto *filter = state.pfilter;
    auto *limit = state.plimit;
    auto &incoming_cb = state.incoming_connection_callback;

    if (is_income && incoming_cb)
      incoming_cb(*real_remote, "ATTEMPT");

    if (filter && !filter->is_remote_host_allowed(*real_remote))
    {
      if (is_income && incoming_cb)
        incoming_cb(*real_remote, "REJECTED:blocked");
      return false;
    }

    if (is_income && limit)
    {
      std::string reject_reason;
      if (limit->is_host_limit(*real_remote, &reject_reason))
      {
        if (incoming_cb)
        {
          std::string event = "REJECTED:" + reject_reason;
          incoming_cb(*real_remote, event.c_str());
        }
        return false;
      }
    }

    ec_t ec;
```

### 2.2 添加 set_connection_limit 和 set_incoming_connection_callback 实现
**0.18.3.1 位置**: 第 956-960 行 `set_connection_filter` 之后

**原代码**:
```cpp
  template<class t_protocol_handler>
  void boosted_tcp_server<t_protocol_handler>::set_connection_filter(i_connection_filter* pfilter)
  {
    assert(m_state != nullptr); // always set in constructor
    m_state->pfilter = pfilter;
  }
  //---------------------------------------------------------------------------------
  template<class t_protocol_handler>
  bool boosted_tcp_server<t_protocol_handler>::run_server(
```

**修改为**:
```cpp
  template<class t_protocol_handler>
  void boosted_tcp_server<t_protocol_handler>::set_connection_filter(i_connection_filter* pfilter)
  {
    assert(m_state != nullptr); // always set in constructor
    m_state->pfilter = pfilter;
  }
  //---------------------------------------------------------------------------------
  template<class t_protocol_handler>
  void boosted_tcp_server<t_protocol_handler>::set_connection_limit(i_connection_limit* plimit)
  {
    assert(m_state != nullptr);
    m_state->plimit = plimit;
  }
  //---------------------------------------------------------------------------------
  template<class t_protocol_handler>
  void boosted_tcp_server<t_protocol_handler>::set_incoming_connection_callback(std::function<void(const network_address&, const char*)> cb)
  {
    assert(m_state != nullptr);
    m_state->incoming_connection_callback = std::move(cb);
  }
  //---------------------------------------------------------------------------------
  template<class t_protocol_handler>
  bool boosted_tcp_server<t_protocol_handler>::run_server(
```

---

## 3. contrib/epee/include/net/http_server_impl_base.h

**说明**: 0.18.3.1 中该文件**不存在** `is_host_limit` 的 override。经检查，0.18.3.1 的 http_server_impl_base.h 不继承 i_connection_limit，因此**无需修改**此文件。

---

## 4. contrib/epee/include/net/local_ip.h

### 4.1 注释 10.x 网段检测
**0.18.3.1 位置**: 第 51-53 行

**原代码**:
```cpp
      if( (ip | 0xffffff00) == 0xffffff0a)
        return true;
```

**修改为**:
```cpp
      // if( (ip | 0xffffff00) == 0xffffff0a)
      //   return true;
```

---

## 5. src/cryptonote_config.h

### 5.1 修改 P2P peerlist 限制
**0.18.3.1 位置**: 第 131-132 行

**原代码**:
```cpp
#define P2P_LOCAL_WHITE_PEERLIST_LIMIT                  1000
#define P2P_LOCAL_GRAY_PEERLIST_LIMIT                   5000
```

**修改为**:
```cpp
#define P2P_LOCAL_WHITE_PEERLIST_LIMIT                  500
#define P2P_LOCAL_GRAY_PEERLIST_LIMIT                   2500
```

---

## 6. src/p2p/incoming_connection_logger.cpp（新文件）

**0.18.3.1 位置**: 新建 `monero_v0.18.3.1/src/p2p/incoming_connection_logger.cpp`

可直接使用 patch 中的完整内容，无需修改。

---

## 7. src/p2p/incoming_connection_logger.h（新文件）

**0.18.3.1 位置**: 新建 `monero_v0.18.3.1/src/p2p/incoming_connection_logger.h`

可直接使用 patch 中的完整内容，无需修改。

---

## 8. src/p2p/net_node.h

### 8.1 添加 incoming_connection_logger.h 头文件
**0.18.3.1 位置**: 第 59 行后（`#include "common/command_line.h"` 之后）

```cpp
#include "common/command_line.h"
#include "incoming_connection_logger.h"
```

### 8.2 添加 i_connection_limit 接口实现声明
**0.18.3.1 位置**: 第 351 行 `is_remote_host_allowed` 之后

**添加**:
```cpp
    //----------------- i_connection_limit  ---------------------------------------------------------
    virtual bool is_host_limit(const epee::net_utils::network_address &address, std::string* reject_reason = nullptr);
```

### 8.3 修改 node_server 继承
**0.18.3.1 位置**: 第 126-128 行

**原代码**:
```cpp
  class node_server: public epee::levin::levin_commands_handler<p2p_connection_context_t<typename t_payload_net_handler::connection_context> >,
                     public i_p2p_endpoint<typename t_payload_net_handler::connection_context>,
                     public epee::net_utils::i_connection_filter
```

**修改为**:
```cpp
  class node_server: public epee::levin::levin_commands_handler<p2p_connection_context_t<typename t_payload_net_handler::connection_context> >,
                     public i_p2p_endpoint<typename t_payload_net_handler::connection_context>,
                     public epee::net_utils::i_connection_filter,
                     public epee::net_utils::i_connection_limit
```

### 8.4 添加 m_incoming_connection_logger 成员
**0.18.3.1 位置**: 第 391 行 `uint32_t max_connections;` 之后

**添加**:
```cpp
    nodetool::incoming_connection_logger m_incoming_connection_logger;
```

---

## 9. src/p2p/net_node.inl

### 9.1 添加 #include <cstring>
**0.18.3.1 位置**: 第 33 行后（`#include <algorithm>` 之后）

```cpp
#include <algorithm>
#include <cstring>
```

### 9.2 添加 is_host_limit 实现
**0.18.3.1 位置**: 在 `is_remote_host_allowed` 实现之后（约第 226 行后）

**注意**: 0.18.3.1 中 node_server 原本没有 is_host_limit。需新增该函数，将 handle_handshake 中的限制逻辑提取出来：

```cpp
  //-----------------------------------------------------------------------------------
  template<class t_payload_net_handler>
  bool node_server<t_payload_net_handler>::is_host_limit(const epee::net_utils::network_address &address, std::string* reject_reason)
  {
    const network_zone& zone = m_network_zones.at(address.get_zone());
    if (zone.m_current_number_of_in_peers >= zone.m_config.m_net_config.max_in_connection_count)
    {
      if (reject_reason)
        *reject_reason = "max_in_peers";
      MWARNING("Exceeded max incoming connections, so dropping this one.");
      return true;
    }

    if(has_too_many_connections(address))
    {
      if (reject_reason)
        *reject_reason = "too_many_connections";
      MWARNING("CONNECTION FROM " << address.host_str() << " REFUSED, too many connections from the same address");
      return true;
    }

    return false;
  }
```

### 9.3 修改 get_ip_seed_nodes 中的 TESTNET 种子节点
**0.18.3.1 位置**: 第 706-713 行

**原代码**:
```cpp
    if (m_nettype == cryptonote::TESTNET)
    {
      full_addrs.insert("176.9.0.187:28080");
      full_addrs.insert("88.99.173.38:28080");
      full_addrs.insert("51.79.173.165:28080");
      full_addrs.insert("192.99.8.110:28080");
      full_addrs.insert("37.187.74.171:28080");
    }
```

**修改为**:
```cpp
    if (m_nettype == cryptonote::TESTNET)
    {
      full_addrs.insert("10.150.0.71:28080");
      full_addrs.insert("10.151.0.71:28080");
      full_addrs.insert("10.152.0.71:28080");
      full_addrs.insert("10.153.0.71:28080");
      full_addrs.insert("10.154.0.71:28080");
      full_addrs.insert("10.155.0.71:28080");
    }
```

### 9.4 添加 incoming_connection_logger 初始化
**0.18.3.1 位置**: 第 938 行后（`m_config_folder = m_config_folder + "/" + public_zone.m_port;` 之后，`res = init_config()` 之前）

**添加**:
```cpp
    m_incoming_connection_logger.init(m_config_folder + "/monero_incoming_connections.log");
```

### 9.5 添加 set_connection_limit 和 set_incoming_connection_callback 调用
**0.18.3.1 位置**: 第 974 行 `set_connection_filter(this)` 之后

**原代码**:
```cpp
        zone.second.m_net_server.set_connection_filter(this);
        MINFO("Binding (IPv4) on " << zone.second.m_bind_ip << ":" << zone.second.m_port);
```

**修改为**:
```cpp
        zone.second.m_net_server.set_connection_filter(this);
        zone.second.m_net_server.set_connection_limit(this);
        zone.second.m_net_server.set_incoming_connection_callback(
          [this](const epee::net_utils::network_address& addr, const char* event) {
            if (!m_incoming_connection_logger.is_initialized())
              return;
            const std::string addr_str = addr.host_str();
            if (std::strcmp(event, "ATTEMPT") == 0)
              m_incoming_connection_logger.log_attempt(addr_str);
            else if (std::strncmp(event, "REJECTED:", 9) == 0)
              m_incoming_connection_logger.log_rejected(addr_str, event + 9);
          });
        MINFO("Binding (IPv4) on " << zone.second.m_bind_ip << ":" << zone.second.m_port);
```

### 9.6 修改 on_connection_new
**0.18.3.1 位置**: 第 2676-2679 行

**原代码**:
```cpp
  void node_server<t_payload_net_handler>::on_connection_new(p2p_connection_context& context)
  {
    MINFO("["<< epee::net_utils::print_connection_context(context) << "] NEW CONNECTION");
  }
```

**修改为**:
```cpp
  void node_server<t_payload_net_handler>::on_connection_new(p2p_connection_context& context)
  {
    if (context.m_is_income && m_incoming_connection_logger.is_initialized())
      m_incoming_connection_logger.log_established(context.m_remote_address.host_str());
    MINFO("["<< epee::net_utils::print_connection_context(context) << "] NEW CONNECTION");
  }
```

### 9.7 修改 on_connection_close
**0.18.3.1 位置**: 第 2682-2698 行

**原代码**:
```cpp
  void node_server<t_payload_net_handler>::on_connection_close(p2p_connection_context& context)
  {
    network_zone& zone = m_network_zones.at(context.m_remote_address.get_zone());
```

**修改为**（在函数开头添加 logger 调用）:
```cpp
  void node_server<t_payload_net_handler>::on_connection_close(p2p_connection_context& context)
  {
    if (context.m_is_income && m_incoming_connection_logger.is_initialized())
      m_incoming_connection_logger.log_closed(context.m_remote_address.host_str(), "IN");
    network_zone& zone = m_network_zones.at(context.m_remote_address.get_zone());
```

---

## 10. src/p2p/CMakeLists.txt

### 10.1 添加 incoming_connection_logger.cpp 到编译
**说明**: 0.18.3.1 的 p2p 使用 `file(GLOB P2P *)` 自动包含所有源文件，因此新建的 `incoming_connection_logger.cpp` 会自动被包含，**无需修改** CMakeLists.txt。

---

## 版本差异总结

| 项目 | 0.18.4.3 | 0.18.3.1 | 迁移说明 |
|------|-----------|----------|----------|
| i_connection_limit | 有 | 无 | 需在 abstract_tcp_server2.h 中新增 |
| http_server_impl_base is_host_limit | 有 override | 无 | 0.18.3.1 中该基类不实现 i_connection_limit，无需修改 |
| 连接限制检查时机 | TCP accept 时 | handshake 时 | 迁移后改为 TCP accept 时检查，与 0.18.4.3 一致 |
| get_ip_seed_nodes TESTNET | 10.150-155.0.71 | 公网 IP | 按实验需求修改为内网种子 |
