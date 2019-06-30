#ifndef THREAD
#define THREAD

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "settings.cpp"
#include "constants.h"

class Worker {
public:
    virtual void work() {}

    ~Worker() {
        std::cout << "Worker " << this << " deleted\n";
    }
};

class WorkerThread : public wxThread {
public:
    bool isRunning = true;
    bool isWorkDone = true;
    Worker **workers;

    WorkerThread() : wxThread() {
        workers = new Worker*[NVSM_WORKERS_MAX];
        for (uint i = 0; i < NVSM_WORKERS_MAX; i++) workers[i] = nullptr;
    }

    virtual ExitCode Entry() {
        isWorkDone = false;

        while (isRunning) {
            for (uint i = 0; i < NVSM_WORKERS_MAX; i++) {
                if (workers[i] == nullptr) continue;
                workers[i]->work();
            }

            usleep(UPDATE_DELAY_USEC);
        }

        std::cout << "WorkerThread done all work!\n";
        isWorkDone = true;

        return 0;
    }

    ~WorkerThread() {
        delete[] workers;
        
        std::cout << "WorkerThread " << this << " deleted\n";
    }
};

#endif