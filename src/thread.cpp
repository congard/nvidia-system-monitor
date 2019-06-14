#ifndef THREAD
#define THREAD

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "settings.cpp"

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
    Worker *worker;

    virtual ExitCode Entry() {
        isWorkDone = false;

        while (isRunning) {
            if (worker == nullptr) continue;
            worker->work();
            usleep(UPDATE_DELAY_USEC);
        }

        std::cout << "WorkerThread done all work!\n";
        isWorkDone = true;

        return 0;
    }

    ~WorkerThread() {
        std::cout << "WorkerThread " << this << " deleted\n";
    }
};

#endif