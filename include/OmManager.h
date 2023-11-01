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
#ifndef OMMANAGER_H
#define OMMANAGER_H

#include "OmBase.h"

#include "OmConfig.h"
#include "OmModHub.h"

// maximum count of recent file path to store
#define OM_MANAGER_MAX_RECENT     10

/// \brief Main manager application object.
///
/// This is the main "back end" application entry point object.
///
class OmManager
{
  public:

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmManager();

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmManager();

    /// \brief Get last error string.
    ///
    /// Returns last error message string.
    ///
    /// \return Last error message string.
    ///
    const wstring& lastError() const {
      return _error;
    }

    /// \brief Get log string.
    ///
    /// Returns log string.
    ///
    /// \return Log string.
    ///
    const wstring& getLog() const {
      return _log;
    }

    /// \brief Initialize application.
    ///
    /// Initializes the application, load or create initial configuration.
    ///
    /// \param[in]  hwnd  : Main Windows handle.
    /// \param[in]  arg   : Optional command line arguments string.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool init(const char* arg = nullptr);

    /// \brief Quit application.
    ///
    /// Clean the instance and quit safely.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool quit();

    /// \brief Create new Mod Hub.
    ///
    /// Create a new Mod Hub configuration in the specified path.
    ///
    /// \param[in]  title     : New Mod Hub title.
    /// \param[in]  path      : Path where to create Mod Hub.
    /// \param[in]  open      : Open newly created Mod Hub after creation.
    ///
    bool modHubCreate(const wstring& title, const wstring& path, bool open = true);

    /// \brief Open Mod Hub.
    ///
    /// Open a Mod Hub configuration at the specified path.
    ///
    /// \param[in]  path      : Mod Hub file path.
    /// \param[in]  select    : Select the opened Mod Hub.
    ///
    bool modHubLoad(const wstring& path, bool select = true);

    /// \brief Close current Mod Hub.
    ///
    /// Close Mod Hub.
    ///
    /// \param[in]  i         : Mod Hub index to close, or -1 to close current selected.
    ///
    void modHubClose(int i = -1);

    /// \brief Get Mod Hub count.
    ///
    /// Returns loaded Mod Hub count.
    ///
    /// \return Loaded Mod Hub count.
    ///
    size_t modHubCount() const {
      return _modHubLs.size();
    }

    /// \brief Get Mod Hub.
    ///
    /// Returns loaded Mod Hub instance at index.
    ///
    /// \param[in]  i     : Mod Hub index.
    ///
    /// \return Loaded Mod Hub instance.
    ///
    OmModHub* modHubGet(unsigned i) {
      return _modHubLs[i];
    }

    /// \brief Set selected Mod Hub.
    ///
    /// Select the specified Mod Hub at index. The selected Mod Hub will be
    /// available by via the OmManager.ctxCur Mod Hub.
    ///
    /// \param[in]  i     : Index of Mod Hub to select or -1 to unselect.
    ///
    void modHubSel(int i);

    /// \brief Get active Mod Hub.
    ///
    /// Returns current active Mod Hub.
    ///
    /// \return Current active Mod Hub.
    ///
    OmModHub* modHubCur() const {
      return _modHubSl >= 0 ? _modHubLs[_modHubSl] : nullptr;
    }

    /// \brief Get active Mod Hub index.
    ///
    /// Returns current active Mod Hub index.
    ///
    /// \return Current active Mod Hub index.
    ///
    int modHubCurIdx() const {
      return _modHubSl;
    }

    /// \brief Get active Mod Channel from active Mod Hub.
    ///
    /// Returns current active Mod Channel from active Mod Hub.
    ///
    /// \return Current active Mod Channel or null.
    ///
    OmModChan* modChanCur() const {
      return _modHubSl >= 0 ? _modHubLs[_modHubSl]->modChanCur() : nullptr;
    }

    /// \brief Save configuration window RECT.
    ///
    /// Stores the specified window RECT to configuration file.
    ///
    /// \param[in]  rect    : Window RECT to save.
    ///
    void saveWindowRect(const RECT& rect);

