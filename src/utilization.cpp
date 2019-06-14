#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <vector>

#include "graph.cpp"
#include "thread.cpp"
#include "core.cpp"
#include "constants.h"
#include "settings.cpp"

class UtilizationPainter : public WaveGraphPainter {
protected:
    wxCoord w, h;
    uint paintedY, paintedX;
    int textBlockWidth, textBlockHeight;
public:
    UtilizationPainter(wxFrame *frame) : WaveGraphPainter(frame) {}

    // utilization painter paint
    virtual void uppaint(wxDC &dc) {}

    virtual void paint(wxDC &dc) {
        wxString text = "100";
        dc.GetTextExtent(text, &w, &h);
        dc.DrawText(text + "%", x(width) - w, y(0) - h);

        text = "0";
        dc.GetTextExtent(text, &w, &h);
        dc.DrawText(text + "%", x(width) - w, y(height));

        dc.DrawText(toString(UPDATE_DELAY / 1000.0f) + " sec step", x(0), y(0) - dc.GetCharHeight());
        dc.DrawText(toString(GRAPH_LENGTH / 1000.0f) + " sec", x(0), y(height));

        paintedY = y(height);

        // g means gpu
        for (size_t g = 0; g < GPU_COUNT; g++) {
            dc.SetPen(wxPen(gcolors[g], 2));
            if (glines[g].size() > 1) {
                for (size_t i = 1; i < glines[g].size(); i++)
                    dc.DrawLine(
                        x(glines[g][i - 1].x * width), y(height - height / 100.0f * glines[g][i - 1].y),
                        x(glines[g][i].x * width),     y(height - height / 100.0f * glines[g][i].y)
                    );
            }
        }

        dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW), 1));
        dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
        dc.DrawRectangle(0, y0, x0, height + 1);

        uppaint(dc);
    }
};

void drawTextBlock(wxDC &dc, const wxString &text,
                   const int &x, const int &y, const int &width, const int &height,
                   const wxColor &color) {
    int w, h;
    dc.GetTextExtent(text, &w, &h);

    dc.SetPen(wxPen(color, 4));
    dc.DrawRectangle(x, y, width, height);

    dc.DrawText(
        text,
        x + width / 2 - w / 2,
        y + height / 2 - h / 2
    );
}

void drawHorizontalTextBlock(wxDC &dc, const wxString *text, const uint &size,
                             const int &x, const int &y, const int &width, const int &height,
                             const wxColor &color) {
    int w = 0, h = 0;
    int *width_all = new int[size];
    for (size_t i = 0; i < size; i++) {
        wxSize size = dc.GetTextExtent(text[i]);
        width_all[i] = size.x;
        w += size.x + LINE_OFFSET;
        if (h < size.y) h = size.y;
    }
    w -= LINE_OFFSET;

    dc.SetPen(wxPen(color, 4));
    dc.DrawRectangle(x, y, width, height);

    #define drawText(index, x_add) dc.DrawText(text[index], x + width / 2 - w / 2 + x_add, y + height / 2 - h / 2)

    drawText(0, 0);
    for (size_t i = 1; i < size; i++) {
        drawText(i, width_all[i - 1] + LINE_OFFSET);
    }

    #undef drawText

    delete[] width_all;
    delete[] text;
}

// GPU Utilization
class GPUUPainter : public UtilizationPainter {
public:
    GPUUPainter(wxFrame *frame) : UtilizationPainter(frame) {
        wxClientDC dc(frame);
        
        dc.GetTextExtent(
            "GPU Utilization: 100%\nAverage: 100%\nMin: 100%\nMax: 100%",
            &textBlockWidth,
            &textBlockHeight
        );

        textBlockHeight += TEXT_BLOCK_OFFSET;
        textBlockWidth += TEXT_BLOCK_OFFSET;
    }

