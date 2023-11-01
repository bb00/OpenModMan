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

#ifndef OMMODHUB_H
#define OMMODHUB_H

#include "OmBase.h"
#include "OmBaseWin.h"

#include "OmConfig.h"
#include "OmModChan.h"
#include "OmBatch.h"

class OmManager;

/// \brief Mod Hub object.
///
/// The Mod Hub object describe a global environment for package management.
///
class OmModHub
{
  friend class OmModChan;
  friend class Package;

  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmModHub(OmManager* pMgr);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmModHub();

    /// \brief Open Mod Hub.
    ///
    /// Load Mod Hub from specified file.
    ///
    /// \param[in]  path    : File path of Mod Hub to be loaded.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool open(const wstring& path);

    /// \brief Close Mod Hub.
    ///
    /// Close and empty the current instance.
    ///
    void close();

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Check whether is valid.
    ///
    /// Checks whether this instance is correctly loaded a ready to use.
    ///
    /// \return True if this instance is valid, false otherwise.
    ///
    bool isValid() const {
      return _valid;
    }

    /// \brief Get Mod Hub file path.
    ///
    /// Returns Mod Hub file path.
    ///
    /// \return Mod Hub file path.
    ///
    const wstring& path() const {
      return _path;
    }

    /// \brief Get Mod Hub UUID.
    ///
    /// Returns Mod Hub UUID.
    ///
    /// \return Mod Hub UUID.
    ///
    const wstring& uuid() const {
      return _uuid;
    }

    /// \brief Get Mod Hub title.
    ///
    /// Returns Mod Hub title.
    ///
    /// \return Mod Hub title.
    ///
    const wstring& title() const {
      return _title;
    }

    /// \brief Get Mod Hub home directory.
    ///
    /// Returns home directory.
    ///
    /// \return Mod Hub home directory.
    ///
    const wstring& home() const {
      return _home;
    }

    /// \brief Get Mod Hub icon.
    ///
    /// Returns Mod Hub icon as icon handle.
    ///
    /// \return Banner bitmap handle.
    ///
    const HICON& icon() const {
      return _icon;
    }

    /// \brief Set Mod Hub title.
    ///
    /// Defines and save Mod Hub title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const wstring& title);

    /// \brief Set Mod Hub icon.
    ///
    /// Defines the Mod Hub icon source file. This must be a valid path to
    /// an icon or executable file or empty string to remove current setting.
    ///
    /// \param[in]  src     : Path to file to extract or empty
    ///                       string to remove current.
    ///
    void setIcon(const wstring& src);

    /// \brief Make new Mod Channel.
    ///
    /// Creates a new Mod Channel within the Mod Hub.
    ///
    /// \param[in]  title     : Title of new Mod Channel to be created.
    /// \param[in]  install   : Package installation destination folder path.
    /// \param[in]  library   : Custom package Library folder path.
    /// \param[in]  backup    : Custom package Backup folder path.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool modChanCreate(const wstring& title, const wstring& install, const wstring& library, const wstring& backup);

    /// \brief Purge existing Mod Channel.
    ///
    /// Cleanup and removes a Mod Channel. Notice that this operation actually delete
    /// the Mod Channel folder and configuration files.
    ///
    /// \param[in]  i           : Mod Channel index to be removed.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool modChanDelete(unsigned i);

    /// \brief Get Mod Channel count.
    ///
    /// Returns count of Mod Channel defined in the Mod Hub.
    ///
    /// \return Mod Channel count.
    ///
    size_t modChanCount() {
      return _modChanLs.size();
    }

    /// \brief Get Mod Channel.
    ///
    /// Returns Mod Channel at index.
    ///
    /// \param[in]  i      : Mod Channel index.
    ///
    /// \return Mod Channel object at index or nullptr if index is out of bound.
    ///
    OmModChan* modChanGet(unsigned i) {
      return (i < _modChanLs.size()) ? _modChanLs[i] : nullptr;
    }

