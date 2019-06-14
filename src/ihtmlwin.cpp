#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/html/htmlwin.h>

class InfoHtmlWindow : public wxHtmlWindow {
public:
    InfoHtmlWindow(const wxString &path, wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxHtmlWindow(parent, id, pos, size, style) {
        SetBorders(0);

        LoadPage(path);

        // Fit the HTML window to the size of its contents
        SetSize(GetInternalRepresentation()->GetWidth(), GetInternalRepresentation()->GetHeight());
    }

    virtual void OnLinkClicked(const wxHtmlLinkInfo &link) {
        wxLaunchDefaultBrowser(link.GetHref());
    }
};

static void showInfoHtmlWindow(wxWindow *parent, const wxString &title, const wxString &path) {
    wxBoxSizer *topsizer;
    InfoHtmlWindow *html;
    wxDialog dlg(parent, wxID_ANY, title);
    topsizer = new wxBoxSizer(wxVERTICAL) ;

    html = new InfoHtmlWindow(path, &dlg, wxID_ANY, wxDefaultPosition, wxSize(300, 300), wxHW_SCROLLBAR_AUTO);

    topsizer->Add(html, 1, wxALL, 32);

    wxButton *but = new wxButton(&dlg, wxID_OK, "OK");
    but->SetDefault();

    topsizer->Add(but, 0, wxALL | wxALIGN_RIGHT, 15);

    dlg.SetSizer(topsizer) ;
    topsizer->Fit(&dlg) ;

    dlg.ShowModal();
    
    // after `but` pressed
    dlg.DestroyChildren();
}