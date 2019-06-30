#ifndef CONSTANTS
#define CONSTANTS

#ifdef __INSTALL_BUILD__
#define NVSM_PATH_ABOUT_HTML "/usr/share/doc/nvidia-system-monitor/html/about.html"
#define NVSM_PATH_HELP_HTML "/usr/share/doc/nvidia-system-monitor/html/help.html"
#else
#define NVSM_PATH_ABOUT_HTML "src/html/about.html"
#define NVSM_PATH_HELP_HTML "src/html/help.html"
#endif

#define NVSM_CONF_UPDATE_DELAY "updateDelay"
#define NVSM_CONF_GRAPH_LENGTH "graphLength"
#define NVSM_CONF_GCOLOR "gpuColor"

#define NVSMI_CMD_GPU_COUNT "nvidia-smi --query-gpu=count --format=csv"
#define NVSMI_CMD_PROCESSES "nvidia-smi pmon -c 1"
#define NVSMI_CMD_GPU_UTILIZATION "nvidia-smi --query-gpu=utilization.gpu --format=csv"
#define NVSMI_CMD_MEM_UTILIZATION "nvidia-smi --query-gpu=utilization.memory,memory.total,memory.free,memory.used --format=csv"

// nvidia-smi command output indices
#define NVSMI_GPUIDX 0
#define NVSMI_PID    1
#define NVSMI_TYPE   2
#define NVSMI_SM     3
#define NVSMI_MEM    4
#define NVSMI_ENC    5
#define NVSMI_DEC    6
#define NVSMI_NAME   7

// nvidia-system-monitor processes columns
#define NVSM_GPUIDX 2
#define NVSM_PID    3
#define NVSM_TYPE   1
#define NVSM_SM     4
#define NVSM_MEM    5
#define NVSM_ENC    6
#define NVSM_DEC    7
#define NVSM_NAME   0

// main notebook pages
#define MNP_PROCESSES       0
#define MNP_GPU_UTILIZATION 1
#define MNP_MEM_UTILIZATION 2

#define TEXT_BLOCK_OFFSET 16
#define LINE_OFFSET       8

#define NVSM_WORKERS_MAX 3

typedef unsigned int uint;

#endif