#ifndef LOGGER_H
#define LOGGER_H

#include <cassert>
#include <queue>
#include <stdlib.h>
#include "P7_Trace.h"
#include <string>
#include <vector>
#include <iostream>

typedef IP7_Trace p7log_t;
typedef IP7_Trace::hModule p7mod_t;

#define DEFAULT_UI_UDP_SINK_PORT 9393
inline bool console_out{false};

enum class LogType_e : uint8_t {
  INFO = 0x0,
  WARN,
  DEBUG,
  ERROR,
  CRIT
};

class P7Logger
{
  public:
    P7Logger(bool init = true)
    {
      if(init) initLogger();
    }

    void initLogger() {
      if(initialized) return;
      P7_Set_Crash_Handler();
      std::string p7ConfString;

      if(!console_out){
        p7ConfString += "/P7.Sink=UdpServer ";
        p7ConfString += "/P7.UdpPort=" + std::to_string(DEFAULT_UI_UDP_SINK_PORT);
        p7ConfString += " /P7.Format=\"\x1F%ix\x1F%id\x1F%lv\x1F%cn\x1F%mi\x1F%mn\x1F%cc\x1F%ti\x1F%tn\x1F%fs\x1F%fl\x1F%fn\x1F%tf\x1F%ms\x1F\"";
        m_client = P7_Create_Client(TM(p7ConfString.c_str()));
      } else {
        m_client = P7_Create_Client(TM(
            "/P7.Sink=Console "
            "/P7.Addr=172.16.8.16 "
            "/P7.Pool=64 "
            "/P7.Name=MADC_TSW "
            "/P7.Roll=00:00tm "
            "/P7.Format=\"%ix [%tm] %mn {%lv} %ms\""
            ));
      }

      m_logger = P7_Create_Trace(m_client, TM("log"));
      m_logger->Share("SharedLog");
      m_logger->Register_Thread(TM("MainMadc_tsw"), 0);

      m_logger->Set_Verbosity(nullptr, eP7Trace_Level::EP7TRACE_LEVEL_TRACE);
      initAllRequestedMods(); // FIXME: Check initialization
      initialized = true;
    }

    static void requestModRegister(const char* name, p7mod_t* mod_handle){
      slave_mods.push_back({name, mod_handle});
    }

    static void initAllRequestedMods(){
      for(auto& m : slave_mods)
        m_logger->Register_Module(TM(m.mod_name.c_str()), m.mod_handle);
      slave_mods.clear();

      while(!msg_queue.empty()){
        const auto& queue_msg = msg_queue.front();
        const auto mod = *queue_msg.mod;
        const auto* msg = queue_msg.msg.c_str();

        switch(queue_msg.ltype){
          case LogType_e::INFO:   m_logger->P7_INFO     (mod, TM(msg));  break;
          case LogType_e::WARN:   m_logger->P7_WARNING  (mod, TM(msg));  break;
          case LogType_e::DEBUG:  m_logger->P7_DEBUG    (mod, TM(msg));  break;
          case LogType_e::ERROR:  m_logger->P7_ERROR    (mod, TM(msg));  break;
          case LogType_e::CRIT:   m_logger->P7_CRITICAL (mod, TM(msg));  break;
          default:                m_logger->P7_DEBUG    (mod, TM(msg));  break;
        }
        msg_queue.pop();
      }
    }

    static p7log_t* getLogger(){
      assert(m_logger != nullptr && "Don't call the P7Logger before its initialization");
      return m_logger;
    }

    template<typename ...Args>
    static void log(p7mod_t* mod, const LogType_e ltype, const char* msg, Args&&... args){
      if(!m_logger){
        std::string form_msg = strFormat(msg, std::forward<Args>(args)...);
        msg_queue.push({mod, form_msg, ltype});
        return;
      }

      switch(ltype){
        case LogType_e::INFO:   m_logger->P7_INFO     (mod, TM(msg), args...);  break;
        case LogType_e::WARN:   m_logger->P7_WARNING  (mod, TM(msg), args...);  break;
        case LogType_e::DEBUG:  m_logger->P7_DEBUG    (mod, TM(msg), args...);  break;
        case LogType_e::ERROR:  m_logger->P7_ERROR    (mod, TM(msg), args...);  break;
        case LogType_e::CRIT:   m_logger->P7_CRITICAL (mod, TM(msg), args...);  break;
        default:                m_logger->P7_DEBUG    (mod, TM(msg), args...);  break;
      }
    }

    ~P7Logger()
    {
      if(!initialized) return;
      m_logger->Unregister_Thread(0);
      m_logger->Release();
      m_client->Release();
    }

  private:
    template<typename ...Args>
    static std::string strFormat(const char* fmt_str, Args&&... args) {
      int size = snprintf(nullptr, 0, fmt_str, std::forward<Args>(args)...);
      if (size <= 0) return "";
      std::string result(size, '\0');

      snprintf(&result[0], size + 1, fmt_str, std::forward<Args>(args)...);
      return result;
    }

  private:
    struct ModuleRecord_t {
      std::string mod_name;
      p7mod_t* mod_handle;
    };

    struct QueueMsg_t {
      p7mod_t*    mod;
      std::string msg;
      LogType_e   ltype;
    };

    inline static std::vector<ModuleRecord_t> slave_mods;
    inline static std::queue<QueueMsg_t> msg_queue; // NOTE: Only messages before initialization
    inline static bool initialized{false};
    inline static IP7_Client* m_client;
    inline static p7log_t*  m_logger = nullptr;
};

#define INFO(msg, ...)  do{ P7Logger::log(&this->m_module, LogType_e::INFO,  msg, ##__VA_ARGS__); }while(0)
#define WARN(msg, ...)  do{ P7Logger::log(&this->m_module, LogType_e::WARN,  msg, ##__VA_ARGS__); }while(0)
#define DEBUG(msg, ...) do{ P7Logger::log(&this->m_module, LogType_e::DEBUG, msg, ##__VA_ARGS__); }while(0)
#define ERROR(msg, ...) do{ P7Logger::log(&this->m_module, LogType_e::ERROR, msg, ##__VA_ARGS__); }while(0)
#define CRIT(msg, ...)  do{ P7Logger::log(&this->m_module, LogType_e::CRIT,  msg, ##__VA_ARGS__); }while(0)

#endif // LOGGER_H
