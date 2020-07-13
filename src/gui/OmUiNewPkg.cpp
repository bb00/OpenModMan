/*
  This file is part of Open Mod Manager.

  Open Mod Manager is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Mod Manager is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Mod Manager. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/res/resource.h"
#include "OmManager.h"
#include "gui/OmUiNewPkg.h"
#include "OmPackage.h"
#include "gui/OmUiProgress.h"

/// \brief Compatible image formats filter
///
/// Open dialog file filter for compatibles images file formats.
///
#define IMAGE_FILE_FILTER     L"Image file (*.bmp,*.jpg,*.jpeg,*.tiff,*.tif,*.gif,*.png)\0*.BMP;*.JPG;*.JPEG;*.TIFF;*.GIF;*.PNG;\
                              \0BMP file (*.bmp)\0*.BMP;\0JPEG file (*.jpg,*.jpeg)\0*.JPG;*.JPEG\0PNG file (*.png)\0*.PNG;\0CompuServe GIF (*.gif)\0*.GIF;\
                              \0TIFF file (*.tiff,*.tif)\0*.TIFF;*.TIF;\0"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the remLocation_fth
/// thread finished his job.
///
#define UWM_BUILDPKG_DONE    (WM_APP+1)

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewPkg::OmUiNewPkg(HINSTANCE hins) : OmDialog(hins),
  _hBlankImg(nullptr),
  _hImgSource(nullptr),
  _buildPkg_hth(nullptr)
{
  this->addChild(new OmUiProgress(hins)); //< for package creation process

  // load the package blank picture
  this->_hBlankImg = reinterpret_cast<HBITMAP>(LoadImage(this->_hins,MAKEINTRESOURCE(IDB_PKG_BLANK),IMAGE_BITMAP,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE,0));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiNewPkg::~OmUiNewPkg()
{
  DeleteObject(this->_hBlankImg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiNewPkg::id() const
{
  return IDD_NEW_PKG;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onShow()
{
  // Enable Create From folder
  SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_SETCHECK, 1, 0);

  // add items into Combo-Box
  HWND hCb = GetDlgItem(this->_hwnd, IDC_CB_LEVEL);

  unsigned cb_count = SendMessageW(hCb, CB_GETCOUNT, 0, 0);

  if(!cb_count) {
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"None ( very fast )");
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"Low ( fast )");
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"Normal ( slow )");
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)"Best ( very slow )");
  }
  SendMessageW(hCb, CB_SETCURSEL, 2, 0);

  // set default snapshot
  SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)this->_hBlankImg);

  // set font for readme
  HFONT hFont = CreateFont(14,0,0,0,400,false,false,false,1,0,0,5,0,"Consolas");
  SendMessage(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), WM_SETFONT, (WPARAM)hFont, 1);

  // Set Dependencies Add and Rem buttons "icon"
  SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_ADD), L"\u2795"); // Heavy Plus Sign +
  //SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_DEL), L"\u274C"); // Cross Mark
  SetWindowTextW(GetDlgItem(this->_hwnd, IDC_BC_DEL), L"\u2796"); // Heavy Minus Sign -
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onResize()
{
  // From folder RadioButton
  this->_setControlPos(IDC_BC_RAD01, 10, 10, 150, 9);
  // From Folder EditControl & Brows Button
  this->_setControlPos(IDC_EC_INPT1, 10, 20, this->width()-38, 13);
  this->_setControlPos(IDC_BC_BROW1, this->width()-26, 20, 16, 13);

  // From existing Package RadioButton
  this->_setControlPos(IDC_BC_RAD02, 10, 40, 150, 9);
  // From existing Package EditControl & Brows Button
  this->_setControlPos(IDC_EC_INPT2, 10, 50, this->width()-38, 13);
  this->_setControlPos(IDC_BC_BROW2, this->width()-26, 50, 16, 13);

  // Dependencies CheckBox
  this->_setControlPos(IDC_BC_CHK03, 10, 70, 112, 9);
  // Dependencies EditControl a Add Button
  this->_setControlPos(IDC_EC_INPT3, 10, 80, this->width()-38, 13);
  this->_setControlPos(IDC_BC_ADD, this->width()-26, 80, 16, 13);
  // Dependencies ListBox & Del button
  this->_setControlPos(IDC_LB_DPNDS, 10, 95, this->width()-38, 24);
  this->_setControlPos(IDC_BC_DEL, this->width()-26, 95, 16, 13);

  // Picture CheckBox & Load button
  this->_setControlPos(IDC_BC_CHK04, 10, 125, 120, 9);
  this->_setControlPos(IDC_BC_BROW4, this->width()-60, 125, 50, 14);
  // Picture Bitmap & Label
  this->_setControlPos(IDC_SB_PKIMG, 10, 136, 85, 78);
  this->_setControlPos(IDC_SC_LBL04, 115, 165, 200, 9);

  // Description CheckBox & Load button
  this->_setControlPos(IDC_BC_CHK05, 10, 220, 120, 9);
  this->_setControlPos(IDC_BC_BROW5, this->width()-60, 220, 50, 14);
  // Description EditControl
  this->_setControlPos(IDC_EC_PKTXT, 10, 235, this->width()-20, this->height()-360);

  // Destination label
  this->_setControlPos(IDC_SC_LBL06, 10, this->height()-115, 120, 9);
  // Destination file name
  this->_setControlPos(IDC_EC_INPT4, 10, this->height()-105, this->width()-72, 14);
  // Destination brows button
  this->_setControlPos(IDC_BC_SAVE, this->width()-60, this->height()-105, 50, 14);

  // Parsed name label & entry
  this->_setControlPos(IDC_SC_LBL07, 10, this->height()-84, 50, 9);
  this->_setControlPos(IDC_EC_INPT5, 60, this->height()-85, this->width()-175, 12);

  // Parsed version label & entry
  this->_setControlPos(IDC_SC_LBL08, this->width()-101, this->height()-84, 51, 9);
  this->_setControlPos(IDC_EC_INPT6, this->width()-45, this->height()-85, 35, 12);

  // Zip Level Label
  this->_setControlPos(IDC_SC_LBL09, 10, this->height()-60, 120, 9);
  // Zip Level ComboBox
  this->_setControlPos(IDC_CB_LEVEL, 10, this->height()-50, this->width()-20, 14);

  // ----- Separator
  this->_setControlPos(IDC_SC_SEPAR, 5, this->height()-25, this->width()-10, 1);
  // Save As... Button
  this->_setControlPos(IDC_BC_OK, this->width()-108, this->height()-19, 50, 14);
  // Close Button
  this->_setControlPos(IDC_BC_CANCEL, this->width()-54, this->height()-19, 50, 14);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onRefresh()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_onQuit()
{
  DeleteObject(this->_hImgSource);
  this->_hImgSource = nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewPkg::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_BUILDPKG_DONE is a custom message sent from Location deletion thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BUILDPKG_DONE) {
    // end the removing Location process
    this->_buildPkg_end();
    // quit the dialog
    this->quit();
    // refresh the main window dialog, this will also refresh this one
    this->root()->refresh();
  }

  if(uMsg == WM_COMMAND) {

    bool has_changed = false;

    OmManager* manager = reinterpret_cast<OmManager*>(this->_data);
    OmContext* curCtx = manager->curContext();
    OmLocation* curLoc = curCtx ? curCtx->curLocation() : nullptr;

    int lb_sel;
    wchar_t wcbuf[OMM_MAX_PATH];
    wchar_t sldir[OMM_MAX_PATH];
    wchar_t fpath[OMM_MAX_PATH];

    switch(LOWORD(wParam))
    {
    case IDC_BC_RAD01:
    case IDC_BC_RAD02:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT1), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT2), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW2), false);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT1), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW1), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT2), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW2), true);
      }
      has_changed = true;
      break;

    case IDC_BC_BROW1:
      // select the initial location for browsing start
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
      if(!wcslen(wcbuf)) {
        if(curLoc) wcscpy(sldir, curLoc->libraryDir().c_str());
      } else {
        wcscpy(sldir, Om_getDirPart(wcbuf).c_str());
      }

      if(Om_dialogBrowseDir(fpath, this->_hwnd, L"Select installation file(s) location", sldir)) {
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, fpath);
      }
      break;

    case IDC_BC_BROW2:
      // select the initial location for browsing start
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
      if(!wcslen(wcbuf)) {
        if(curLoc) wcscpy(sldir, curLoc->libraryDir().c_str());
      } else {
        wcscpy(sldir, Om_getDirPart(wcbuf).c_str());
      }

      if(Om_dialogOpenFile(fpath, this->_hwnd, L"Select Package file", OMM_PKG_FILES_FILTER, sldir)) {
        OmPackage pkg;
        if(pkg.sourceParse(fpath)) {
          SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_RESETCONTENT, 0, 0);
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, fpath);
          if(pkg.dependCount()) {
            SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK03), BM_SETCHECK, 1, 0);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), true);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), true);
            for(unsigned i = 0; i < pkg.dependCount(); ++i) {
              SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_ADDSTRING, 0, (LPARAM)pkg.depend(i).c_str());
            }
          }
          if(pkg.picture()) {
            SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK04), BM_SETCHECK, 1, 0);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), true);
            this->_hImgSource = static_cast<HBITMAP>(CopyImage(pkg.picture(),IMAGE_BITMAP,0,0,0));
            HBITMAP hBmp = static_cast<HBITMAP>(Om_getBitmapThumbnail(this->_hImgSource,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE));
            SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
            DeleteObject(hBmp);
          }
          if(pkg.desc().size()) {
            SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK05), BM_SETCHECK, 1, 0);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW5), true);
            EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), true);
            SetDlgItemTextW(this->_hwnd, IDC_EC_PKTXT, pkg.desc().c_str());
          }
        } else {
          wstring err = L"The file \""; err += fpath;
          err += L"\" is not valid Open Mod Manager Package.";
          Om_dialogBoxErr(this->_hwnd, L"Error parsing Package", err);
        }
      }
      break;

    case IDC_EC_INPT1:
      has_changed = true;
      break;

    case IDC_EC_INPT2:
      has_changed = true;
      break;

    case IDC_BC_CHK03: // Add Dependencies CheckBox
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK03), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_INPT3), false);
      }
    break;

    case IDC_EC_INPT3: // Dependencies EditControl
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, wcbuf, OMM_MAX_PATH);
      if(wcslen(wcbuf)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), false);
      }
      break;

    case IDC_LB_DPNDS: // Dependencies ListBox
      if(SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_GETCURSEL, 0, 0) >= 0) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
      }
      break;

    case IDC_BC_ADD: // Add Dependency Button
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, wcbuf, OMM_MAX_PATH);
      if(wcslen(wcbuf)) {
        SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_ADDSTRING, 0, (LPARAM)wcbuf);
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT3, L"");
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_ADD), false);
      }
      break;

    case IDC_BC_DEL: // Remove Dependency Button
      lb_sel = SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_GETCURSEL, 0, 0);
      if(lb_sel >= 0) {
        SendMessageW(GetDlgItem(this->_hwnd, IDC_LB_DPNDS), LB_DELETESTRING, lb_sel, 0);;
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_DEL), false);
      }
      break;

    case IDC_BC_CHK04:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK04), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW4), false);
        this->_hImgSource = nullptr;
        SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)this->_hBlankImg);
      }
      break;

    case IDC_BC_BROW4:
      // select the start directory from package source path
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
        wcscpy(sldir, Om_getDirPart(wcbuf).c_str());
      } else {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
        wcscpy(sldir, Om_getDirPart(wcbuf).c_str());
      }

      if(Om_dialogOpenFile(fpath, this->_hwnd, L"Open Image file", IMAGE_FILE_FILTER, sldir)) {
        if(Om_isFile(fpath)) {
          this->_hImgSource = static_cast<HBITMAP>(Om_loadBitmap(fpath));
          HBITMAP hBmp = static_cast<HBITMAP>(Om_getBitmapThumbnail(this->_hImgSource,OMM_PKG_THMB_SIZE,OMM_PKG_THMB_SIZE));
          SendMessage(GetDlgItem(this->_hwnd, IDC_SB_PKIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
          DeleteObject(hBmp);
        }
      }
      break;

    case IDC_BC_CHK05:
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_CHK05), BM_GETCHECK, 0, 0)) {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW5), true);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), true);
      } else {
        EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_BROW5), false);
        EnableWindow(GetDlgItem(this->_hwnd, IDC_EC_PKTXT), false);
      }
    break;

    case IDC_BC_BROW5:
      // select the start directory from package source path
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
        wcscpy(sldir, Om_getDirPart(wcbuf).c_str());
      } else {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
        wcscpy(sldir, Om_getDirPart(wcbuf).c_str());
      }

      if(Om_dialogOpenFile(fpath, this->_hwnd, L"Open Text file", L"Text file (*.txt)\0*.TXT\0", sldir)) {
        if(Om_isFile(fpath)) {
          string ascii = Om_loadPlainText(fpath);
          SetDlgItemTextA(this->_hwnd, IDC_EC_PKTXT, ascii.c_str());
        }
      }
      break;

    case IDC_BC_SAVE:

      // create the file initial name based on source folder name
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
        swprintf(fpath, OMM_MAX_PATH, L"%ls.zip", Om_getFilePart(wcbuf).c_str());
      } else {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
        wcscpy(fpath, Om_getFilePart(wcbuf).c_str());
      }

      // select the initial location for browsing start
      if(curLoc) {
        wcscpy(sldir, curLoc->libraryDir().c_str());
      } else {
        wcscpy(sldir, Om_getDirPart(wcbuf).c_str());
      }

      if(Om_dialogSaveFile(fpath, this->_hwnd, L"Save Package as...", OMM_PKG_FILES_FILTER, sldir)) {
        if(Om_isValidName(fpath)) {
          SetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, fpath);
        } else {
          Om_dialogBoxErr(this->_hwnd, L"Invalid file name", L"The specified file name is not valid.");
        }
      }
      break;

    case IDC_EC_INPT4:
      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);
      if(wcslen(wcbuf)) {
        wstring name, vers;
        Om_parsePkgIdent(name, vers, wcbuf, true, true);
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT5, name.c_str());
        SetDlgItemTextW(this->_hwnd, IDC_EC_INPT6, vers.c_str());
      }
      has_changed = true;
      break;

    case IDC_BC_OK:
      this->_apply();
      break;

    case IDC_BC_CANCEL:
      this->quit();
      break;
    }

    if(has_changed) {
      bool allow = true;

      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD01), BM_GETCHECK, 0, 0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
      }
      if(SendMessage(GetDlgItem(this->_hwnd, IDC_BC_RAD02), BM_GETCHECK, 0, 0)) {
        GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
      }

      if(!wcslen(wcbuf))
        allow = false;

      GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);

      if(!wcslen(wcbuf))
        allow = false;

      EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_OK), allow);
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiNewPkg::_apply()
{
  wchar_t wcbuf[OMM_MAX_PATH];

  // Step 1, verify everything
  if(SendMessage(GetDlgItem(this->_hwnd,IDC_BC_RAD01),BM_GETCHECK,0,0)) {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
    if(!Om_isDir(wcbuf)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source path",
                                    L"The specified source path is not a "
                                    L"valid folder.");
      return false;
    }
  } else {
    GetDlgItemTextW(this->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
    if(!Om_isFileZip(wcbuf)) {
      Om_dialogBoxWarn(this->_hwnd, L"Invalid source file",
                                    L"The specified source Package is not a "
                                    L"valid zip file.");
      return false;
    }
  }

  GetDlgItemTextW(this->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);
  if(Om_isValidName(wcbuf)) {
    if(Om_isFile(wcbuf)) {
      wstring msg = L"The file \"" + Om_getFilePart(wcbuf);
      msg += L"\" already exists, do you want to overwrite the existing file ?";
      if(!Om_dialogBoxQuerry(this->_hwnd, L"File already exists", msg)) {
        return false;
      }
    }
  } else {
    Om_dialogBoxErr(this->_hwnd,  L"Invalid file name",
                                  L"The specified destination file "
                                  L"name is not valid.");
    return false;
  }

  // disable the OK button
  EnableWindow(GetDlgItem(this->_hwnd, IDC_BC_OK), false);

  // start package building thread
  DWORD dWid;
  this->_buildPkg_hth = CreateThread(nullptr, 0, this->_buildPkg_fth, this, 0, &dWid);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiNewPkg::_buildPkg_end()
{
  if(this->_buildPkg_hth) {
    WaitForSingleObject(this->_buildPkg_hth, INFINITE);
    CloseHandle(this->_buildPkg_hth);
    this->_buildPkg_hth = nullptr;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiNewPkg::_buildPkg_fth(void* arg)
{
  OmUiNewPkg* self = reinterpret_cast<OmUiNewPkg*>(arg);

  OmUiProgress* uiProgress = reinterpret_cast<OmUiProgress*>(self->childById(IDD_PROGRESS));

  // open the progress dialog
  uiProgress->open(true);

  uiProgress->setCaption(L"Building Package");
  uiProgress->setTitle(L"Preparing data...");

  wchar_t wcbuf[OMM_MAX_PATH];

  OmPackage package(nullptr);

  // get source (either a folder or another package) and parse it
  GetDlgItemTextW(self->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
  if(SendMessage(GetDlgItem(self->_hwnd,IDC_BC_RAD01),BM_GETCHECK,0,0)) {
    GetDlgItemTextW(self->_hwnd, IDC_EC_INPT1, wcbuf, OMM_MAX_PATH);
  } else {
    GetDlgItemTextW(self->_hwnd, IDC_EC_INPT2, wcbuf, OMM_MAX_PATH);
  }

  if(!package.sourceParse(wcbuf)) {
    // TODO: Add some dialog error here... one day
    return 1;
  }

  // get package dependencies list
  if(SendMessage(GetDlgItem(self->_hwnd, IDC_BC_CHK03), BM_GETCHECK, 0, 0)) {
    unsigned lb_cnt = SendMessageW(GetDlgItem(self->_hwnd, IDC_LB_DPNDS), LB_GETCOUNT, 0, 0);
    if(lb_cnt) {
      for(unsigned i = 0; i < lb_cnt; ++i) {
        SendMessageW(GetDlgItem(self->_hwnd, IDC_LB_DPNDS), LB_GETTEXT, i, (LPARAM)wcbuf);
        package.addDepend(wcbuf);
      }
    }
  }

  // get package picture data
  if(SendMessage(GetDlgItem(self->_hwnd, IDC_BC_CHK04), BM_GETCHECK, 0, 0)) {
    package.setPicture(self->_hImgSource);
  }

  // get package description text
  if(SendMessage(GetDlgItem(self->_hwnd, IDC_BC_CHK05), BM_GETCHECK, 0, 0)) {
    size_t len = SendMessageW(GetDlgItem(self->_hwnd, IDC_EC_PKTXT), WM_GETTEXTLENGTH, 0, 0);
    wchar_t* wtxt = nullptr;
    try {
      wtxt = new wchar_t[len+1];
    } catch (std::bad_alloc& ba) {
      // TODO: Add some dialog error here... one day
      return 1;
    }
    SendMessageW(GetDlgItem(self->_hwnd, IDC_EC_PKTXT), WM_GETTEXT, len+1, (LPARAM)wtxt);
    package.setDesc(wtxt);
    delete [] wtxt;
  }

  // get package compression level
  unsigned zlv = SendMessageW(GetDlgItem(self->_hwnd, IDC_CB_LEVEL), CB_GETCURSEL, 0, 0);

  // get destination filename
  GetDlgItemTextW(self->_hwnd, IDC_EC_INPT4, wcbuf, OMM_MAX_PATH);
  wstring dst_path = wcbuf;

  // hide the main dialog
  self->hide();

  uiProgress->setTitle(L"Add files to packages...");

  HWND hPb = (HWND)uiProgress->getProgressBar();
  HWND hSc = (HWND)uiProgress->getStaticComment();

  wstring msg;

  if(!package.save(dst_path, zlv, hPb, hSc, uiProgress->getAbortPtr())) {

    // show error dialog box
    msg = L"An error occurred during Package creation:\n" + package.lastError();
    Om_dialogBoxErr(uiProgress->hwnd(), L"Package creation error", msg);

    // close the progress dialog
    uiProgress->quit();

  } else {

    // close the progress dialog
    uiProgress->quit();

    // show success dialog box
    msg = L"The Package \"" + Om_getFilePart(dst_path);
    msg += L"\" was successfully created.";
    Om_dialogBoxInfo(self->_hwnd, L"Package creation success", msg);
  }

  PostMessage(self->_hwnd, UWM_BUILDPKG_DONE, 0, 0);

  return 0;
}
