#pragma once

#include <map>
#include <thread>

#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "TVPaintAnimationSDK/TVPaintSDK.h"

#include "./queue.hpp"

using websocketpp::connection_hdl;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::thread> server_thread;

struct GeorgeCommand {
  int id;
  std::string command;
  connection_hdl hdl;
  websocketpp::frame::opcode::value opcode;
};

/**
 * Wrapper class for the WebSocket server
 */
class WSServer {
private:
  /* The server thread */
  server_thread run_thread;

  /* TVPaint plugin instance pointer */
  PIPlugin *iFilter;

  void on_open(connection_hdl hdl);
  void on_message(server *s, connection_hdl hdl, server::message_ptr msg);

public:
  /* websocketpp server instance with Boost Asio */
  server wsserver;

  /* Queue storing george commands to process */
  ThreadSafeQueue<GeorgeCommand> george_commands;

  WSServer(PIPlugin *iFilter);

  void run(uint16_t port);
  void stop();
};