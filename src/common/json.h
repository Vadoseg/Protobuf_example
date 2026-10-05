#ifndef JSON_H
#define JSON_H

#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <stdexcept>
#include <vector>
#include <map>

#include "logger.h"

#define JSON_ERROR_RESPONSE_WITH_DATA

class ParseExc : std::runtime_error
{
  public:
    ParseExc(const std::string s = "") : std::runtime_error(s) {}
};

#undef RAPIDJSON_ASSERT
#define RAPIDJSON_ASSERT(x) do { \
if (!static_cast <bool>(x)) \
    throw std::runtime_error( "Rapidjson assertion failed: " \
                             + std::string(__func__ /*__PRETTY_FUNCTION__*/));\
} while(0)


#ifdef NDEBUG
  using jsonWr = rapidjson::Writer<rapidjson::StringBuffer>;
#else
  using jsonWr = rapidjson::PrettyWriter<rapidjson::StringBuffer>;
#endif

class Json
{
  public:
    Json() {
      m_logger->Register_Module(TM("CONFIG"), &m_module);
    }

    p7log_t* m_logger = P7_Get_Shared_Trace(TM("SharedLog")); // FIXME Temporary
    p7mod_t  m_module; // FIXME Temporary

    enum ERROR_CODES : int {
      PARSE_ERROR = -32700,
      INVALID_REQUEST = -32600,
      METHOD_NOT_FOUND = -32601,
      INVALID_PARAMS = -32602,
      INTERNAL_ERROR = -32603,
      DEFAULT_ERROR = -32000,
      PARAMS_NOT_FOUND = -32001,
      OUT_OF_RANGE = -32002,
      IS_NOT_PERMITTED = -32003,
      UNKNOWN_VALUE = -32004,
      I2C_READY = -32800,
      I2C_MISSING_ACK = -32801,
      I2C_RX_FIFO_COUNTER = -32802,
      I2C_TX_FIFO_COUNTER = -32803
    };

    // std::string ultohs(const ulong value, const ulong bitWidth){
      // u_int chars = (bitWidth - 1) / 4 + 1 ;
      // printf("chars = %d\n", chars);
      // char buffer [chars + 2];
      // printf("buffer = %lu\n", sizeof(buffer));
      // const std::string format  = "0x%0" + std::to_string(chars) + "zx";
      // printf("format = %s\n", format.c_str());
      // std::sprintf(&buffer[0], format.c_str(), value);
      // printf("sprintf\n");
      // return std::string(buffer, chars + 2);
    // }

    std::string ultohs(const ulong value){
      uint chars = (msBit(value)) / 4 + 1 ;
      char buffer [chars + 2];
      const std::string format  = "0x%0" + std::to_string(chars) + "zx";
      std::sprintf(&buffer[0], format.c_str(), value);

      return std::string(buffer, chars + 2);}

    bool hex2Json(jsonWr &wr, const char* key, const ulong value){
      return string2Json(wr, key, ultohs(value));
    }

    constexpr uint msBit(std::size_t x) {
      std::size_t t = 1;
      uint res = 0;
      std::size_t tmp = sizeof(std::size_t) * 8 / 2;

      while(tmp > 0) {
        if ( x >= t << tmp){
          t <<= tmp;
          res += tmp;
        }
        tmp = tmp>>1;
      }

      return res;
    }

    void errorResponse(jsonWr &wr, const int &code = DEFAULT_ERROR, const char* const key = nullptr) {
      if ( key )
        m_logger->P7_ERROR(m_module, TM("%s! %s for key: %s. Error code: [%d]."), error_codes_map[code].message.c_str(), error_codes_map[code].data.c_str(), key, code);
      else
        m_logger->P7_ERROR(m_module, TM("%s! %s. Error code: [%d]."), error_codes_map[code].message.c_str(), error_codes_map[code].data.c_str(), code);

      wr.Key("error");
      wr.StartObject();
      int2Json(wr, "code", code);
      string2Json(wr, "message", error_codes_map[code].message.c_str(), error_codes_map[code].message.size());
    #ifdef JSON_ERROR_RESPONSE_WITH_DATA
      string2Json(wr, "data", error_codes_map[code].data.c_str(), error_codes_map[code].data.size());
    #endif
      wr.EndObject();
    }

