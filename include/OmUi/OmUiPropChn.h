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
#ifndef OMUIPROPCHN_H
#define OMUIPROPCHN_H

#include "OmDialogProp.h"

class OmModChan;

/// \brief Mod Channel Properties parent dialog
///
/// OmDialogProp class derived for Mod Channel Properties parent dialog window.
///
class OmUiPropChn : public OmDialogProp
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiPropChn(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiPropChn();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

    /// \brief Get associated Mod Channel.
    ///
    /// Returns associated Mod Channel object
    ///
    /// \return Associated Mod Channel or nullptr if none.
    ///
    OmModChan* chnCur() const {
      return _pChn;
    }

    /// \brief Set associated Mod Channel.
    ///
    /// Define the associated Mod Channel, which the dialog is intended to
    /// work with and on. A valid Mod Channel must be set in order before
    /// opening the dialog.
    ///
    /// \param[in]  pChn  : Mod Channel object to associate.
    ///
    void chnSet(OmModChan* pChn) {
      _pChn = pChn;
    }

    /// \brief Check for properties changes
    ///
    /// Checks whether dialog's dedicated properties changed then perform
    /// proper operations, such as enabling or disabling the Apply button.
    ///
    bool checkChanges();

    /// \brief Apply properties changes
    ///
    /// Retrieve dialog's dedicated properties then apply changes.
    ///
    bool applyChanges();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModChan*         _pChn;

    void*               _movBck_hth;

    wstring             _movBck_dest;

    void                _movBck_init(const wstring& dest);

    void                _movBck_stop();

    static DWORD WINAPI _movBck_fth(void*);

    static bool         _movBck_progress_cb(void* ptr, size_t tot, size_t cur, uint64_t data);

    void                _onPropInit();

    INT_PTR             _onPropMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUIPROPCHN_H
