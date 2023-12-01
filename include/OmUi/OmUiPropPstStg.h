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
#ifndef OMUIPROPPSTSTG_H
#define OMUIPROPPSTSTG_H

#include "OmDialogPropTab.h"

#define PST_PROP_STG_TITLE  0
#define PST_PROP_STG_IONLY  1

/// \brief Mod Preset Properties / Setting tab child
///
/// OmDialog class derived for Mod Preset Properties / Settings tab child dialog window
///
class OmUiPropPstStg : public OmDialogPropTab
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropPstStg(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropPstStg();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void                _onCkBoxIonly();

    void                _onTabInit();

    void                _onTabResize();

    void                _onTabRefresh();

    INT_PTR             _onTabMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPPSTSTG_H