    static constexpr char hashUp(const char* const str, const size_t &size) {
      int tmp{0};
      char result = -1;
      for ( auto i = 0; i < size; i++ ){
        result &= str[size-1] ^ str[size-2];
        result ^= str[i] & str[size-i-1];
        result ^=str[size-i-1];
      }

      for ( auto i = 0; i < size; i++ )
        result = 255 - (int)result * str[i] - str[size - i - 1] * str[0];
      return result;
    }

    friend constexpr char operator ""_jh(const char* const literal, const size_t len);

    bool json2Bool(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        if ( value[key].IsBool() )
          return value[key].GetBool();
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, PARAMS_NOT_FOUND, key);
      return false;
    }

    bool bool2Json(jsonWr &wr, const char* key, const bool &value) {
      wr.Key(key);
      return wr.Bool(value);
    }

    static bool bool2Js(jsonWr &wr, const char* key, const bool &value) {
      wr.Key(key);
      return wr.Bool(value);
    }

    bool value2Json(jsonWr &wr, const char* key, const bool &value){
      wr.Key(key);
      return wr.Bool(value);
    };

    float json2Float(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        if ( value[key].IsFloat() )
          return value[key].GetFloat();
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, PARAMS_NOT_FOUND, key);
      return 0.;
    }

    double json2Double(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        if ( value[key].IsDouble() )
          return value[key].GetDouble();
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, PARAMS_NOT_FOUND, key);
      return 0.;
    }

    bool double2Json(jsonWr &wr, const char* key, const double &value) {
      wr.Key(key);
      return wr.Double(value);
    }

    bool value2Json(jsonWr &wr, const char* key, const double &value) {
      wr.Key(key);
      return wr.Double(value);
    }

    int json2Int(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        if ( value[key].IsInt() )
          return value[key].GetInt();
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, PARAMS_NOT_FOUND, key);
      return 0;
    }

    bool int2Json(jsonWr &wr, const char* key, const int &value) {
      wr.Key(key);
      return wr.Int(value);
    }

    bool value2Json(jsonWr &wr, const char* key, const int &value) {
      wr.Key(key);
      return wr.Int(value);
    }

    uint json2Uint(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        if ( value[key].IsUint() )
          return value[key].GetUint();
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, PARAMS_NOT_FOUND, key);
      return 0;
    }

    bool uint2Json(jsonWr &wr, const char* key, const uint &value) {
      wr.Key(key);
      return wr.Uint(value);
    }

    static bool uint2Js(jsonWr &wr, const char* key, const uint &value) {
      wr.Key(key);
      return wr.Uint(value);
    }

    bool value2Json(jsonWr &wr, const char* key, const uint &value) {
      wr.Key(key);
      return wr.Uint(value);
    }

    long json2Long(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        if ( value[key].IsInt64() )
          return value[key].GetInt64();
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, PARAMS_NOT_FOUND, key);
      return 0l;
    }

    bool long2Json(jsonWr &wr, const char* key, const long &value) {
      wr.Key(key);
      return wr.Int64(value);
    }

    bool value2Json(jsonWr &wr, const char* key, const long &value) {
      wr.Key(key);
      return wr.Int64(value);
    }

    ulong json2Ulong(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        #if defined(__aarch64__) || defined(__x86_64__)
        if ( value[key].IsUint64() )
          return value[key].GetUint64();
        #else
        if ( value[key].IsUint() )
          return value[key].GetUint();
        #endif
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, PARAMS_NOT_FOUND, key);
      return 0ul;
    }

    bool ulong2Json(jsonWr &wr, const char* key, const ulong &value) {
      wr.Key(key);
      return wr.Uint64(value);
    }

    bool value2Json(jsonWr &wr, const char* key, const ulong &value) {
      wr.Key(key);
      return wr.Uint64(value);
    }

    bool writeResponse(jsonWr &wr){
        this->string2Json(wr, "result", "ok");
        return 1;
    }

    static bool writeResp(jsonWr& wr){
      Json::str2Js(wr, "result", "ok");
      return 1;
    }
