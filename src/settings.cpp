#ifndef SETTINGS
#define SETTINGS

#include <wx/colour.h>
#include <pwd.h>
#include <iostream>
#include <vector>

#include "core.cpp"
#include "constants.h"

static uint UPDATE_DELAY = 2000; // 2 sec
static uint GRAPH_LENGTH = 60000; // 60 sec

static int GPU_COUNT = -1;

#define c(r, g, b) wxColor(r, g, b)

static wxColor gcolors[8] = {
    c(32, 140, 32),
    c(128, 0, 0),
    c(0, 0, 128),
    c(128, 0, 128),
    c(210, 105, 30),
    c(153, 51, 255),
    c(122, 163, 31),
    c(22, 122, 122)
};

#define UPDATE_DELAY_USEC (UPDATE_DELAY * 1000)
#define GRAPH_STEP ((float)UPDATE_DELAY / (float)GRAPH_LENGTH)

static void loadSettings() {
    std::cout << "Loading settings\n";

    std::string path = getpwuid(getuid())->pw_dir;
    path += "/.config/nvidia-system-monitor/config";

    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        std::cout << "No config file found. Using default settings.\n";
        return;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *conf_tmp = new char[size + 1];
    fseek(file, 0, SEEK_SET);
    fread(conf_tmp, 1, size, file);
    fclose(file);
    conf_tmp[size] = '\0';
    std::string conf = conf_tmp;
    delete[] conf_tmp;

    std::vector<std::string> lines = split(conf, "\n"), line;

    int lid; // line id
    if ((lid = startsWith(lines, NVSM_CONF_UPDATE_DELAY)) != std::string::npos) {
        UPDATE_DELAY = atoi(split(streamline(lines[lid]), " ")[1].c_str());
        lines.erase(lines.begin() + lid);
    }
    if ((lid = startsWith(lines, NVSM_CONF_GRAPH_LENGTH)) != std::string::npos) {
        GRAPH_LENGTH = atoi(split(streamline(lines[lid]), " ")[1].c_str());
        lines.erase(lines.begin() + lid);
    }

    lid = 0;
    while (lid != std::string::npos) {
        if ((lid = startsWith(lines, NVSM_CONF_GCOLOR)) != std::string::npos) {
            std::vector<std::string> line = split(streamline(lines[lid]), " ");
            gcolors[atoi(line[1].c_str())] = c(atoi(line[2].c_str()), atoi(line[3].c_str()), atoi(line[4].c_str()));
            lines.erase(lines.begin() + lid);
        }
    }

    std::cout << "Done\n";
}

#undef c
#endif