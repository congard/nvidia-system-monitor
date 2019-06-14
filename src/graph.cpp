#ifndef GRAPH
#define GRAPH

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <vector>

#include "settings.cpp"

#define y(n) (y0 + n)
#define x(n) (x0 + n)

#define WAVE_GRAPH_UPDATED_ID 0

struct Point {
    float x; // [0; 1]
    int y; // [0; 100]

    Point(const float &x, const int &y) {
        this->x = x;
        this->y = y;
    }
};

// gpu utilization data
struct GPUUData {
    int level = 0; // current graph point in percents
    int avgLevel = 0, minLevel = 0, maxLevel = 0;
};

class Painter {
protected:
    wxFrame *frame;
public:
    uint width, height;
    uint x0, y0;
    GPUUData *gdata;

    Painter(wxFrame *frame) {
        this->frame = frame;
        gdata = new GPUUData[GPU_COUNT];

        reset();
    }

    virtual void paint(wxDC &dc) {}
    virtual void reset() {
        for (size_t i = 0; i < GPU_COUNT; i++) gdata[i] = GPUUData();
    }

    ~Painter() {
        delete[] gdata;
        std::cout << "Painter " << this << " deleted\n";
    }
};

class WaveGraphPainter : public Painter {
public:
    std::vector<Point> *glines; // graph lines

    WaveGraphPainter(wxFrame *frame) : Painter(frame) {
        glines = new std::vector<Point>[GPU_COUNT];

        // cant call reset. It will call Painter::reset!
        for (size_t i = 0; i < GPU_COUNT; i++) glines[i] = std::vector<Point>();
    }

    void deleteSuperfluousPoints(const size_t &index) {
        // We can't use this approach since it does not take into account step.
        // It will split graph into equal sections what we haven't
        //
        // to build a graph we need `the number of lines + 1` points
        // and if 1 point is superfluous, we delete it
        // so we have +2
        // if (glines.size() >= GRAPH_LENGTH / UPDATE_DELAY + 2) {
        //     glines.erase(glines.begin());
        //     std::cout << glines.size() << "\n";
        // }
        //
        // This approach takes into account step
        if (glines[index].size() > 2 && glines[index][0].x < 0 && glines[index][1].x <= 0) {
            glines[index].erase(glines[index].begin());
            //std::cout << glines[index].size() << "\n";
        }
    }

    virtual void reset() {
        Painter::reset();
        for (size_t i = 0; i < GPU_COUNT; i++) glines[i] = std::vector<Point>();
    }

    ~WaveGraphPainter() {
        delete[] glines;
        std::cout << "WaveGraphPainter " << this << " deleted\n";
    }
};

class WaveGraph : public wxPanel {
public:
    // default width and x0 must be overridden by EVT_SIZE
    uint width = 448, height = 169;
    uint x0 = 32, y0 = 32;
    Painter *p = nullptr;

    WaveGraph(wxWindow* parent) : wxPanel(parent) {}

    /*
     * Called by the system of by wxWidgets when the panel needs
     * to be redrawn. You can also trigger this call by
     * calling Refresh()/Update().
     */
    void paintEvent(wxPaintEvent &evt) {
        wxPaintDC dc(this);
        render(dc);
    };

    /*
     * Alternatively, you can use a clientDC to paint on the panel
     * at any time. Using this generally does not free you from
     * catching paint events, since it is possible that e.g. the window
     * manager throws away your drawing when the window comes to the
     * background, and expects you will redraw it when the window comes
     * back (by sending a paint event).
     *
     * In most cases, this will not be needed at all; simply handling
     * paint events and calling Refresh() when a refresh is needed
     * will do the job.
     */
    void paintNow() {
        wxClientDC dc(this);
        render(dc);
    }
    
    void render(wxDC &dc) {
        dc.SetPen(wxPen(wxColor(200, 200, 200), 1));

        // vertical lines
        dc.DrawLine(x(0),             y(0), x(0),             y(height));
        dc.DrawLine(x(width / 4),     y(0), x(width / 4),     y(height));
        dc.DrawLine(x(width / 2),     y(0), x(width / 2),     y(height));
        dc.DrawLine(x(width / 4 * 3), y(0), x(width / 4 * 3), y(height));
        dc.DrawLine(x(width),         y(0), x(width),         y(height));
        // horizontal lines
        dc.DrawLine(x(0), y(0),              x(width), y(0));
        dc.DrawLine(x(0), y(height / 4),     x(width), y(height / 4));
        dc.DrawLine(x(0), y(height / 2),     x(width), y(height / 2));
        dc.DrawLine(x(0), y(height / 4 * 3), x(width), y(height / 4 * 3));
        dc.DrawLine(x(0), y(height),         x(width), y(height));

        if (p == nullptr) return;
        p->x0 = x0;
        p->y0 = y0;
        p->width = width;
        p->height = height;
        p->paint(dc);
    }

    void onDataUpdate(wxCommandEvent &evt) {
        Refresh();
    }

    void onWindowResized(wxSizeEvent &event) {
        wxSize size = GetClientSize();
        x0 = (uint)(0.0625f * size.x);
        width = size.x - 2 * x0;
    }
    
    // some useful events
    /*
     void mouseMoved(wxMouseEvent& event);
     void mouseDown(wxMouseEvent& event);
     void mouseWheelMoved(wxMouseEvent& event);
     void mouseReleased(wxMouseEvent& event);
     void rightClick(wxMouseEvent& event);
     void mouseLeftWindow(wxMouseEvent& event);
     void keyPressed(wxKeyEvent& event);
     void keyReleased(wxKeyEvent& event);
     */

    ~WaveGraph() {
        std::cout << "WaveGraph " << this << " deleted\n";
    }
    
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WaveGraph, wxPanel)
    EVT_COMMAND(WAVE_GRAPH_UPDATED_ID, wxEVT_COMMAND_TEXT_UPDATED, WaveGraph::onDataUpdate)
    EVT_SIZE(WaveGraph::onWindowResized)
// some useful events
/*
 EVT_MOTION(WaveGraph::mouseMoved)
 EVT_LEFT_DOWN(WaveGraph::mouseDown)
 EVT_LEFT_UP(WaveGraph::mouseReleased)
 EVT_RIGHT_DOWN(WaveGraph::rightClick)
 EVT_LEAVE_WINDOW(WaveGraph::mouseLeftWindow)
 EVT_KEY_DOWN(WaveGraph::keyPressed)
 EVT_KEY_UP(WaveGraph::keyReleased)
 EVT_MOUSEWHEEL(WaveGraph::mouseWheelMoved)
 */

// catch paint events
EVT_PAINT(WaveGraph::paintEvent)

END_EVENT_TABLE()


// some useful events
/*
 void WaveGraph::mouseMoved(wxMouseEvent& event) {}
 void WaveGraph::mouseDown(wxMouseEvent& event) {}
 void WaveGraph::mouseWheelMoved(wxMouseEvent& event) {}
 void WaveGraph::mouseReleased(wxMouseEvent& event) {}
 void WaveGraph::rightClick(wxMouseEvent& event) {}
 void WaveGraph::mouseLeftWindow(wxMouseEvent& event) {}
 void WaveGraph::keyPressed(wxKeyEvent& event) {}
 void WaveGraph::keyReleased(wxKeyEvent& event) {}
 */

#endif