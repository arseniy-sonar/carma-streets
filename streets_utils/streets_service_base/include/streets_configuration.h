#pragma once

#include "streets_singleton.h"
#include "streets_configuration_exception.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h" //support for async logging.
#include "spdlog/sinks/daily_file_sink.h" // support for dailty file sink
#include "spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no colors needed
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <iosfwd>
#include <sstream>
#include <map>
#include <iostream>
#include <iterator>
#include <boost/lexical_cast.hpp>


/**
 * @enum Data type enum for configurations.
 */ 
enum class data_type { config_string=0, config_int=1, config_double=2, config_bool=3 }; 

/**
 * @struct Configuration struct to store configuration parameter name, value, 
 * description and data type. Also contains methods to deserialize from json
 * and operators for equivalency.
 */
struct configuration {
    /**
     * Name of configuration
     */
    std::string name;
    /**
     * Value of configuration
     */ 
    std::string value;
    /**
     * Description of configuration
     */ 
    std::string description;
    /**
     * Data type enum for configuration
     */
    data_type type;

    /**
     * @brief operator== checks if two instances are equal.
     * @param t configuration comparison.
     */ 
    bool operator==(const configuration& t) const {
        return name == t.name && value == t.value && description == t.description && type == t.type;
    }

    /**
     * @brief operator!= checks if two instances are not equal.
     * @param t configuration comparison.
     */ 
    bool operator!=(const configuration& t) const {
        return !(name == t.name && value == t.value && description == t.description && type == t.type);
    }
    /**
     * @brief Method to translate json object to configuration struct.
     * @param val rapidjson::Value object representing configuration.
     */ 
    void fromJson(const rapidjson::Value &val ){
        if ( val.HasMember("name") ) {
            name = val.FindMember("name")->value.GetString();
        }else {
            throw streets_configuration_exception("Configuration is missing name!");
        }

        if ( val.HasMember("description") ){
            description = val.FindMember("description")->value.GetString();
        } else {
            throw streets_configuration_exception("Configuration is" + name + " missing a description");
        }

        if ( val.HasMember("type") ){
            std::string doc_type = val.FindMember("type")->value.GetString();
            if ( doc_type == "STRING" ) {
                type = data_type::config_string;
                if ( val.HasMember("value") && val.FindMember("value")->value.IsString() ) {
                    value = val.FindMember("value")->value.GetString();
                }
                else {
                    throw streets_configuration_exception("Configuration is missing/incorrect a value");
                }
            }
            else if (doc_type == "INTEGER"){
                type = data_type::config_int;
                if ( val.HasMember("value") && val.FindMember("value")->value.IsInt() ) {
                    value = std::to_string(val.FindMember("value")->value.GetInt());
                }
                else {
                    throw streets_configuration_exception("Configuration is missing/incorrect a value");
                }
            }
            else if (doc_type == "DOUBLE"){
                type = data_type::config_double;
                if ( val.HasMember("value") && val.FindMember("value")->value.IsDouble() ) {
                    value = std::to_string(val.FindMember("value")->value.GetDouble());
                }
                else {
                    throw streets_configuration_exception("Configuration is missing/incorrect a value");
                }
            }
            else if (doc_type == "BOOL") {
                type = data_type::config_bool;
                if ( val.HasMember("value") && val.FindMember("value")->value.IsBool() ) {
                    value = val.FindMember("value")->value.GetBool() ? "true" : "false";
                }
                else {
                    throw streets_configuration_exception("Configuration is missing/incorrect a value");
                }
            } else {
                throw streets_configuration_exception("Data type configuration is invalid!");
            }
        } else {
            throw streets_configuration_exception("Configuration " + name + " is missing a type");
        }
        
        spdlog::debug("Create config name : {0}, value : {1}, type : {2}, description : {3} ",name, value, type, description );
    
    }
};
class streets_configuration : public streets_singleton<streets_configuration> {
    friend class streets_singleton<streets_configuration>;

    private:
        /* String filepath to manifest.json configuration file */
        std::string filepath;
        /* mutex lock for streets_configuration*/
        std::mutex config_lock;
        /* Map of configuration names and values*/
        std::map< std::string, configuration > configuration_map;
        /* Flag to indicate whether streets_configuration singleton has been initialized */
        bool initialized = false;
        /**
         * @brief Constructor that takes filepath as a parameter. 
         * @param filepath relative path to manifest.json configuration file.
         */
        streets_configuration(const std::string &filepath = "../manifest.json");


    public:
        /**
         * @brief Retreive configuration parameter string value with provided name.
         * @param config_param_name configuration parameter name.
         */ 
        static std::string get_string_config( const std::string &config_param_name);
        /**
         * @brief Retreive configuration parameter integer value with provided name.
         * @param config_param_name configuration parameter name.
         */ 
        static int get_int_config( const std::string &config_param_name);
        /**
         * @brief Retreive configuration parameter double value with provide name.
         * @param config_param_name configuration parameter name.
         */ 
        static double get_double_config( const std::string &config_param_name);
        /**
         * @brief Retreive configuration parameter boolean value with provide name.
         * @param config_param_name configuration parameter name.
         */ 
        static bool get_boolean_config( const std::string &config_param_name);
    
    protected:

        // // Hide inherited get_singleton method access
        // using streets_singleton::get_singleton;
        /**
         * @brief Parse manifest.json configuration file for CARMA-Streets service and update configurations.
         */ 
        rapidjson::Document parse_configuration_file();
        /**
         * @brief Update configuration parameters using json Document.
         */ 
        void update_configuration(const rapidjson::Document &doc);
        /**
         * @brief
         */
        void initialize(const rapidjson::Document &doc); 
        /**
         * 
         */
        void configure_logger( const std::string &service_name); 
        /**
         * 
         */
        void set_loglevel( const std::string &loglevel); 

        using streets_singleton::get_singleton;

};