    virtual void uppaint(wxDC &dc) {
        frame->SetStatusText("GPU Utilization. For help press Ctrl-H");

        paintedY += dc.GetCharHeight() + LINE_OFFSET;
        paintedX = 0;

        size_t cols = (width + LINE_OFFSET) / (textBlockWidth + LINE_OFFSET), rows;
        if (cols == 0) cols = 1;
        if (cols > GPU_COUNT) cols = GPU_COUNT;
        rows = (size_t)ceil(GPU_COUNT / (float)cols);

        for (size_t y = 0; y < rows; y++) {
            for (size_t x = 0; x < cols; x++) {
                if (y * cols + x >= GPU_COUNT) return;
                // measure text block
                wxString text =
                    "GPU Utilization: " + std::to_string(gdata[y * cols + x].level) + "%\n" +
                    "Average: " + std::to_string(gdata[y * cols + x].avgLevel) + "%\n" +
                    "Min: " + std::to_string(gdata[y * cols + x].minLevel) + "%\n" +
                    "Max: " + std::to_string(gdata[y * cols + x].maxLevel) + "%";

                drawTextBlock(
                    dc,
                    text,
                    x(paintedX), y(paintedY),
                    textBlockWidth, textBlockHeight,
                    gcolors[y * cols + x]
                );

                paintedX += textBlockWidth + LINE_OFFSET;
            }

            paintedY += textBlockHeight + LINE_OFFSET;
            paintedX = 0;
        }
    }

    ~GPUUPainter() {
        std::cout << "GPUUPainter " << this << " deleted\n";
    }
};

struct MemData {
    int total, free, used;
};

// Memory Utilization
class MemUPainter : public UtilizationPainter {
public:
    MemData *gmdata; // gpu memory data

    MemUPainter(wxFrame *frame) : UtilizationPainter(frame) {
        gmdata = new MemData[GPU_COUNT];
        for (size_t i = 0; i < GPU_COUNT; i++) gmdata[i] = MemData();

        wxClientDC dc(frame);
        
        dc.GetTextExtent(
            "Memory Utilization: 100%\nAverage: 100%\nMin: 100%\nMax: 100%",
            &textBlockWidth,
            &textBlockHeight
        );

        textBlockHeight += TEXT_BLOCK_OFFSET;
        textBlockWidth += TEXT_BLOCK_OFFSET + LINE_OFFSET + dc.GetTextExtent("Total: 8000 MiB\nFree: 8000 MiB\nUsed: 8000 MiB").x;
    }

    virtual void uppaint(wxDC &dc) {
        frame->SetStatusText("Memory Utilization. For help press Ctrl-H");

        paintedY += dc.GetCharHeight() + LINE_OFFSET;
        paintedX = 0;

        size_t cols = (width + LINE_OFFSET) / (textBlockWidth + LINE_OFFSET), rows;
        if (cols == 0) cols = 1;
        if (cols > GPU_COUNT) cols = GPU_COUNT;
        rows = (size_t)ceil(GPU_COUNT / (float)cols);

        for (size_t y = 0; y < rows; y++) {
            for (size_t x = 0; x < cols; x++) {
                if (y * cols + x >= GPU_COUNT) return;
                // measure text block
                wxString text =
                    "GPU Utilization: " + std::to_string(gdata[y * cols + x].level) + "%\n" +
                    "Average: " + std::to_string(gdata[y * cols + x].avgLevel) + "%\n" +
                    "Min: " + std::to_string(gdata[y * cols + x].minLevel) + "%\n" +
                    "Max: " + std::to_string(gdata[y * cols + x].maxLevel) + "%";

                wxString text1 =
                    "Total: " + std::to_string(gmdata[y * cols + x].total) + " MiB\n" +
                    "Free: " + std::to_string(gmdata[y * cols + x].free) + " MiB\n" +
                    "Used: " + std::to_string(gmdata[y * cols + x].used) + " MiB\n";

                drawHorizontalTextBlock(
                    dc,
                    new wxString[2] {text, text1}, 2,
                    x(paintedX), y(paintedY),
                    textBlockWidth, textBlockHeight,
                    gcolors[y * cols + x]
                );

                paintedX += textBlockWidth + LINE_OFFSET;
            }

            paintedY += textBlockHeight + LINE_OFFSET;
            paintedX = 0;
        }
    }

