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
#include "OmBase.h"

#include "OmBaseUi.h"

#include "OmManager.h"

#include "OmUiMgr.h"
#include "OmUiAddChn.h"
#include "OmUiPropCtx.h"
#include "OmUiPropChn.h"
#include "OmUiProgress.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropCtxChn.h"

/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the _delChn_fth
/// thread finished his job.
///
#define UWM_BACKPURGE_DONE     (WM_APP+1)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxChn::OmUiPropCtxChn(HINSTANCE hins) : OmDialog(hins),
  _delChn_hth(nullptr),
  _delChn_id(-1)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i) {
    this->_chParam[i] = false;
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropCtxChn::~OmUiPropCtxChn()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropCtxChn::id() const
{
  return IDD_PROP_CTX_CHN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::setChParam(unsigned i, bool en)
{
  this->_chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_delChn_init(int id)
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  // store Mod Channel id
  this->_delChn_id = id;
  OmModChan* pChn = pCtx->chnGet(id);

  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMgr*>(this->root())->safemode(true);

  // unselect location
  pCtx->chnSel(-1);

  // if Mod Channel does not have backup data, we can bypass the purge and
  // do directly to the end
  if(pChn->bckHasData()) {

    OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS));

    pUiProgress->open(true);
    pUiProgress->setCaption(L"Delete Mod Channel");
    pUiProgress->setScHeadText(L"Restoring all backup data");

    DWORD dwId;
    this->_delChn_hth = CreateThread(nullptr, 0, this->_delChn_fth, this, 0, &dwId);

  } else {

    // directly delete the location
    this->_delChn_stop();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_delChn_stop()
{
  DWORD exitCode;

  if(this->_delChn_hth) {
    WaitForSingleObject(this->_delChn_hth, INFINITE);
    GetExitCodeThread(this->_delChn_hth, &exitCode);
    CloseHandle(this->_delChn_hth);
    this->_delChn_hth = nullptr;
  }

  // quit the progress dialog
  static_cast<OmUiProgress*>(this->siblingById(IDD_PROGRESS))->quit();

  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  wstring msg;

  // check whether purge succeed
  if(exitCode == 0) {

    // backup data purged, now delete Mod Channel
    if(!pCtx->chnRem(this->_delChn_id)) {
      Om_dlgBox_ok(this->_hwnd, L"Hub properties", IDI_WRN,
                L"Channel remove error", L"Channel "
                "remove process encountered error(s), some file may "
                "had not properly deleted. Please read debug log for details.");
    }

  } else {
    // an error occurred during backup purge
    Om_dlgBox_ok(this->_hwnd, L"Hub properties", IDI_WRN,
                L"Channel backup purge error", L"Channel "
                "backup purge process encountered error(s), some backup data may "
                "had not properly restored. Please read debug log for details.");
  }

  // select the first location in list
  pCtx->chnSel(0);

  // Back to main dialog window to normal state
  static_cast<OmUiMgr*>(this->root())->safemode(false);

  // refresh all dialogs from root
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropCtxChn::_delChn_fth(void* arg)
{
  OmUiPropCtxChn* self = static_cast<OmUiPropCtxChn*>(arg);

  OmContext* pCtx = static_cast<OmUiPropCtx*>(self->_parent)->ctxCur();
  if(!pCtx) return 1;
  OmModChan* pChn = pCtx->chnGet(self->_delChn_id);
  if(!pChn) return 1;

  DWORD exitCode = 0;

  if(!pChn->bckPurge(&self->_delChn_progress_cb, self->siblingById(IDD_PROGRESS))) {
    exitCode = 1; //< report error
  }

  // sends message to window to inform process ended
  PostMessage(self->_hwnd, UWM_BACKPURGE_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropCtxChn::_delChn_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data)
{
  OmUiProgress* pUiProgress = reinterpret_cast<OmUiProgress*>(ptr);

  if(data) {
    pUiProgress->setScItemText(reinterpret_cast<wchar_t*>(data));
  }
  pUiProgress->setPbRange(0, tot);
  pUiProgress->setPbPos(cur);

  return !pUiProgress->abortGet();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onLbLoclsSel()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  int chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  if(chn_id >= 0) {

    OmModChan* pChn = pCtx->chnGet(chn_id);

    this->setItemText(IDC_EC_READ2, pChn->dstDir());
    this->setItemText(IDC_EC_READ3, pChn->libDir());
    this->setItemText(IDC_EC_READ4, pChn->bckDir());

    this->enableItem(IDC_SC_LBL02, true);
    this->enableItem(IDC_EC_READ2, true);
    this->enableItem(IDC_SC_LBL03, true);
    this->enableItem(IDC_EC_READ3, true);
    this->enableItem(IDC_SC_LBL04, true);
    this->enableItem(IDC_EC_READ4, true);

    this->enableItem(IDC_BC_DEL, true);
    this->enableItem(IDC_BC_EDI, true);

    this->enableItem(IDC_BC_UP, (lb_sel > 0));
    int lb_max = this->msgItem(IDC_LB_CHN, LB_GETCOUNT) - 1;
    this->enableItem(IDC_BC_DN, (lb_sel < lb_max));
  } else {
    this->setItemText(IDC_EC_READ2, L"<no selection>");
    this->setItemText(IDC_EC_READ3, L"<no selection>");
    this->setItemText(IDC_EC_READ4, L"<no selection>");

    this->enableItem(IDC_SC_LBL02, false);
    this->enableItem(IDC_EC_READ2, false);
    this->enableItem(IDC_SC_LBL03, false);
    this->enableItem(IDC_EC_READ3, false);
    this->enableItem(IDC_SC_LBL04, false);
    this->enableItem(IDC_EC_READ4, false);

    this->enableItem(IDC_BC_DEL, false);
    this->enableItem(IDC_BC_EDI, false);

    this->enableItem(IDC_BC_UP, false);
    this->enableItem(IDC_BC_DN, false);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onBcUpChn()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);

  // check whether we can move up
  if(lb_sel == 0)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  // retrieve the package List-Box label
  this->msgItem(IDC_LB_CHN, LB_GETTEXT, lb_sel - 1, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel - 1);

  this->msgItem(IDC_LB_CHN, LB_DELETESTRING, lb_sel - 1);

  this->msgItem(IDC_LB_CHN, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, lb_sel, idx);

  this->enableItem(IDC_BC_UP, (lb_sel > 1));
  this->enableItem(IDC_BC_DN, true);

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_CHN_ORDER, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onBcDnChn()
{
  // get selected item (index)
  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  // get count of item in List-Box as index to for insertion
  int lb_max = this->msgItem(IDC_LB_CHN, LB_GETCOUNT) - 1;

  // check whether we can move down
  if(lb_sel == lb_max)
    return;

  wchar_t item_buf[OMM_ITM_BUFF];
  int idx;

  this->msgItem(IDC_LB_CHN, LB_GETTEXT, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  idx = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);
  this->msgItem(IDC_LB_CHN, LB_DELETESTRING, lb_sel);

  lb_sel++;

  this->msgItem(IDC_LB_CHN, LB_INSERTSTRING, lb_sel, reinterpret_cast<LPARAM>(item_buf));
  this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, lb_sel, idx);
  this->msgItem(IDC_LB_CHN, LB_SETCURSEL, lb_sel);

  this->enableItem(IDC_BC_UP, true);
  this->enableItem(IDC_BC_DN, (lb_sel < lb_max));

  // user modified parameter, notify it
  this->setChParam(CTX_PROP_CHN_ORDER, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onBcDelChn()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  int chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  if(chn_id < 0) return;

  OmModChan* pChn = pCtx->chnGet(chn_id);

  // warns the user before committing the irreparable
  if(!Om_dlgBox_ca(this->_hwnd, L"Mod Hub properties", IDI_QRY,
            L"Delete Mod Channel", L"The operation will permanently delete "
            "the Mod Channel \""+pChn->title()+L"\" and its associated data."))
  {
    return;
  }

  if(pChn->bckHasData()) {

    if(!Om_dlgBox_ca(this->_hwnd, L"Mod Hub properties", IDI_QRY,
              L"Remaining backup data", L"The Mod Channel currently have "
              "installed packages, the deletion process will uninstall them "
              "and restore all backup data."))
    {
      return;
    }
  }

  // here we go for Mod Channel delete
  this->_delChn_init(chn_id);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onBcEdiChn()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  int lb_sel = this->msgItem(IDC_LB_CHN, LB_GETCURSEL);
  int chn_id = this->msgItem(IDC_LB_CHN, LB_GETITEMDATA, lb_sel);

  if(chn_id >= 0) {
    // open the Mod Channel Properties dialog
    OmUiPropChn* pUiPropLoc = static_cast<OmUiPropChn*>(this->siblingById(IDD_PROP_CHN));
    pUiPropLoc->chnSet(pCtx->chnGet(chn_id));
    pUiPropLoc->open();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onBcAddChn()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  // open add Mod Channel dialog
  OmUiAddChn* pUiNewLoc = static_cast<OmUiAddChn*>(this->siblingById(IDD_ADD_CHN));
  pUiNewLoc->ctxSet(pCtx);
  pUiNewLoc->open(true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onInit()
{
  // Set buttons inner icons
  this->setBmIcon(IDC_BC_ADD, Om_getResIcon(this->_hins, IDI_BT_ADD));
  this->setBmIcon(IDC_BC_DEL, Om_getResIcon(this->_hins, IDI_BT_REM));
  this->setBmIcon(IDC_BC_EDI, Om_getResIcon(this->_hins, IDI_BT_MOD));
  this->setBmIcon(IDC_BC_UP, Om_getResIcon(this->_hins, IDI_BT_UP));
  this->setBmIcon(IDC_BC_DN, Om_getResIcon(this->_hins, IDI_BT_DN));

  // define controls tool-tips
  this->_createTooltip(IDC_LB_CHN,  L"Mod Channels list");
  this->_createTooltip(IDC_BC_UP,   L"Move up in list");
  this->_createTooltip(IDC_BC_DN,   L"Move down in list");
  this->_createTooltip(IDC_BC_DEL,  L"Delete Mod Channel and its associated data");
  this->_createTooltip(IDC_BC_ADD,  L"Configure a new Mod Channel for this Mod Hub");
  this->_createTooltip(IDC_BC_EDI,  L"Modify Mod Channel properties");

  // Update values
  this->_onRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onResize()
{
  // Mod Channel list Label & ListBox
  this->_setItemPos(IDC_SC_LBL01, 50, 15, 64, 9);
  this->_setItemPos(IDC_LB_CHN, 50, 25, this->cliUnitX()-107, 30);
  // Up and Down buttons
  this->_setItemPos(IDC_BC_UP, this->cliUnitX()-55, 25, 16, 15);
  this->_setItemPos(IDC_BC_DN, this->cliUnitX()-55, 40, 16, 15);
  // Remove & Modify Buttons
  this->_setItemPos(IDC_BC_DEL, 50, 57, 50, 14);
  this->_setItemPos(IDC_BC_EDI, 105, 57, 50, 14);
  // Add button
  this->_setItemPos(IDC_BC_ADD, this->cliUnitX()-108, 57, 50, 14);
  // Target path Label & EditControl
  this->_setItemPos(IDC_SC_LBL02, 50, 75, 110, 9);
  this->_setItemPos(IDC_EC_READ2, 115, 75, this->cliUnitX()-155, 13);
  // Mods Library Label & EditControl
  this->_setItemPos(IDC_SC_LBL03, 50, 87, 110, 9);
  this->_setItemPos(IDC_EC_READ3, 115, 87, this->cliUnitX()-155, 13);
  // Mods Backup Label & EditControl
  this->_setItemPos(IDC_SC_LBL04, 50, 99, 110, 9);
  this->_setItemPos(IDC_EC_READ4, 115, 99, this->cliUnitX()-155, 13);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropCtxChn::_onRefresh()
{
  OmContext* pCtx = static_cast<OmUiPropCtx*>(this->_parent)->ctxCur();
  if(!pCtx) return;

  this->msgItem(IDC_LB_CHN, LB_RESETCONTENT);

  for(unsigned i = 0; i < pCtx->chnCount(); ++i) {
    this->msgItem(IDC_LB_CHN, LB_ADDSTRING, i, reinterpret_cast<LPARAM>(pCtx->chnGet(i)->title().c_str()));
    this->msgItem(IDC_LB_CHN, LB_SETITEMDATA, i, i); // for Mod Channel index reordering
  }

  // Set controls default states and parameters
  this->setItemText(IDC_EC_READ2, L"<no selection>");
  this->setItemText(IDC_EC_READ3, L"<no selection>");
  this->setItemText(IDC_EC_READ4, L"<no selection>");

  this->enableItem(IDC_SC_LBL02, false);
  this->enableItem(IDC_EC_READ2, false);
  this->enableItem(IDC_SC_LBL03, false);
  this->enableItem(IDC_EC_READ3, false);
  this->enableItem(IDC_SC_LBL04, false);
  this->enableItem(IDC_EC_READ4, false);

  this->enableItem(IDC_BC_DEL,  false);
  this->enableItem(IDC_BC_EDI, false);

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropCtxChn::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_BACKPURGE_DONE is a custom message sent from Mod Channel backups purge thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_BACKPURGE_DONE) {
    // end the removing Mod Channel process
    this->_delChn_stop();
  }

  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_LB_CHN: //< Mod Channel(s) list List-Box
      this->_onLbLoclsSel();
      break;

    case IDC_BC_UP: //< Up Buttn
      this->_onBcUpChn();
      break;

    case IDC_BC_DN: //< Down Buttn
      this->_onBcDnChn();
      break;

    case IDC_BC_DEL: //< "Remove" Button
      this->_onBcDelChn();
      break;

    case IDC_BC_EDI: //< "Modify" Button
      this->_onBcEdiChn();
      break;

    case IDC_BC_ADD: //< "New" Button
      this->_onBcAddChn();
      break;
    }
  }

  return false;
}
