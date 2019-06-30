// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/html/htmlwin.h>
#include <wx/notebook.h>
#include <thread>

#include "process_list.cpp"
#include "core.cpp"
#include "thread.cpp"
#include "ihtmlwin.cpp"
#include "graph.cpp"
#include "utilization.cpp"

#define ID_MAIN_NOTEBOOK 0

WorkerThread *workerThread;

DECLARE_EVENT_TYPE(EVT_CLEANUP_DONE, wxID_ANY)
DEFINE_EVENT_TYPE(EVT_CLEANUP_DONE)

class KillerThread : public wxThread {
public:
    virtual ExitCode Entry();

    ~KillerThread() {
        std::cout << "KillerThread " << this << " deleted\n";
    }
};

KillerThread *killerThread;

class NVSMApp: public wxApp {
public:
    virtual bool OnInit();

    ~NVSMApp() {
        std::cout << "NVSMApp " << this << " deleted\n";
    }
};

class NVSMFrame: public wxFrame {
public:
    NVSMFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
private:
    void close() {
        Hide();

        killerThread = new KillerThread();
        killerThread->Run();
    }

    void onHelp(wxCommandEvent& event);
    void onExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void onCleanupDone(wxCommandEvent &event) {
        DestroyChildren();
        std::cout << "Cleanup done. Finishing.\n";
        Destroy();
    }

    void onClose(wxCloseEvent &event) {
        close();
    }

    ~NVSMFrame() {
        std::cout << "NVSMFrame " << this << " deleted\n";
    }

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_HELP = 0,
};

wxBEGIN_EVENT_TABLE(NVSMFrame, wxFrame)
    EVT_MENU(ID_HELP,   NVSMFrame::onHelp)
    EVT_MENU(wxID_EXIT,  NVSMFrame::onExit)
    EVT_MENU(wxID_ABOUT, NVSMFrame::OnAbout)
    EVT_CLOSE(NVSMFrame::onClose)
    EVT_COMMAND(wxID_ANY, EVT_CLEANUP_DONE, NVSMFrame::onCleanupDone)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(NVSMApp);

NVSMFrame *frame;

class MainNotebook : public wxNotebook {
private:
    void addWorker(Worker *worker) {
        uint slot;

        for (uint i = 0; i < NVSM_WORKERS_MAX; i++) {
            if (workerThread->workers[i] == nullptr) slot = i;
            if (workerThread->workers[i] == worker) return;
        }

        std::cout << "Worker " << worker << " not added yet, adding at slot " << slot << "\n";
        workerThread->workers[slot] = worker;
    }
public:
    wProcessList *processes;
    UPLWorker *uplWorker;
    WaveGraph *gpuUtilization;
    WaveGraph *memUtilization;
    GPUUWorker *gpuUtilizationWorker;
    GPUUPainter *gpuUtilizationPainter;
    MemUWorker *memUtilizationWorker;
    MemUPainter *memUtilizationPainter;

    MainNotebook(wxWindow *parent) : wxNotebook(parent, ID_MAIN_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE) {
        GPU_COUNT = atoi(split(exec(NVSMI_CMD_GPU_COUNT), "\n")[1].c_str());
        // GPU_COUNT = 4; // for displaying fake GPUs
        loadSettings();

        processes = new wProcessList(this, (wxFrame*)parent);
        uplWorker = new UPLWorker(processes);
        gpuUtilization = new WaveGraph(this);
        gpuUtilizationWorker = new GPUUWorker(gpuUtilization);
        gpuUtilizationPainter = new GPUUPainter((wxFrame*)parent);
        memUtilization = new WaveGraph(this);
        memUtilizationWorker = new MemUWorker(memUtilization);
        memUtilizationPainter = new MemUPainter((wxFrame*)parent);

        gpuUtilization->p = gpuUtilizationPainter;
        gpuUtilizationWorker->p = gpuUtilizationPainter;
        memUtilization->p = memUtilizationPainter;
        memUtilizationWorker->p = memUtilizationPainter;

        AddPage(processes, "Processes");
        AddPage(gpuUtilization, "GPU Utilization");
        AddPage(memUtilization, "Memory Utilization");
        Layout();
    }

    void onPageChanged(wxBookCtrlEvent &event) {
        switch (event.GetSelection()) {
            case MNP_PROCESSES:
                addWorker(uplWorker);
                break;
            case MNP_GPU_UTILIZATION:
                addWorker(gpuUtilizationWorker);
                break;
            case MNP_MEM_UTILIZATION:
                addWorker(memUtilizationWorker);
                break;
        }
    }

    ~MainNotebook() {
        delete uplWorker;
        delete gpuUtilizationPainter;
        delete gpuUtilizationWorker;
        delete memUtilizationPainter;
        delete memUtilizationWorker;

        std::cout << "MainNotebook " << this << " deleted\n";
    }

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MainNotebook, wxNotebook)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_MAIN_NOTEBOOK, MainNotebook::onPageChanged)
wxEND_EVENT_TABLE()

MainNotebook *mn;

bool NVSMApp::OnInit() {
    std::cout << "Connecting to nvidia-smi...\n";
    if (system("which nvidia-smi > /dev/null 2>&1")) {
        std::cout << "nvidia-smi not found. Are you have NVIDIA drivers?\n";
        wxMessageBox("nvidia-smi not found. Are you have NVIDIA drivers?", "Error");
        return false;
    } else {
        std::string nvsmi_out = exec("nvidia-smi");
        if (startsWith(split(nvsmi_out, "\n"), "NVIDIA-SMI has failed") != std::string::npos) {
            std::cout << "nvidia-smi was found, but " << nvsmi_out;
            wxMessageBox(nvsmi_out + "If you using laptop with discrete NVIDIA GPU, launch this app with optirun", "Error");
            return false;
        }
    }

    std::cout << "Done\n";

    frame = new NVSMFrame("NVIDIA System Monitor", wxPoint(64, 64), wxSize(512, 512));
    frame->Show(true);
    return true;
}

NVSMFrame::NVSMFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size) {
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(ID_HELP, "Help\tCtrl-H", "Show help");
    menuHelp->AppendSeparator();
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "File");
    menuBar->Append(menuHelp, "Help");
    SetMenuBar(menuBar);
    CreateStatusBar();

    wxInitAllImageHandlers();

    workerThread = new WorkerThread();
    mn = new MainNotebook(this);
    workerThread->Run();
}

void NVSMFrame::onExit(wxCommandEvent& event) {
    close();
}

void NVSMFrame::OnAbout(wxCommandEvent& event) {
    showInfoHtmlWindow(this, "About", NVSM_PATH_ABOUT_HTML);
}

void NVSMFrame::onHelp(wxCommandEvent& event) {
    showInfoHtmlWindow(this, "Help", NVSM_PATH_HELP_HTML);
}

wxThread::ExitCode KillerThread::Entry() {
    workerThread->isRunning = false;
    while (!workerThread->isWorkDone); // we wait while WorkerThread did the work

    // in order to output of WorkerThread destruction is not overlayed with KillerThread output (or others)
    usleep(100000);

    frame->GetEventHandler()->AddPendingEvent(wxCommandEvent(EVT_CLEANUP_DONE, wxID_ANY));
    return 0;
}
