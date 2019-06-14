#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/listctrl.h>
#include <wx/notebook.h>

#include "core.cpp"
#include "settings.cpp"
#include "constants.h"
#include "thread.cpp"

#include <vector>

// the ID we'll use to identify our event
#define UPL_ID 0

struct ProcessList {
    std::string name;
    std::string type; // C or G
    std::string gpuIdx, pid, sm, mem, enc, dec; // integers
};

std::vector<ProcessList> processes;

enum {
    ID_PROCESS_LIST_VIEW,
    ID_MENU_ITEM_KILL
};

class ProcessListView : public wxListView {
public:
    long selected = -1;

    ProcessListView(wxPanel *panel) {
        Create(panel, ID_PROCESS_LIST_VIEW, wxDefaultPosition, wxSize(), wxLC_REPORT);

        AppendColumn("Name");
        AppendColumn("Type (C/G)");
        AppendColumn("GPU ID");
        AppendColumn("pid");
        AppendColumn("sm");
        AppendColumn("mem");
        AppendColumn("enc");
        AppendColumn("dec");
    }

    void rightClick(wxListEvent &event) {
        if (event.GetIndex() == -1) return;

        wxMenu menu;
        menu.Append(ID_MENU_ITEM_KILL, "Kill " + processes[event.GetIndex()].name + " (pid " + processes[event.GetIndex()].pid + ")");
        PopupMenu(&menu, event.GetPoint());
    }

    void menuItemClicked(wxCommandEvent &event) {
        // only one menu item - kill
        exec("kill " + processes[selected].pid);
        selected = -1;
    }

    void menuItemSelected(wxListEvent &event) {
        selected = event.GetIndex();
    }

    void menuItemDeselected(wxListEvent &event) {
        selected = -1;
    }

    ~ProcessListView() {
        std::cout << "ProcessListView " << this << " deleted\n";
    }

    DECLARE_EVENT_TABLE()
};

wxBEGIN_EVENT_TABLE(ProcessListView, wxListView)
    EVT_MENU(ID_MENU_ITEM_KILL, ProcessListView::menuItemClicked)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_PROCESS_LIST_VIEW, ProcessListView::rightClick)
    EVT_LIST_ITEM_SELECTED(ID_PROCESS_LIST_VIEW, ProcessListView::menuItemSelected)
    EVT_LIST_ITEM_DESELECTED(ID_PROCESS_LIST_VIEW, ProcessListView::menuItemDeselected)
wxEND_EVENT_TABLE()

// a class that will periodically send events to the GUI thread
// UPL - update process list
class UPLWorker : public Worker {
    wxPanel *panel;
    std::vector<std::string> lines, data;
    
public:
    UPLWorker(wxPanel *panel) {
        this->panel = panel;
    }

    virtual void work() {
        lines = split(streamline(exec(NVSMI_CMD_PROCESSES)), "\n");
        processes.clear();

        for (size_t i = 2; i < lines.size(); i++) {
            if (lines[i] == "") continue;
                
            data = split(lines[i], " ");
                
            ProcessList pl;
            pl.name = data[NVSMI_NAME];
            pl.type = data[NVSMI_TYPE];
            pl.gpuIdx = data[NVSMI_GPUIDX];
            pl.pid = data[NVSMI_PID];
            pl.sm = data[NVSMI_SM];
            pl.mem = data[NVSMI_MEM];
            pl.enc = data[NVSMI_ENC];
            pl.dec = data[NVSMI_DEC];

            processes.push_back(pl);
        }

        panel->GetEventHandler()->AddPendingEvent(wxCommandEvent(wxEVT_COMMAND_TEXT_UPDATED, UPL_ID));
    }
};

// w means widget
class wProcessList : public wxPanel {
    wxNotebook *parent;
    wxFrame *frame;
    
public:
    ProcessListView *listView;

    wProcessList(wxNotebook *parent, wxFrame *frame) {
        this->parent = parent;
        this->frame = frame;

        Create(parent);
        
        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
        listView = new ProcessListView(this);
        vbox->Add(listView, 1, wxEXPAND);

        SetSizer(vbox);
    }

    void onDataUpdate(wxCommandEvent &evt) {
        listView->DeleteAllItems();
        for (size_t i = 0; i < processes.size(); i++) {
            ProcessList &pl = processes[i];
            listView->InsertItem(i, pl.name);
            listView->SetItem(i, NVSM_TYPE, pl.type);
            listView->SetItem(i, NVSM_GPUIDX, pl.gpuIdx);
            listView->SetItem(i, NVSM_PID, pl.pid);
            listView->SetItem(i, NVSM_SM, pl.mem);
            listView->SetItem(i, NVSM_MEM, pl.mem);
            listView->SetItem(i, NVSM_ENC, pl.enc);
            listView->SetItem(i, NVSM_DEC, pl.dec);
        }
        if (listView->selected != -1) listView->Select(listView->selected);

        frame->SetStatusText(std::to_string(processes.size()) + " processes running");
    }

    ~wProcessList() {
        std::cout << "wProcessList " << this << " deleted\n";
    }

    DECLARE_EVENT_TABLE()
};

// catch the event from the thread
BEGIN_EVENT_TABLE(wProcessList, wxPanel)
    EVT_COMMAND(UPL_ID, wxEVT_COMMAND_TEXT_UPDATED, wProcessList::onDataUpdate)
END_EVENT_TABLE()