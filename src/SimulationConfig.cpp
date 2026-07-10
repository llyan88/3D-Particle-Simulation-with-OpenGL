#include "SimulationConfig.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// =============================================================================
// SimulationConfig.cpp  –  ECE 4122/6122 Final Project  (STUDENT SKELETON)
// =============================================================================
// SimulationConfig::load() parses a simple INI file.
//
// The file format uses [section] headings and  key = value  pairs.
// Lines starting with ';' or '#' are comments.
//
// You must implement:
//   1. The trim() helper – strips leading/trailing whitespace from a string.
//   2. SimulationConfig::load() – reads the file and populates the struct.
//
// The struct and its default values are already defined in SimulationConfig.h.
// If the file cannot be opened, simply print a notice and return (defaults stay).
// =============================================================================


// ---------------------------------------------------------------------------
// trim  –  return a copy of s with leading and trailing whitespace removed.
//          Whitespace characters: ' ', '\t', '\r', '\n'
// ---------------------------------------------------------------------------
static std::string trim(const std::string& s)
{
    // TODO: find first non-whitespace character (std::string::find_first_not_of)
    std::size_t first = s.find_first_not_of(" \t\r\n");

    // TODO: find last  non-whitespace character (std::string::find_last_not_of)
    if (first == std::string::npos)
        return "";

    std::size_t last = s.find_last_not_of(" \t\r\n");

    // TODO: return the substring between them, or "" if all whitespace
    return s.substr(first, last - first + 1);
}


// ---------------------------------------------------------------------------
// SimulationConfig::load
//   Open the file at path.  If it cannot be opened, print a notice and return.
//
//   Parse line-by-line:
//     • Skip empty lines and comment lines (first non-space char is ';' or '#')
//     • If line is "[section]", store the section name (not needed for key lookup)
//     • Otherwise find '=' and split into key / value (both trimmed)
//     • Match known keys and parse their values with std::stoi / std::stof / =val
//
//   Supported keys (all listed in SimulationConfig.h):
//     width, height, title
//     max_particles, spawn_rate, num_threads
//     gravity_x, gravity_y, gravity_z
//     restitution
//     emitter_type, emitter_x, emitter_y, emitter_z
//     bbox_min_x, bbox_min_y, bbox_min_z
//     bbox_max_x, bbox_max_y, bbox_max_z
//
//   Wrap the parsing in try/catch(...) and print a warning if a value fails.
//   Print "[Config] Loaded from '<path>'." on success.
// ---------------------------------------------------------------------------
void SimulationConfig::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[Config] '" << path << "' not found - using defaults.\n";
        return;
    }

    std::string line, section;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        // TODO: detect [section] lines and save the section name
        if (line.front() == '[' && line.back() == ']') {
            section = trim(line.substr(1, line.size() - 2));
            continue;
        }

        // TODO: find '=' separator; skip lines without one
        std::size_t eq = line.find('=');
        if (eq == std::string::npos)
            continue;

        // TODO: split into key = trim(left of '='), val = trim(right of '=')
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        // TODO: match key against each supported name and parse the value
        //       Use std::stoi for ints, std::stof for floats, direct assign for strings
        //       Wrap in try { … } catch (...) { print warning }
        try {
            if (key == "width") {
                windowWidth = std::stoi(val);
            } else if (key == "height") {
                windowHeight = std::stoi(val);
            } else if (key == "title") {
                windowTitle = val;
            } else if (key == "max_particles") {
                maxParticles = std::stoi(val);
            } else if (key == "spawn_rate") {
                spawnRate = std::stof(val);
            } else if (key == "num_threads") {
                numThreads = std::stoi(val);
            } else if (key == "gravity_x") {
                gravity.x = std::stof(val);
            } else if (key == "gravity_y") {
                gravity.y = std::stof(val);
            } else if (key == "gravity_z") {
                gravity.z = std::stof(val);
            } else if (key == "restitution") {
                restitution = std::stof(val);
            } else if (key == "emitter_type") {
                emitterType = std::stoi(val);
            } else if (key == "emitter_x") {
                emitterPos.x = std::stof(val);
            } else if (key == "emitter_y") {
                emitterPos.y = std::stof(val);
            } else if (key == "emitter_z") {
                emitterPos.z = std::stof(val);
            } else if (key == "bbox_min_x") {
                bboxMin.x = std::stof(val);
            } else if (key == "bbox_min_y") {
                bboxMin.y = std::stof(val);
            } else if (key == "bbox_min_z") {
                bboxMin.z = std::stof(val);
            } else if (key == "bbox_max_x") {
                bboxMax.x = std::stof(val);
            } else if (key == "bbox_max_y") {
                bboxMax.y = std::stof(val);
            } else if (key == "bbox_max_z") {
                bboxMax.z = std::stof(val);
            }
        } catch (...) {
            std::cerr << "[Config] Warning: could not parse key '"
                      << key << "' with value '" << val << "'.\n";
        }
    }

    std::cout << "[Config] Loaded from '" << path << "'.\n";
}