/*
    bool readResponse (jsonWr &wr, dsp::ip_control_t &ip_control){
        wr.Key("result");
        wr.StartObject();
        wr.Key("Status");
        wr.StartObject();
        //ip_control.serialize(json, request.GetObject()["params"].GetObject()["Control"], wr_response);
        this->bool2Json(wr, "DSPEnabled", ip_control.usr.dsp_enable);
        this->bool2Json(wr, "ProbeEnabled", ip_control.usr.probe_enable);
        this->bool2Json(wr, "PilotEnabled", ip_control.usr.pilot_enable);
        wr.EndObject();
        this->ulong2Json(wr, "OverviewMode", ip_control.usr.overview_mode);
        wr.EndObject();
    }
*/
    std::string json2String(jsonWr &wr, const char* key, const rapidjson::Value &value) {
      if ( value.HasMember(key) )
        if ( value[key].IsString() )
          return std::string(value[key].GetString(), value[key].GetStringLength());
        else
          errorResponse(wr, INVALID_PARAMS, key);
      else
        errorResponse(wr, INVALID_PARAMS, key);
      return "0";
    }

    bool string2Json(jsonWr &wr, const char* key, const std::string &value) {
      wr.Key(key);
      return wr.String(value.c_str(), value.size());
    }

    static void str2Js(jsonWr &wr, const char* key, const std::string &value){
      wr.Key(key);
      wr.String(value.c_str(), value.size());
    }

    bool value2Json(jsonWr &wr, const char* key, const std::string &value) {
      wr.Key(key);
      return wr.String(value.c_str(), value.size());
    }

    bool string2Json(jsonWr &wr, const char* key, const char *value, const size_t &size) {
      wr.Key(key);
      return wr.String(value, size);
    }

    bool parseRequestJsonRPC(const rapidjson::Value &jo, std::string &method, ulong &id, jsonWr &wr_response) {
      if ( jo.IsObject() ) {
        #if defined(__aarch64__) || defined(__x86_64__)
        if ( jo.GetObject().HasMember("id") && jo.GetObject()["id"].IsUint64() ) {
            id = jo.GetObject()["id"].GetUint64();
          #else
        if ( jo.GetObject().HasMember("id") && jo.GetObject()["id"].IsUint() ) {
            id = jo.GetObject()["id"].GetUint();
          #endif
          m_logger->P7_DEBUG(m_module, TM("ID: %lu."), id);
        } else
          m_logger->P7_DEBUG(m_module, TM("Json RPC message id field is missed."));

        if ( jo.GetObject().HasMember("jsonrpc")
          && jo.GetObject()["jsonrpc"].IsString()
          && jo.GetObject()["jsonrpc"] == "2.0"
          && jo.GetObject().HasMember("method")
          && jo.GetObject()["method"].IsString()
          && jo.GetObject().HasMember("params")
          && (jo.GetObject()["params"].IsObject() || jo.GetObject()["params"].IsArray())) {//
            method = jo.GetObject()["method"].GetString();
            m_logger->P7_TRACE(m_module, TM("Json RPC message with method '%s' was detected."), method.c_str());
            return true;
        } else {
          errorResponse(wr_response, INVALID_REQUEST);
          return false;
        }
      } else {
        printf("here\n");
        errorResponse(wr_response, INVALID_REQUEST);
        return false;
      }
    }

    void writeResponseJsonRPC(jsonWr &wr, const rapidjson::StringBuffer &response, const ulong &id) {
      wr.StartObject();
      wr.Key("jsonrpc", 7);
      wr.RawValue(response.GetString(), response.GetSize(), rapidjson::Type::kObjectType);
      wr.Key("id", 2); wr.Uint64(id);
      wr.EndObject();
    }

    // Fastest methods
    void writeResponseJsonRPC(jsonWr &wr) {
      wr.StartObject();
      wr.Key("jsonrpc", 7); wr.String("2.0", 3);
    }

    void writeResponseJsonRPC(jsonWr &wr, const ulong &id) {
      wr.Key("id", 2); wr.Uint64(id);
      wr.EndObject();
    }

    void writeRequestJsonRPC(jsonWr &wr, const rapidjson::StringBuffer &params, const ulong &id, const std::string &method) {
      wr.StartObject();
      wr.Key("jsonrpc", 7); wr.String("2.0", 3);
      wr.Key("method", 6); wr.String(method.c_str(), method.size());
      wr.Key("params", 6);
      wr.RawValue(params.GetString(), params.GetSize(), rapidjson::Type::kObjectType);
      wr.Key("id", 2); wr.Uint64(id);
      wr.EndObject();
    }

    // Fastest methods
    void writeRequestJsonRPC(jsonWr &wr, const std::string &method) {
      wr.StartObject();
      wr.Key("jsonrpc", 7); wr.String("2.0", 3);
      wr.Key("method", 6); wr.String(method.c_str(), method.size());
      wr.Key("params", 6);
    }

    void writeRequestJsonRPC(jsonWr &wr) {
      wr.EndObject();
    }

    void writeRequestJsonRPC(jsonWr &wr, const ulong &id) {
      wr.Key("id", 2); wr.Uint64(id);
      wr.EndObject();
    }

    const rapidjson::Value& desObj(const char* jk, const rapidjson::Value &jo)
    {
        if ( jo.HasMember(jk) )
            if ( jo[jk].IsObject() )
                return jo[jk];
            //else
                //parsingFailedMsg(jk);//add loging
        //else
           // missingValueMsg(jk);//add loging
        return jo;
    }

    void empytyArray2Json(jsonWr &wr, const char* name){
      wr.Key(name);
      wr.StartArray();
      wr.EndArray(0);
    };

    void array2Json(const std::vector<u_long>array, const unsigned long size, jsonWr &wr, const char* name){
      if(size == 0){
        wr.Key(name);
        wr.StartArray();
        wr.EndArray(size);
      }
      else{
        wr.Key(name);
        wr.StartArray();
        for(auto i = 0; i < size; ++i){
          wr.Uint(array.at(i));
        }
        wr.EndArray(size);
      }
    }

    void array2Json(const std::vector<u_long>array, const unsigned long size, jsonWr &wr, const char* name, bool string_type){
      std::vector<std::string>str_array;

      if(string_type){
        for(size_t i = 0; i < size; ++i){
          str_array.emplace_back(ultohs(array.at(i)));
        }
      }

      if(size == 0){
        wr.Key(name);
        wr.StartArray();
        wr.EndArray(size);
      }
      else{
        wr.Key(name);
        wr.StartArray();
        for(size_t i = 0; i < size; ++i){
          if(string_type)
            wr.String(str_array.at(i).c_str());
          else
            wr.Uint(array.at(i));
        }
        wr.EndArray(size);
      }

    };

    void array2Json(const std::vector<double>array, unsigned long size, jsonWr &wr, const char* name){
      if(size == 0){
        wr.Key(name);
        wr.StartArray();
        wr.EndArray(size);
      }
      else{
        wr.Key(name);
        wr.StartArray();
        for(auto i = 0; i < size; ++i){
          wr.Double(array.at(i));
        }
        wr.EndArray(size);
      }
    }

    bool value2Json(jsonWr &wr , const u_long &value) {
      return wr.Uint64(value);
    }

    /*
     bool hex2Json(jsonWr &wr, const char* key, const ulong value, const uint bitWidth = 32)
    {
      try {
        return Json::string2Json(wr, key, ultohs(value, bitWidth));
      }
      catch (std::exception&)
      {
        return false;
      }
    }

    ulong json2hex(const char* key, const rapidjson::Value &jo, u_char &res = _res)
    {
      try {
        return std::stoul(json2String(key, jo, res), 0, 16);
      }
      catch (std::invalid_argument&) {
        res |= parseStatus::parsingFailed;
      }
      return 0;
    }
    */
  private:
    // p7log_t* m_logger = P7_Get_Shared_Trace(TM("SharedLog"));
    // p7mod_t  m_module;

    typedef struct error_t {
      const std::string message;
      const std::string data;
    } error_t;

    std::map<const int, const error_t> error_codes_map {
      {PARSE_ERROR, {"Parse error", "Invalid JSON was received by the server"}},
      {INVALID_REQUEST, {"Invalid Request", "The JSON sent is not a valid Request object"}},
      {METHOD_NOT_FOUND, {"Method not found", "The method does not exist or is not available"}},
      {INVALID_PARAMS, {"Invalid params", "Invalid method parameters"}},
      {INTERNAL_ERROR, {"Internal error", "Internal JSON-RPC error"}},
      {DEFAULT_ERROR, {"Default error", "This error occurs when an error code is not specified"}},
      {PARAMS_NOT_FOUND, {"Params not found", "The parameter does not exist"}},
      {OUT_OF_RANGE, {"Out of range", "The value is out of range"}},
      {IS_NOT_PERMITTED, {"Is not permitted", "Operation is not permitted"}},
      {UNKNOWN_VALUE, {"Unknown value", "The parameter value is not known"}},
    };

};

constexpr char operator ""_jh(const char* const literal, const size_t len){
  return Json::hashUp(literal, len);
}

#endif // JSON_H
