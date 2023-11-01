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
#include "OmUiPropChn.h"
#include "OmUiAddBat.h"
#include "OmUiPropBat.h"
#include "OmUiPropHubStg.h"
#include "OmUiPropHubChn.h"
#include "OmUiPropHubBat.h"
#include "OmUiProgress.h"

#include "OmUtilDlg.h"
#include "OmUtilStr.h"         //< Om_isValidPath
#include "OmUtilWin.h"         //< Om_getResIcon

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropHub.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHub::OmUiPropHub(HINSTANCE hins) : OmDialogProp(hins),
  _modHub(nullptr)
{
  // create child tab dialogs
  this->_addPage(L"Settings", new OmUiPropHubStg(hins));
  this->_addPage(L"Channels", new OmUiPropHubChn(hins));
  this->_addPage(L"Scripts", new OmUiPropHubBat(hins));

  this->addChild(new OmUiAddChn(hins));     //< Dialog for Mod Channel creation
  this->addChild(new OmUiAddBat(hins));     //< Dialog for Batch creation
  this->addChild(new OmUiPropChn(hins));    //< Dialog for Mod Channel properties
  this->addChild(new OmUiPropBat(hins));    //< Dialog for Mod Channel properties
  this->addChild(new OmUiProgress(hins));   //< for Mod Channel backup cleaning
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropHub::~OmUiPropHub()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropHub::id() const
{
  return IDD_PROP_CTX;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHub::checkChanges()
{
  if(!this->_modHub)
    return false;

  OmUiPropHubStg* pUiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropHubChn* pUiPropHubLoc  = static_cast<OmUiPropHubChn*>(this->childById(IDD_PROP_CTX_CHN));
  OmUiPropHubBat* pUiPropHubBat  = static_cast<OmUiPropHubBat*>(this->childById(IDD_PROP_CTX_BAT));

  bool changed = false;

  wstring item_str;

  if(pUiPropHubStg->hasChParam(CTX_PROP_STG_TITLE)) {  //< parameter for Mod Hub title
    pUiPropHubStg->getItemText(IDC_EC_INP03, item_str);
    if(this->_modHub->title() != item_str) {
      changed = true;
    } else {
      pUiPropHubStg->setChParam(CTX_PROP_STG_TITLE, false);
    }
  }

  if(pUiPropHubStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Mod Hub icon
    changed = true;
  }

  if(pUiPropHubLoc->hasChParam(CTX_PROP_CHN_ORDER)) { // parameter for Mod Channel index order
    changed = true;
  }

  if(pUiPropHubBat->hasChParam(CTX_PROP_BAT_ORDER)) { // parameter for Batches index order
    changed = true;
  }

  if(pUiPropHubBat->hasChParam(CTX_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode

    bool bm_chk = pUiPropHubBat->msgItem(IDC_BC_CKBX1, BM_GETCHECK);

    changed = bm_chk != this->_modHub->batQuietMode();
  }

  // enable Apply button
  this->enableItem(IDC_BC_APPLY, changed);

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiPropHub::applyChanges()
{
  if(!this->_modHub)
    return false;

  OmUiPropHubStg* pUiPropHubStg  = static_cast<OmUiPropHubStg*>(this->childById(IDD_PROP_CTX_STG));
  OmUiPropHubChn* pUiPropHubLoc  = static_cast<OmUiPropHubChn*>(this->childById(IDD_PROP_CTX_CHN));
  OmUiPropHubBat* pUiPropHubBat  = static_cast<OmUiPropHubBat*>(this->childById(IDD_PROP_CTX_BAT));

  wstring ctx_name, ico_path;

  // Step 1, verify everything
  if(pUiPropHubStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Mod Hub title
    pUiPropHubStg->getItemText(IDC_EC_INP03, ctx_name);
    if(!Om_dlgValidName(this->_hwnd, L"Mod Hub name", ctx_name))
      return false;
  }

  // Step 2, save changes
  if(pUiPropHubStg->hasChParam(CTX_PROP_STG_TITLE)) { //< parameter for Mod Hub title
    this->_modHub->setTitle(ctx_name);
    // Reset parameter as unmodified
    pUiPropHubStg->setChParam(CTX_PROP_STG_TITLE, false);
  }

  if(pUiPropHubStg->hasChParam(CTX_PROP_STG_ICON)) { // parameter for Mod Hub icon

    pUiPropHubStg->getItemText(IDC_EC_INP04, ico_path);

    if(Om_isValidPath(ico_path)) {
      this->_modHub->setIcon(ico_path);
    } else {
      this->_modHub->setIcon(L""); //< remove current icon
    }
    // Reset parameter as unmodified
    pUiPropHubStg->setChParam(CTX_PROP_STG_ICON, false);
  }

  if(pUiPropHubLoc->hasChParam(CTX_PROP_CHN_ORDER)) { // parameter for Mod Channel index order

    // To prevent inconsistency we unselect location in the main dialog
    static_cast<OmUiMgr*>(this->root())->safemode(true);

    unsigned n = pUiPropHubLoc->msgItem(IDC_LB_CHN, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Mod Channel according current List-Box order
      this->_modHub->modChanGet(pUiPropHubLoc->msgItem(IDC_LB_CHN, LB_GETITEMDATA, i))->setIndex(i);
    }

    // unselect Mod Channel in context
    this->_modHub->modChanSelect(-1);
    // sort Mod Channel list
    this->_modHub->modChanSort();
    // select the first location in list
    this->_modHub->modChanSelect(0);

    // restore main dialog to normal state
    static_cast<OmUiMgr*>(this->root())->safemode(false);

    // Reset parameter as unmodified
    pUiPropHubLoc->setChParam(CTX_PROP_CHN_ORDER, false);
  }

  if(pUiPropHubBat->hasChParam(CTX_PROP_BAT_ORDER)) { // parameter for Batches index order

    unsigned n = pUiPropHubBat->msgItem(IDC_LB_BAT, LB_GETCOUNT);
    for(unsigned i = 0; i < n; ++i) {
      // set new index number of Mod Channel according current List-Box order
      this->_modHub->batGet(pUiPropHubBat->msgItem(IDC_LB_BAT, LB_GETITEMDATA,i))->setIndex(i);
    }

    // sort Mod Channel list
    this->_modHub->batSort();

    // Reset parameter as unmodified
    pUiPropHubBat->setChParam(CTX_PROP_CHN_ORDER, false);
  }

  if(pUiPropHubBat->hasChParam(CTX_PROP_BAT_QUIETMODE)) { // parameter for Batch Quiet Mode

    this->_modHub->setBatQuietMode(pUiPropHubBat->msgItem(IDC_BC_CKBX1, BM_GETCHECK));

    // Reset parameter as unmodified
    pUiPropHubBat->setChParam(CTX_PROP_BAT_QUIETMODE, false);
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
void OmUiPropHub::_onPropInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}