    /// \brief Load configuration window RECT.
    ///
    /// Retrieve the stored window RECT from configuration file.
    ///
    /// \param[out]  rect   : RECT reference to get saved data.
    ///
    void loadWindowRect(RECT& rect);

    /// \brief Save configuration window foot height.
    ///
    /// Stores the specified window foot frame height to configuration file.
    ///
    /// \param[in]  height  : Foot height to save.
    ///
    void saveWindowFoot(int height);

    /// \brief Load configuration window foot height.
    ///
    /// Retrieve the stored window foot frame height to configuration file.
    ///
    /// \param[out]  height : Foot height to save.
    ///
    void loadWindowFoot(int* height);

    /// \brief Save configuration recent file.
    ///
    /// Stores a new recent file path to configuration file.
    ///
    /// \param[in]  path    : Path to file.
    ///
    void saveRecentFile(const wstring& path);

    /// \brief Load configuration recent files list.
    ///
    /// Retrieve recent files path list from configuration file.
    ///
    /// \param[in]  paths   : Reference to wide string array to get list.
    ///
    void loadRecentFiles(vector<wstring>& paths);

    /// \brief Clear configuration recent files list.
    ///
    /// Deletes all recent files path list from configuration file.
    ///
    void clearRecentFiles();

    /// \brief Save default working location.
    ///
    /// Saves the default working location path in configuration file.
    ///
    /// \param[in]  path    : last used Mod Hub path.
    ///
    void saveDefaultLocation(const wstring& path);

    /// \brief Get default working location
    ///
    /// Retrieve best default save location path, according last created
    /// Mod Hub and or environment variables.
    ///
    /// \param[in]  path    : Reference to wide string array to get path.
    ///
    void getDefaultLocation(wstring& path);

    /// \brief Save configuration startup Mod Hub.
    ///
    /// Stores startup Mod Hub path.
    ///
    /// \param[in]  enable  : enable auto-load.
    /// \param[in]  path    : startup Mod Hub(s) file path list.
    ///
    void saveStartHubs(bool enable, const vector<wstring>& path);

    /// \brief Load configuration startup Mod Hub(s) path.
    ///
    /// Retrieve path to Mod Hub(s) to be loaded at application start.
    ///
    /// \param[in]  path    : Pointer to boolean to get auto-load.
    /// \param[in]  path    : Reference to wide string to get path.
    ///
    void getStartHubs(bool* enable, vector<wstring>& path);

    /// \brief Get icons size option.
    ///
    /// Returns icons size option value.
    ///
    /// \return Icons size in pixels
    ///
    unsigned iconsSize() const {
      return _iconsSize;
    }

    /// \brief Set icons size option.
    ///
    /// Define and save icons size option value.
    ///
    /// \param[in]  size    : Icons size in pixels
    ///
    void setIconsSize(unsigned size);

    /// \brief Get no-markdown option.
    ///
    /// Returns no-markdown option value.
    ///
    /// \return True if enabled, false otherwise.
    ///
    bool noMarkdown() const {
      return _noMarkdown;
    }

    /// \brief Set no-markdown option.
    ///
    /// Define and save no-markdown option value.
    ///
    /// \param[in]  enable  : Boolean value to set.
    ///
    void setNoMarkdown(bool enable);

    /// \brief Set log output.
    ///
    /// Defines an Edit Control to output log in real time.
    ///
    /// \param[in]  hwnd      : Windows handle (HWND) to Edit Control.
    ///
    void setLogOutput(HWND hWnd);

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    wstring             _home;

    OmConfig            _config;

    vector<OmModHub*>   _modHubLs;

    int                 _modHubSl;

    // general options
    unsigned            _iconsSize;

    bool                _folderPackages;

    bool                _warnEnabled;

    bool                _warnOverlaps;

    bool                _warnExtraInstall;

    bool                _warnMissingDepend;

    bool                _warnExtraUninst;

    bool                _quietBatches;

    bool                _noMarkdown;


    wstring             _error;

    wstring             _log;

    void*               _logHwnd;

    void*               _logFile;

    bool                _migrate();

};

#endif // OMMANAGER_H
