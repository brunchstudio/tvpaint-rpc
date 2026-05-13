#ifdef _WIN32
// Include <winsock2.h> before <windows.h> (needed for websocketpp)
#include <winsock2.h>
#endif

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include "TVPaintAnimationSDK/TVPaintSDK.h"

#include "./rpc.hpp"
#include "./server.hpp"
#include "./utils.hpp"

// Global WebSockets server instance
WSServer *wsserver;

// Global requester pointer
INTPTR req;

/**
 * Replaces the default logger and log to a file
 */
void replace_default_logger(const char *log_path) {
  auto log_file = std::string(log_path) + "/" + ".tvpaint-rpc.log";
  auto max_size = 1048576 * 1; // 1 Mb
  auto max_files = 2;
  auto logger = spdlog::rotating_logger_mt("tvpaint-ws-server", log_file,
                                           max_size, max_files);

  spdlog::flush_on(spdlog::level::info);
  spdlog::set_default_logger(logger);
}

INTPTR create_requester(PIPlugin *iFilter) {
  int width = 150;
  int height = 80;
  int text_margin = 10;

  // Create an empty requester to force enabling ticks
  // The requester is hidden
  INTPTR req = TVOpenFilterReqEx(iFilter, width, height, NULL, NULL,
                                 bPIRequesterFlags_Hidden,
                                 bPIFilterFlags_NoTopBar);

  TVGrabTicks(iFilter, req, kPITicks_On);

  return req;
}

/**
 * Called first during the TVPaint plugin initialization
 */
int STDCALL PI_Open(PIPlugin *iFilter) {
  // Use log path env variable to configure the log location
  const char *log_path = std::getenv("TVP_RPC_LOG_PATH");

  if (log_path) {
    replace_default_logger(log_path);
  }

  // Set plugin name
  strcpy(iFilter->PIName, "George WebSocket server");

  // Set plugin version.revision
  iFilter->PIVersion = 1;
  iFilter->PIRevision = 0;

  // Get the listen port
  const char *port_env = std::getenv("TVP_RPC_WS_PORT");
  int port = port_env ? atoi(port_env) : 3000;

  // Create a new server instance
  wsserver = new WSServer(iFilter);

  // Start the WebSocket server
  try {
    wsserver->run(port);
  } catch (const std::exception &e) {
    spdlog::error(e.what());
  }

  // Open the window on start (needs to be open in order to tick and process WS
  // messages)
  req = create_requester(iFilter);

  return 1;
}

/**
 * Called on plugin shutdown, do the necessary cleanup here
 */
void STDCALL PI_Close(PIPlugin *iFilter) {
  // Shutting down the server
  if (wsserver) {
    wsserver->stop();
    delete wsserver;
  }
}

/**
 * Handle George commands in the main thread
 */
void processGeorgeCommands(PIPlugin *iFilter) {
  while (!wsserver->george_commands.empty()) {
    auto payload = wsserver->george_commands.front();

    // Execute the George command and store the result
    char george_result[2048];
    int executionStatus =
        TVSendCmd(iFilter, payload.command.c_str(), george_result);

    if (executionStatus == NULL) { // Handle an error
      auto error = json_rpc_error(payload.id, JSON_RPC_SERVER_ERROR,
                                  "Error when executing George command");
      wsserver->wsserver.send(payload.hdl, error, payload.opcode);
    } else { // Send the result
      auto json_response = json_rpc_result(payload.id, george_result);
      wsserver->wsserver.send(payload.hdl, json_response, payload.opcode);
    }

    wsserver->george_commands.pop();
  }
}

/**
 * We have something to process
 */
int STDCALL PI_Msg(PIPlugin *iFilter, INTPTR iEvent, INTPTR iReq,
                   INTPTR *iArgs) {
  switch (iEvent) {
  case kPIEvents_Ticks: // Called every 20 milliseconds at each timer ticks
    processGeorgeCommands(iFilter);
    break;
  case kPIEvents_WindowClose:
    // The requester is closed
    req = 0;
    break;
  }

  return 1;
}

// Below this line, not needed for this plugin
// ------------------------------------------------------------------

/**
 * Initializes the settings of the parameters.
 */
int STDCALL PI_Parameters(PIPlugin *iFilter, const char *iArg) { return 1; }

void STDCALL PI_About(PIPlugin *iFilter) {}

int STDCALL PI_Start(PIPlugin *iFilter, double pos, double size) {
  return 1;
}

int STDCALL PI_Work(PIPlugin *iFilter) { return 1; }

void STDCALL PI_Finish(PIPlugin *iFilter) {}
