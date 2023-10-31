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
#include "OmContext.h"

#include "OmUiWizCtxBeg.h"
#include "OmUiWizCtxCfg.h"
#include "OmUiWizCtxChn.h"
#include "OmUiMgr.h"

#include "OmUtilDlg.h"
#include "OmUtilWin.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiWizCtx.h"

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtx::OmUiWizCtx(HINSTANCE hins) : OmDialogWiz(hins)
{
  // create wizard pages
  this->_addPage(new OmUiWizCtxBeg(hins));
  this->_addPage(new OmUiWizCtxCfg(hins));
  this->_addPage(new OmUiWizCtxChn(hins));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiWizCtx::~OmUiWizCtx()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiWizCtx::id() const
{
  return IDD_WIZ_CTX;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmUiWizCtx::_onWizNext()
{
  switch(this->_currPage)
  {
  case 1: // Context parameters Wizard page
    return static_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG))->hasValidParams();
    break;
  case 2: // Mod Channel parameters Wizard page
    return static_cast<OmUiWizCtxChn*>(this->childById(IDD_WIZ_CTX_CHN))->hasValidParams();
    break;
  default:
    return true;
    break;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtx::_onWizFinish()
{
  OmManager* pMgr = static_cast<OmManager*>(this->_data);
  OmUiWizCtxCfg* pUiWizCtxCfg = static_cast<OmUiWizCtxCfg*>(this->childById(IDD_WIZ_CTX_CFG));
  OmUiWizCtxChn* pUiWizLocCfg = static_cast<OmUiWizCtxChn*>(this->childById(IDD_WIZ_CTX_CHN));

  // Retrieve Context parameters
  wstring ctx_name, ctx_home;
  pUiWizCtxCfg->getItemText(IDC_EC_INP01, ctx_name);
  pUiWizCtxCfg->getItemText(IDC_EC_INP02, ctx_home);

  // Retrieve Mod Channel parameters
  wstring chn_name, chn_dst, chn_lib, chn_bck;
  pUiWizLocCfg->getItemText(IDC_EC_INP01, chn_name);
  pUiWizLocCfg->getItemText(IDC_EC_INP02, chn_dst);

  if(pUiWizLocCfg->msgItem(IDC_BC_CKBX1, BM_GETCHECK)) {
    pUiWizLocCfg->getItemText(IDC_EC_INP03, chn_lib);
  }

  if(pUiWizLocCfg->msgItem(IDC_BC_CKBX2, BM_GETCHECK)) {
    pUiWizLocCfg->getItemText(IDC_EC_INP04, chn_bck);
  }

  this->quit();

  // create the new Context, if an error occur, error message
  if(pMgr->ctxNew(ctx_name, ctx_home, true)) {
    // get current selected Context (the just created one)
    OmContext* pCtx = pMgr->ctxCur();
    // create new Mod Channel in Context
    if(!pCtx->chnAdd(chn_name, chn_dst, chn_lib, chn_bck)) {
      Om_dlgBox_okl(this->_hwnd, L"Mod Hub Wizard", IDI_ERR,
                    L"Mod Channel creation error", L"Mod Channel "
                    "creation failed because of the following error:",
                    pCtx->lastError());
    }
  } else {
    Om_dlgBox_okl(this->_hwnd, L"Mod Hub Wizard", IDI_ERR,
                  L"Mod Hub creation error", L"Mod Hub "
                  "creation failed because of the following error:",
                  pMgr->lastError());
  }

  // force parent dialog to refresh
  this->root()->refresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiWizCtx::_onWizInit()
{
  // set dialog icon
  this->setIcon(Om_getResIcon(this->_hins, IDI_APP, 2), Om_getResIcon(this->_hins, IDI_APP, 1));
}