    virtual void reset() {
        UtilizationPainter::reset();
        for (size_t i = 0; i < GPU_COUNT; i++) gmdata[i] = MemData();
    }

    ~MemUPainter() {
        delete[] gmdata;
        std::cout << "MemUPainter " << this << " deleted\n";
    }
};

class UtilizationWorker : public Worker {
protected:
    wxPanel *panel; // for sending events
    std::vector<std::string> lines;
public:
    UtilizationPainter *p;
    long lastTime = 0;

    UtilizationWorker(wxPanel *panel) {
        this->panel = panel;
    }

    // utilization worker work
    virtual void uwwork() {}

    virtual void work() {
        if (lastTime == 0) {
            lastTime = getTime();
            return;
        }

        uwwork();

        float step = (float)(getTime() - lastTime) / UPDATE_DELAY * GRAPH_STEP;

        // g means gpu
        for (size_t g = 0; g < GPU_COUNT; g++) {
            for (size_t i = 0; i < p->glines[g].size(); i++) p->glines[g][i].x -= step;
            p->glines[g].push_back(Point(1.0f, p->gdata[g].level));
            p->deleteSuperfluousPoints(g);

            // calculate average, min, max
            p->gdata[g].avgLevel = p->gdata[g].maxLevel = 0;
            p->gdata[g].minLevel = 100;
            for (size_t i = 0; i < p->glines[g].size(); i++) {
                p->gdata[g].avgLevel += p->glines[g][i].y;
                if (p->gdata[g].maxLevel < p->glines[g][i].y) p->gdata[g].maxLevel = p->glines[g][i].y;
                if (p->gdata[g].minLevel > p->glines[g][i].y) p->gdata[g].minLevel = p->glines[g][i].y;
            }
            p->gdata[g].avgLevel /= p->glines[g].size();
        }

        // We say that it is time to update Graph
        panel->GetEventHandler()->AddPendingEvent(wxCommandEvent(wxEVT_COMMAND_TEXT_UPDATED, WAVE_GRAPH_UPDATED_ID));

        lastTime = getTime();
    }
};

class GPUUWorker : public UtilizationWorker {
public:
    GPUUWorker(wxPanel *panel) : UtilizationWorker(panel) {}

    virtual void uwwork() {
        lines = split(exec(NVSMI_CMD_GPU_UTILIZATION), "\n");
        // fake, "emulated" GPUs
        // lines[2] = std::to_string(50 + rand() % 20) + " %";
        // lines.push_back(std::to_string(30 + rand() % 10) + " %");
        // lines.push_back(std::to_string(70 + rand() % 30) + " %");
        // lines.push_back("");
        for (size_t i = 1; i < lines.size() - 1; i++) {
            p->gdata[i - 1].level = std::atoi(split(lines[i], " ")[0].c_str());
        }
    }
};

class MemUWorker : public UtilizationWorker {
private:
    std::vector<std::string> data;
public:
    MemUWorker(wxPanel *panel) : UtilizationWorker(panel) {}

    virtual void uwwork() {
        lines = split(exec(NVSMI_CMD_MEM_UTILIZATION), "\n");
        for (size_t i = 1; i < lines.size() - 1; i++) {
            data = split(lines[i], ", ");
            p->gdata[i - 1].level = std::atoi(split(data[0], " ")[0].c_str());
            ((MemUPainter*)p)->gmdata[i - 1].total = std::atoi(split(data[1], " ")[0].c_str());
            ((MemUPainter*)p)->gmdata[i - 1].free = std::atoi(split(data[2], " ")[0].c_str());
            ((MemUPainter*)p)->gmdata[i - 1].used = std::atoi(split(data[3], " ")[0].c_str());
        }
    }
};