    /// \brief Get Mod Channel.
    ///
    /// Returns Mod Channel with specified UUID.
    ///
    /// \param[in]  uuid     : Mod Channel UUID to search.
    ///
    /// \return Mod Channel object or nullptr if not found.
    ///
    OmModChan* modChanGet(const wstring& uuid);

    /// \brief Get Mod Channel index.
    ///
    /// Returns the index of the Mod Channel that matches the specified UUID.
    ///
    /// \param[in]  uuid     : Mod Channel UUID to search.
    ///
    /// \return Mod Channel index or -1 if not found.
    ///
    int modChanIdx(const wstring& uuid);

    /// \brief Sort Mod Channel list.
    ///
    /// Sort Mod Channel list according Mod Channel ordering index.
    ///
    void modChanSort();

    /// \brief Select Mod Channel.
    ///
    /// Sets the specified Mod Channel as active one.
    ///
    /// \param[in]  i       : Mod Channel index or -1 to unselect.
    ///
    /// \return True if operation succeed, false if id is out of bound.
    ///
    bool modChanSelect(int i);

    /// \brief Select Mod Channel.
    ///
    /// Sets the specified Mod Channel as active one.
    ///
    /// \param[in]  uuid    : Mod Channel UUID to select.
    ///
    /// \return True if operation succeed, false if Mod Channel with such UUID does not exists.
    ///
    bool modChanSelect(const wstring& uuid);

    /// \brief Get active Mod Channel.
    ///
    /// Returns current active Mod Channel.
    ///
    /// \return Current active Mod Channel or nullptr if none is active.
    ///
    OmModChan* modChanCur() {
      return _modChanSl >= 0 ? _modChanLs[_modChanSl] : nullptr;
    }

    /// \brief Get active Mod Channel index.
    ///
    /// Returns index of the current active Mod Channel.
    ///
    /// \return Index of the active Mod Channel.
    ///
    int modChanCurIdx() const {
      return _modChanSl;
    }

    /// \brief Get Mod Hub Batch Scripts.
    ///
    /// Returns count of Scripts defined in the Mod Hub.
    ///
    /// \return Scripts count.
    ///
    size_t batCount() {
      return _batLs.size();
    }

    /// \brief Get Script.
    ///
    /// Returns Batch at index.
    ///
    /// \param[in]  i      : Batch index to get.
    ///
    /// \return Batch object at index.
    ///
    OmBatch* batGet(unsigned i) {
      return (i < _batLs.size()) ? _batLs[i] : nullptr;
    }

    /// \brief Sort Batches list.
    ///
    /// Sort Batches list according Mod Channel ordering index.
    ///
    void batSort();

    /// \brief Make new Batch.
    ///
    /// Creates a new Batch within the Mod Hub.
    ///
    /// \param[in]  title         : Title of new Batch to be created.
    ///
    /// \return Newly created batch object
    ///
    OmBatch* batAdd(const wstring& title);

    /// \brief Delete Batch.
    ///
    /// Delete batch definition file and remove it from list.
    ///
    /// \param[in]  i     : Batch index to delete.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool batRem(unsigned i);

    /// \brief Get batches warning quiet mode.
    ///
    /// Returns batches warning quiet mode option value.
    ///
    /// \return warning quiet mode option value.
    ///
    bool batQuietMode() const {
      return _batQuietMode;
    }

    /// \brief Set batches warning quiet mode.
    ///
    /// Define and save batches warning quiet mode option value.
    ///
    /// \param[in]  enable    : Warning quiet mode enable or disable.
    ///
    void setBatQuietMode(bool enable);

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///              - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmManager*          _manager;

    OmConfig            _config;

    wstring             _path;

    wstring             _uuid;

    wstring             _title;

    wstring             _home;

    HICON               _icon;

    vector<OmModChan*>  _modChanLs;

    int                 _modChanSl;

    vector<OmBatch*>    _batLs;

    bool                _batQuietMode;

    bool                _valid;

    wstring             _error;

    bool                _migrate(const wstring& path);
};

#endif // OMMODHUB_H
