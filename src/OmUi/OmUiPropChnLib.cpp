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

#include "OmUiPropChn.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmUiPropChnLib.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnLib::OmUiPropChnLib(HINSTANCE hins) : OmDialog(hins)
{
  // modified parameters flags
  for(unsigned i = 0; i < 8; ++i)
    this->_chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmUiPropChnLib::~OmUiPropChnLib()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
long OmUiPropChnLib::id() const
{
  return IDD_PROP_CHN_LIB;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::setChParam(unsigned i, bool en)
{
  _chParam[i] = en;
  static_cast<OmDialogProp*>(this->_parent)->checkChanges();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onCkBoxDev()
{
  // user modified parameter, notify it
  this->setChParam(CHN_PROP_LIB_DEVMODE, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onCkBoxWrn()
{
  // user modified parameter, notify it
  this->setChParam(CHN_PROP_LIB_WARNINGS, true);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onCkBoxHid()
{
  // user modified parameter, notify it
  this->setChParam(CHN_PROP_LIB_SHOWHIDDEN, true);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onInit()
{
  // define controls tool-tips
  this->_createTooltip(IDC_BC_CKBX1,  L"Parse library subfolders as packages source for mod development purpose or legacy support");

  this->_createTooltip(IDC_BC_CKBX2,  L"Warn when packages install will overlap any previously installed");
  this->_createTooltip(IDC_BC_CKBX3,  L"Warn when packages install require additional dependencies installation");
  this->_createTooltip(IDC_BC_CKBX4,  L"Warn when packages dependencies are missing");
  this->_createTooltip(IDC_BC_CKBX5,  L"Warn when packages uninstall require additional uninstallations");
  this->_createTooltip(IDC_BC_CKBX6,  L"Parse and show library hidden files and subfolders");

  OmModChan* pModChan = static_cast<OmUiPropChn*>(this->_parent)->modChan();
  if(!pModChan) return;

  this->msgItem(IDC_BC_CKBX1, BM_SETCHECK, pModChan->libDevMode());
  this->msgItem(IDC_BC_CKBX2, BM_SETCHECK, pModChan->warnOverlaps());
  this->msgItem(IDC_BC_CKBX3, BM_SETCHECK, pModChan->warnExtraInst());
  this->msgItem(IDC_BC_CKBX4, BM_SETCHECK, pModChan->warnMissDeps());
  this->msgItem(IDC_BC_CKBX5, BM_SETCHECK, pModChan->warnExtraUnin());
  this->msgItem(IDC_BC_CKBX6, BM_SETCHECK, pModChan->libShowHidden());

  // reset modified parameters flags
  for(unsigned i = 0; i < 8; ++i) _chParam[i] = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmUiPropChnLib::_onResize()
{
  // Library Dev Mode CheckBox
  this->_setItemPos(IDC_BC_CKBX1, 50, 20, 200, 9);

  // Enable warnings Label
  this->_setItemPos(IDC_SC_LBL01, 50, 45, 200, 9);

  // Enable warnings CheckBoxes
  this->_setItemPos(IDC_BC_CKBX2, 65, 55, 200, 9);
  this->_setItemPos(IDC_BC_CKBX3, 65, 65, 200, 9);
  this->_setItemPos(IDC_BC_CKBX4, 65, 75, 200, 9);
  this->_setItemPos(IDC_BC_CKBX5, 65, 85, 200, 9);

  // Show Hidden CheckBox
  this->_setItemPos(IDC_BC_CKBX6, 50, 110, 200, 9);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
INT_PTR OmUiPropChnLib::_onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if(uMsg == WM_COMMAND) {

    switch(LOWORD(wParam))
    {
    case IDC_BC_CKBX1: //< Check box for Package legacy (folders) support
      this->_onCkBoxDev();
      break;

    case IDC_BC_CKBX2: //< Check box for Warn at Installation overlaps
    case IDC_BC_CKBX3: //< Check box for Warn at Additional installation due to dependencies
    case IDC_BC_CKBX4: //< Check box for Warn at Installation dependencies missing
    case IDC_BC_CKBX5: //< Check box for Warn at Additional restoration due to overlaps
      this->_onCkBoxWrn();
      break;

    case IDC_BC_CKBX6: //< Check box for Warn at Additional restoration due to overlaps
      this->_onCkBoxHid();
      break;
    }
  }

  return false;
}
