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
#include "OmUiAddRep.h"
#include "OmUiPropChnStg.h"
#include "OmUiPropChnLib.h"
#include "OmUiPropChnBck.h"
#include "OmUiPropChnNet.h"
#include "OmUiProgress.h"

#include "OmUtilFs.h"
#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChn.h"


/// \brief Custom window Message
///
/// Custom window message to notify the dialog window that the moveBackup_fth
/// thread finished his job.
///
#define UWM_MOVEBACKUP_DONE     (WM_APP+4)


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChn::OmUiPropChn(HINSTANCE hins) : OmDialogProp(hins),
  _pChn(nullptr),
  _movBck_hth(nullptr),
  _movBck_dest()
{
  // create tab dialogs
  this->_addPage(L"Settings", new OmUiPropChnStg(hins));
  this->_addPage(L"Library", new OmUiPropChnLib(hins));
  this->_addPage(L"Backup", new OmUiPropChnBck(hins));
  this->_addPage(L"Repositories", new OmUiPropChnNet(hins));

  // creates child sub-dialogs
  this->addChild(new OmUiAddRep(hins));     //< Dialog for new Repository
  this->addChild(new OmUiProgress(hins));   //< for Mod Channel backup transfer or deletion
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChn::~OmUiPropChn()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChn::id()  const
{
  return IDD_PROP_CHN;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropChn::checkChanges()
{
  OmUiPropChnStg* pUiPropLocStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));
  OmUiPropChnLib* pUiPropLocLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));
  OmUiPropChnBck* pUiPropLocBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));
  OmUiPropChnNet* pUiPropLocNet  = static_cast<OmUiPropChnNet*>(this->childById(IDD_PROP_CHN_NET));

  bool changed = false;

  wstring item_str;

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_TITLE)) { //< parameter for Mod Channel title
    pUiPropLocStg->getItemText(IDC_EC_INP01, item_str);
    if(this->_pChn->title() != item_str) changed = true;
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_INSTALL)) { //< parameter for Target path
    pUiPropLocStg->getItemText(IDC_EC_INP02, item_str);
    if(this->_pChn->dstDir() != item_str) changed = true;
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_LIBRARY)) { //< parameter for Library path
    if(pUiPropLocStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      pUiPropLocStg->getItemText(IDC_EC_INP03, item_str);
      if(this->_pChn->libDir() != item_str || !this->_pChn->hasCustLibDir())
        changed = true;
    } else {
      if(this->_pChn->hasCustLibDir()) changed = true;
    }
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_BACKUP)) { //< parameter for Backup path
    if(pUiPropLocStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
      pUiPropLocStg->getItemText(IDC_EC_INP04, item_str);
      if(this->_pChn->bckDir() != item_str || !this->_pChn->hasCustBckDir())
        changed = true;
    } else {
      if(this->_pChn->hasCustBckDir()) changed = true;
    }
  }

  if(pUiPropLocBck->hasChParam(CHN_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(pUiPropLocBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      int cb_sel = pUiPropLocBck->msgItem(IDC_CB_LVL, CB_GETCURSEL);
      if(this->_pChn->bckZipLevel() != cb_sel) changed = true;
    } else {
      if(this->_pChn->bckZipLevel() != -1) changed = true;
    }
  }

  if(pUiPropLocLib->hasChParam(CHN_PROP_LIB_DEVMODE)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_pChn->libDevMode())
      changed = true;
  }

  if(pUiPropLocLib->hasChParam(CHN_PROP_LIB_WARNINGS)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_pChn->warnOverlaps())
      changed = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_pChn->warnExtraInst())
      changed = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK) != this->_pChn->warnMissDeps())
      changed = true;
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK) != this->_pChn->warnExtraUnin())
      changed = true;
  }

  if(pUiPropLocLib->hasChParam(CHN_PROP_LIB_SHOWHIDDEN)) {
    if(pUiPropLocLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK) != this->_pChn->libShowHidden())
      changed = true;
  }

  if(pUiPropLocNet->hasChParam(CHN_PROP_NET_WARNINGS)) {
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK) != this->_pChn->warnExtraDnld())
      changed = true;
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK) != this->_pChn->warnMissDnld())
      changed = true;
    if(pUiPropLocNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK) != this->_pChn->warnUpgdBrkDeps())
      changed = true;
  }

  if(pUiPropLocNet->hasChParam(CHN_PROP_NET_ONUPGRADE)) {
    if(pUiPropLocNet->msgItem(IDC_BC_RAD02, BM_GETCHECK) != this->_pChn->upgdRename())
      changed = true;
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropChn::applyChanges()
{
  OmUiPropChnBck* pUiPropLocBck  = static_cast<OmUiPropChnBck*>(this->childById(IDD_PROP_CHN_BCK));
  OmUiPropChnLib* pUiPropLocLib  = static_cast<OmUiPropChnLib*>(this->childById(IDD_PROP_CHN_LIB));
  OmUiPropChnStg* pUiPropLocStg  = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));
  OmUiPropChnNet* pUiPropLocNet  = static_cast<OmUiPropChnNet*>(this->childById(IDD_PROP_CHN_NET));

  wstring chn_name, chn_dst, chn_lib, chn_bck;

  bool cust_lib = false;
  bool cust_bck = false;

  // Step 1, verify everything
  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_TITLE)) { //< parameter for Channel name
    pUiPropLocStg->getItemText(IDC_EC_INP01, chn_name);
    if(!Om_dlgValidName(this->_hwnd, L"Channel name", chn_name))
      return false;
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_INSTALL)) { //< parameter for Target path
    pUiPropLocStg->getItemText(IDC_EC_INP02, chn_dst);
    if(!Om_dlgValidDir(this->_hwnd, L"Target path", chn_dst))
      return false;
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_LIBRARY)) { //< parameter for Library path
    cust_lib = pUiPropLocStg->msgItem(IDC_BC_CKBX1, BM_GETCHECK);
    if(cust_lib) { //< Custom Library folder Check-Box checked
      pUiPropLocStg->getItemText(IDC_EC_INP03, chn_lib);
      if(Om_dlgValidPath(this->_hwnd, L"Library folder path", chn_lib)) {
        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Library folder", chn_lib))
          return false;
      } else {
        return false;
      }
    }
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_BACKUP)) { //< parameter for Backup path
    cust_bck = pUiPropLocStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK);
    if(cust_bck) { //< Custom Backup folder Check-Box checked
      pUiPropLocStg->getItemText(IDC_EC_INP04, chn_bck);
      if(Om_dlgValidPath(this->_hwnd, L"Backup folder path", chn_bck)) {
        if(!Om_dlgCreateFolder(this->_hwnd, L"Custom Backup folder", chn_bck))
          return false;
      } else {
        return false;
      }
    }
  }

  // Step 2, save changes
  if(pUiPropLocLib->hasChParam(CHN_PROP_LIB_DEVMODE)) {

    this->_pChn->setLibDevMode(pUiPropLocLib->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->setChParam(CHN_PROP_LIB_DEVMODE, false);
  }

  if(pUiPropLocLib->hasChParam(CHN_PROP_LIB_WARNINGS)) {

    this->_pChn->setWarnOverlaps(pUiPropLocLib->msgItem(IDC_BC_CKBX2, BM_GETCHECK));
    this->_pChn->setWarnExtraInst(pUiPropLocLib->msgItem(IDC_BC_CKBX3, BM_GETCHECK));
    this->_pChn->setWarnMissDeps(pUiPropLocLib->msgItem(IDC_BC_CKBX4, BM_GETCHECK));
    this->_pChn->setWarnExtraUnin(pUiPropLocLib->msgItem(IDC_BC_CKBX5, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->setChParam(CHN_PROP_LIB_WARNINGS, false);
  }

  if(pUiPropLocLib->hasChParam(CHN_PROP_LIB_SHOWHIDDEN)) {

    this->_pChn->setLibShowHidden(pUiPropLocLib->msgItem(IDC_BC_CKBX6, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocLib->setChParam(CHN_PROP_LIB_SHOWHIDDEN, false);
  }

  if(pUiPropLocNet->hasChParam(CHN_PROP_NET_WARNINGS)) {

    this->_pChn->setWarnExtraDnld(pUiPropLocNet->msgItem(IDC_BC_CKBX1, BM_GETCHECK));
    this->_pChn->setWarnMissDnld(pUiPropLocNet->msgItem(IDC_BC_CKBX2, BM_GETCHECK));
    this->_pChn->setWarnUpgdBrkDeps(pUiPropLocNet->msgItem(IDC_BC_CKBX3, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocNet->setChParam(CHN_PROP_NET_WARNINGS, false);
  }


  if(pUiPropLocNet->hasChParam(CHN_PROP_NET_ONUPGRADE)) {

    this->_pChn->setUpgdRename(pUiPropLocNet->msgItem(IDC_BC_RAD02, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropLocNet->setChParam(CHN_PROP_NET_ONUPGRADE, false);
  }

  if(pUiPropLocBck->hasChParam(CHN_PROP_BCK_COMP_LEVEL)) { //< parameter for Backup compression level
    if(pUiPropLocBck->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
      int cb_sel = pUiPropLocBck->msgItem(IDC_CB_LVL, CB_GETCURSEL);
      if(cb_sel >= 0) this->_pChn->setBckZipLevel(cb_sel);
    } else {
      // disable zipped backups
      this->_pChn->setBckZipLevel(-1);
    }

    // Reset parameter as unmodified
    pUiPropLocBck->setChParam(CHN_PROP_BCK_COMP_LEVEL, false);
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_INSTALL)) { //< parameter for Mod Channel Install path
    this->_pChn->setDstDir(chn_dst);

    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(CHN_PROP_STG_INSTALL, false);
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_LIBRARY)) { //< parameter for Mod Channel Library path
    if(cust_lib) {
      this->_pChn->setCustLibDir(chn_lib);
    } else {
      this->_pChn->remCustLibDir();
    }
    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(CHN_PROP_STG_LIBRARY, false);
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_BACKUP)) { //< parameter for Mod Channel Backup path

    // check whether we need to transfer backup data, if yes we
    // launch the process via a new thread with progress dialog window. The
    // Mod Channel backup setting will be properly modified withing the
    // dedicates thread
    if(this->_pChn->bckDir() != chn_bck) {

      // start move backup thread
      this->_movBck_init(chn_bck);

      // if backup transfer thread is running, we do not quit since it will
      // end the process before it ends. We will wait for the UWM_MOVEBACKUP_DONE
      // message sent from the thread, then quit this dialog safely at this
      // moment
      return false;

    } else {
      // uncheck the unnecessary "custom" flag
      if(!cust_bck && this->_pChn->hasCustBckDir())
        this->_pChn->remCustBckDir();

      // Reset parameter as unmodified
      pUiPropLocStg->setChParam(CHN_PROP_STG_BACKUP, false);
    }
  }

  if(pUiPropLocStg->hasChParam(CHN_PROP_STG_TITLE)) { //< parameter for Mod Channel title

    this->_pChn->setTitle(chn_name);

    // To prevent crash during operation we unselect location in the main dialog
    static_cast<OmUiMgr*>(this->root())->safemode(true);

    if(!this->_pChn->renameHome(chn_name)) {
      Om_dlgBox_okl(this->_hwnd, L"Channel properties", IDI_WRN,
                   L"Channel files rename error", L"Channel "
                   "title changed but folder and definition file rename "
                   "failed because of the following error:", this->_pChn->lastError());
    }

    // Back to main dialog window to normal state
    static_cast<OmUiMgr*>(this->root())->safemode(false);

    // Reset parameter as unmodified
    pUiPropLocStg->setChParam(CHN_PROP_STG_TITLE, false);
  }

  // disable Apply button
  this->enableItem(IDC_BC_APPLY, false);

  // refresh all dialogs from root (Main dialog)
  this->root()->refresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChn::_movBck_init(const wstring& dest)
{
  if(!this->_pChn) return;

  // verify we have something to move
  if(!Om_isDirEmpty(this->_pChn->bckDir())) {
    this->_movBck_stop();
  }

  // keep destination path
  this->_movBck_dest = dest;

  // To prevent crash during operation we unselect location in the main dialog
  static_cast<OmUiMgr*>(this->root())->safemode(true);

  OmUiProgress* pUiProgress = static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS));
  pUiProgress->open(true);
  pUiProgress->setCaption(L"Change backup folder");
  pUiProgress->setScHeadText(L"Transferring backup data");

  DWORD dwid;
  this->_movBck_hth = CreateThread(nullptr, 0, this->_movBck_fth, this, 0, &dwid);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChn::_movBck_stop()
{
  DWORD exitCode;

  if(this->_movBck_hth) {
    WaitForSingleObject(this->_movBck_hth, INFINITE);
    GetExitCodeThread(this->_movBck_hth, &exitCode);
    CloseHandle(this->_movBck_hth);
    this->_movBck_hth = nullptr;
  }

  // Back to main dialog window to normal state
  static_cast<OmUiMgr*>(this->root())->safemode(false);

  // Close progress dialog
  static_cast<OmUiProgress*>(this->childById(IDD_PROGRESS))->quit();

  if(exitCode == 1)  {
    // an error occurred during backup purge
    Om_dlgBox_ok(this->_hwnd, L"Channel properties", IDI_WRN,
                 L"Channel backup transfer error", L"Channel "
                 "backup data transfer encountered error(s), some backup data may "
                 "had not properly moved. Please read debug log for details.");
  }

  OmUiPropChnStg* pUiPropLocStg = static_cast<OmUiPropChnStg*>(this->childById(IDD_PROP_CHN_STG));

  // Reset parameter as unmodified
  pUiPropLocStg->setChParam(CHN_PROP_STG_BACKUP, false);

  // modify the backup path for the Mod Channel
  if(pUiPropLocStg->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) { // custom backup checked
    this->_pChn->setCustBckDir(this->_movBck_dest);
  } else {
    this->_pChn->remCustBckDir();
  }

  // Call apply again in case it still changes to be applied
  this->applyChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
DWORD WINAPI OmUiPropChn::_movBck_fth(void* arg)
{
  OmUiPropChn* self = static_cast<OmUiPropChn*>(arg);

  OmModChan* pChn = self->_pChn;
  if(!pChn) return 1;

  DWORD exitCode = 0;

  if(!pChn->bckMove(self->_movBck_dest, &self->_movBck_progress_cb, self->childById(IDD_PROGRESS))) {
    exitCode = 1;
  }

  // send message to window, to proper quit dialog and finish
  PostMessage(self->_hwnd, UWM_MOVEBACKUP_DONE, 0, 0);

  return exitCode;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropChn::_movBck_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data)
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
void OmUiPropChn::_onPropInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChn::_onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // UWM_MOVEBACKUP_DONE is a custom message sent from backup transfer thread
  // function, to notify the progress dialog ended is job.
  if(uMsg == UWM_MOVEBACKUP_DONE) {
    // end the backup transfer process
    this->_movBck_stop();
  }

  return false;
}
