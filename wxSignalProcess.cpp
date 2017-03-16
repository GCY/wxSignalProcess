 /*
  * wxSignalProcess - wxMathPlot Example
  * Copyright (C) 2017  GCY <http://gcyrobot.blogspot.tw/>.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#include <wx/wx.h>
#include <wx/filename.h>
#include <mathplot.h>

#include <iostream>
#include <fstream>
#include <cmath>

enum{
   ID_EXIT = 200,
   ID_OPEN,
   ID_SAVE_PLOT,
   ID_QRS_DETECTION
};

class App:public wxApp
{
   public:
      bool OnInit();
};

class Frame:public wxFrame
{
   public:
      Frame(const wxString&);
      ~Frame();

      void CreateUI();

      void OnQRSDetection(wxCommandEvent&);
      void OnSavePlot(wxCommandEvent&);
      void OnOpen(wxCommandEvent&);
      void OnExit(wxCommandEvent&);

      void LoadFile(const wxString&);
      void Process(std::vector<double>&,std::vector<double>&);

      void OnFit(wxCommandEvent&);
   private:

      mpWindow *original_plot;
      mpWindow *qrs_detection_plot;

      std::vector<double> vectorx, vectory;

      bool is_load_file;

      DECLARE_EVENT_TABLE();
};

IMPLEMENT_APP(App)
DECLARE_APP(App)

BEGIN_EVENT_TABLE(Frame,wxFrame)
   EVT_MENU(ID_EXIT,Frame::OnExit)
   EVT_MENU(ID_OPEN,Frame::OnOpen)
   EVT_MENU(ID_QRS_DETECTION,Frame::OnQRSDetection)
   EVT_MENU(ID_SAVE_PLOT,Frame::OnSavePlot)
   EVT_MENU(mpID_FIT, Frame::OnFit)
END_EVENT_TABLE()

bool App::OnInit()
{
   Frame *frame = new Frame(wxT("wxSignalProcess"));

   frame->Show(true);

   return true;
}

Frame::Frame(const wxString &title):wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxSize(800,700),wxMINIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU),is_load_file(false)
{
   CreateUI();
}

Frame::~Frame()
{

}

void Frame::CreateUI()
{
   wxMenu *file = new wxMenu;
   file->Append(ID_OPEN,wxT("O&pen\tAlt-o"),wxT("Open"));
   file->Append(ID_SAVE_PLOT,wxT("S&ave Plot\tAlt-s"),wxT("Save Plot"));
   file->Append(ID_EXIT,wxT("E&xit\tAlt-e"),wxT("Exit"));

   wxMenu *tools = new wxMenu;
   tools->Append(ID_QRS_DETECTION,wxT("QRS D&tection\tAlt-d"),wxT("QRS Detection"));

   wxMenu *view = new wxMenu;
   view->Append(mpID_FIT,wxT("F&it\tAlt-f"),wxT("Fit"));
  
   wxMenuBar *bar = new wxMenuBar;

   bar->Append(file,wxT("File"));
   bar->Append(tools,wxT("Tools"));
   bar->Append(view,wxT("View"));
   SetMenuBar(bar);

   wxBoxSizer *top = new wxBoxSizer(wxVERTICAL);
   this->SetSizer(top);

   CreateStatusBar(1);
   SetStatusText(wxT("wxSignalProcess"));

   original_plot = new mpWindow( this, -1, wxPoint(0,0), wxSize(800,300), wxSUNKEN_BORDER );
   qrs_detection_plot = new mpWindow( this, -1, wxPoint(0,350), wxSize(800,300), wxSUNKEN_BORDER );

   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

   topsizer->Add( original_plot, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,5);
   topsizer->Add( qrs_detection_plot, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,5);   
}

void Frame::LoadFile(const wxString &file_name)
{
   std::fstream ecg_file(file_name,std::ios::in);
   mpFXYVector* vectorLayer = new mpFXYVector(wxT("Original ECG Signal"));
   vectorLayer->ShowName(false);
   double idx_val = 0;
   double val;
   while(ecg_file >> val){
      vectorx.push_back(idx_val);
      vectory.push_back(val);
      ++idx_val;
   }
   vectorLayer->SetData(vectorx, vectory);
   vectorLayer->SetContinuity(true);
   wxPen vectorpen(*wxBLACK, 2, wxSOLID);
   vectorLayer->SetPen(vectorpen);
   vectorLayer->SetDrawOutsideMargins(false);

   wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
   mpScaleX* xaxis = new mpScaleX(wxT("X"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   mpScaleY* yaxis = new mpScaleY(wxT("Y"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   original_plot->SetMargins(30, 30, 50, 100);
   original_plot->AddLayer(     xaxis );
   original_plot->AddLayer(     yaxis );
   original_plot->AddLayer(     vectorLayer );
   wxBrush hatch(wxColour(200,200,200), wxSOLID);

   mpInfoLegend* leg;
   original_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true);

   original_plot->EnableDoubleBuffer(true);
   original_plot->SetMPScrollbars(false);
   original_plot->Fit();

}

void Frame::OnQRSDetection(wxCommandEvent &event)
{
   if(!is_load_file){
      wxMessageBox(wxT("Not Load File"));
      return ;
   }

   std::vector<double> signal;

   Process(vectory,signal);

   mpFXYVector* vectorLayer = new mpFXYVector(wxT("QRS Detection"));
   vectorLayer->ShowName(false);
   
   vectorLayer->SetData(vectorx, signal);
   vectorLayer->SetContinuity(true);
   wxPen vectorpen(*wxBLACK, 2, wxSOLID);
   vectorLayer->SetPen(vectorpen);
   vectorLayer->SetDrawOutsideMargins(false);

   wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
   mpScaleX* xaxis = new mpScaleX(wxT("X"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   mpScaleY* yaxis = new mpScaleY(wxT("Y"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   qrs_detection_plot->SetMargins(30, 30, 50, 100);
   qrs_detection_plot->AddLayer(     xaxis );
   qrs_detection_plot->AddLayer(     yaxis );
   qrs_detection_plot->AddLayer(     vectorLayer );
   wxBrush hatch(wxColour(200,200,200), wxSOLID);

   mpInfoLegend* leg;
   qrs_detection_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true);



   std::fstream qrs_file("R peak point.txt",std::ios::in);
   mpFXYVector* vectorLayer2 = new mpFXYVector(_("R Peak"));
   vectorLayer2->ShowName(false);   
   std::vector<double> r_peak_position,r_peak_value;
   int idx_val = 0;
   double val = 0;
   while(qrs_file >> val){
      if(val == 0){
      }
      else{
	 r_peak_position.push_back(idx_val);
	 r_peak_value.push_back(val);
      }
      ++idx_val;

   }
   vectorLayer2->SetData(r_peak_position, r_peak_value);
   vectorLayer2->SetContinuity(false);
   wxPen vectorpen2(*wxRED, 7, wxUSER_DASH);
   vectorLayer2->SetPen(vectorpen2);
   vectorLayer2->SetDrawOutsideMargins(false);

   qrs_detection_plot->AddLayer(     vectorLayer2 );



   qrs_detection_plot->EnableDoubleBuffer(true);
   qrs_detection_plot->SetMPScrollbars(false);
   qrs_detection_plot->Fit();   
}

void Frame::Process(std::vector<double> &input,std::vector<double> &output)
{
   output = input;
}


void Frame::OnFit( wxCommandEvent &WXUNUSED(event) )
{
   original_plot->Fit();
   qrs_detection_plot->Fit();
}

void Frame::OnOpen(wxCommandEvent &event)
{

   wxFileDialog 
        ofd(this, wxT("Open RAW Data file"), "", "",
                       "RAW Data files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if(ofd.ShowModal() == wxID_CANCEL){
        return;     // the user changed idea...
    }


    //PHW(ofd.GetPath().mb_str());
    //wxMessageBox(ofd.GetPath());
    is_load_file = true;
    LoadFile(ofd.GetPath());
}

void Frame::OnSavePlot(wxCommandEvent &event)
{
   if(!is_load_file){
      wxMessageBox(wxT("Not Load File"));
      return ;
   }

   wxFileDialog fileDialog(this, _("Original"), wxT(""), wxT("Original"), wxT("BMP image (*.bmp)|.bmp|JPEG image (*.jpg)|.jpeg;.jpg|PNG image (*.png)|.png"), wxFD_SAVE);
   if(fileDialog.ShowModal() == wxID_OK) {
      wxFileName namePath(fileDialog.GetPath());
      int fileType = wxBITMAP_TYPE_BMP;
      if( namePath.GetExt().CmpNoCase(wxT("jpeg")) == 0 ) fileType = wxBITMAP_TYPE_JPEG;
      if( namePath.GetExt().CmpNoCase(wxT("jpg")) == 0 )  fileType = wxBITMAP_TYPE_JPEG;
      if( namePath.GetExt().CmpNoCase(wxT("png")) == 0 )  fileType = wxBITMAP_TYPE_PNG;
      wxSize imgSize(800,600);
      original_plot->SaveScreenshot(fileDialog.GetPath(), fileType, imgSize, false);
    }
   wxFileDialog fileDialog2(this, _("QRS"), wxT(""), wxT("QRS"), wxT("BMP image (*.bmp)|.bmp|JPEG image (*.jpg)|.jpeg;.jpg|PNG image (*.png)|.png"), wxFD_SAVE);
   if(fileDialog2.ShowModal() == wxID_OK) {
      wxFileName namePath(fileDialog2.GetPath());
      int fileType = wxBITMAP_TYPE_BMP;
      if( namePath.GetExt().CmpNoCase(wxT("jpeg")) == 0 ) fileType = wxBITMAP_TYPE_JPEG;
      if( namePath.GetExt().CmpNoCase(wxT("jpg")) == 0 )  fileType = wxBITMAP_TYPE_JPEG;
      if( namePath.GetExt().CmpNoCase(wxT("png")) == 0 )  fileType = wxBITMAP_TYPE_PNG;
      wxSize imgSize(800,600);
      qrs_detection_plot->SaveScreenshot(fileDialog2.GetPath(), fileType, imgSize, false);
    }

    event.Skip();
}

void Frame::OnExit(wxCommandEvent &event)
{
   Close();
}
