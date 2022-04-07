/*------------------------------------------------------------------------------
--  This file is a part of the C++ Config library
--  Copyright (C) 2022, Plasma Physics Laboratory - CNRS
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
-------------------------------------------------------------------------------*/
/*--                  Author : Alexis Jeandet
--                     Mail : alexis.jeandet@lpp.polytechnique.fr
----------------------------------------------------------------------------*/
#pragma once
#include <dict.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace cppconfig::config_json
{
using json = nlohmann::json;
using json_t = decltype(json::parse(""));

template <typename config_t>
inline void _load_json_object(const json_t& obj, config_t& cfg);

template <typename config_t>
inline void _build_json_object(const config_t& cfg, json_t& obj);

template <typename config_t>
inline void _load_config(const json_t& node, config_t& cfg)
{
    switch (node.type())
    {
        case json::value_t::string:
            cfg = std::string { node };
            break;
        case json::value_t::boolean:
            cfg = bool { node };
            break;
        case json::value_t::number_integer:
        case json::value_t::number_unsigned:
            cfg = int { node };
            break;
        case json::value_t::number_float:
            cfg = double { node };
            break;
        case json::value_t::object:
            _load_json_object(node, cfg);
            break;
        default:
            break;
    }
}

template <typename config_t>
inline void _load_json_object(const json_t& obj, config_t& cfg)
{
    for (auto& [key, value] : obj.items())
    {
        _load_config(value, cfg[key]);
    }
}

template <typename node_t>
inline void _build_json_object_from_node(const node_t& cfg, json_t& obj)
{
    for (const auto& [key, node] : cfg)
    {
        _build_json_object(*node, obj[key]);
    }
}

template <typename config_t>
inline void _build_json_object(const config_t& cfg, json_t& obj)
{
    if (cfg.isNode())
    {
        cfg.visit(
            cppdict::visit_all_nodes,
            [&obj](const std::string& key, const typename config_t::node_t& v)
            { _build_json_object_from_node(v, obj[key]); },
            [&obj](const std::string&, const typename config_t::empty_leaf_t&) {},
            [&obj](const std::string& key, const auto& v) { obj[key] = v; });
    }
    else
    {
        if(cfg.isValue())
        {
            if(std::holds_alternative<bool>(cfg.data))
            {
                obj = std::get<bool>(cfg.data);
                return;
            }
            if(std::holds_alternative<int>(cfg.data))
            {
                obj = std::get<int>(cfg.data);
                return;
            }
            if(std::holds_alternative<double>(cfg.data))
            {
                obj = std::get<double>(cfg.data);
                return;
            }
            if(std::holds_alternative<std::string>(cfg.data))
            {
                obj = std::get<std::string>(cfg.data);
                return;
            }
        }
    }
}

template <typename config_t>
inline config_t load_config(const json_t& json_doc)
{
    config_t cfg;
    _load_json_object(json_doc, cfg);
    return cfg;
}

template <typename config_t>
inline json_t build_json(const config_t& cfg)
{
    json_t json_doc;
    _build_json_object(cfg, json_doc);
    return json_doc;
}

template <typename config_t>
inline config_t load_config(const std::string& json)
{
    return load_config<config_t>(json::parse(json));
}

template <typename config_t>
inline config_t load_config(const std::filesystem::path& json_file)
{
    return load_config<config_t>(json::parse(std::ifstream { json_file.string() }));
}


template <typename config_t>
inline std::string save_config(const config_t& cfg)
{
    return build_json(cfg).dump();
}

template <typename config_t>
inline bool save_config(const std::filesystem::path& json_file, const config_t& cfg)
{
    std::ofstream o(json_file);
    o << std::setw(4) << build_json(cfg) << std::endl;
    return true;
}


}
