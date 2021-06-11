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
#ifndef OMUITOOLPKG_H
#define OMUITOOLPKG_H

#include "OmDialog.h"
#include "OmImage.h"

/// \brief Package Editor tool dialog
///
/// OmDialog class derived for Package Editor tool dialog window
///
class OmUiToolPkg : public OmDialog
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    /// \param[in]  hins    : API Instance handle.
    ///
    OmUiToolPkg(HINSTANCE hins);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmUiToolPkg();

    /// \brief Get resource id.
    ///
    /// Returns dialog window resource id.
    ///
    /// \return dialog resource id.
    ///
    long id() const;

  protected:

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    HFONT               _hFtMonos;

    HBITMAP             _hBmBlank;

    HBITMAP             _hBmBcNew;

    HBITMAP             _hBmBcDel;

    OmImage             _image;

    bool                _parsePkg(const wstring& path);

    void                _resetPkg();

    void                _parseName();

    void                _buildPkg_init();

    void                _buildPkg_stop();

    void*               _buildPkg_hth;

    static DWORD WINAPI _buildPkg_fth(void*);

    bool                _abort;

    bool                _apply();

    void                _onInit();

    void                _onResize();

    void                _onRefresh();

    void                _onQuit();

    bool                _onMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // OMUITOOLPKG_H
