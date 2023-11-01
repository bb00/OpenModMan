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

#ifndef OMBATCH_H
#define OMBATCH_H

#include "OmBase.h"
#include "OmConfig.h"

class OmModHub;
class OmModChan;
class OmPackage;

/// \brief Batch object.
///
/// Object for Package installation batch
///
class OmBatch
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmBatch();

    /// \brief Constructor.
    ///
    /// Constructor with Mod Hub.
    ///
    /// \param[in]  pModHub  : Related Mod Hub for the Batch.
    ///
    OmBatch(OmModHub* pModHub);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmBatch();

    /// \brief Parse Batch definition file.
    ///
    /// Analyses the supplied file to check whether this is a valid
    /// Batch definition, and if yes, parse its content.
    ///
    /// \param[in]  path    : Path to definition file to parse.
    ///
    /// \return True if supplied file is a valid Batch and operation succeed,
    /// false otherwise.
    ///
    bool open(const wstring& path);

    /// \brief Initialize Batch definition file.
    ///
    /// Create new Batch definition file at given path.
    ///
    /// \param[in]  path    : Path to definition file to create.
    /// \param[in]  title   : Installation Batch title.
    /// \param[in]  index   : Initial list index to set.
    ///
    /// \return True if operation succeed, false otherwise
    ///
    bool init(const wstring& path, const wstring& title, unsigned index = 0);

    /// \brief Get Batch uuid.
    ///
    /// Returns Batch uuid.
    ///
    /// \return Batch uuid.
    ///
    const wstring& uuid() const {
      return _uuid;
    }

    /// \brief Get Batch title.
    ///
    /// Returns Batch title.
    ///
    /// \return Batch title.
    ///
    const wstring& title() const {
      return _title;
    }

    /// \brief Get Batch index.
    ///
    /// Returns Batch ordering index.
    ///
    /// \return Ordering index number.
    ///
    unsigned index() const {
      return _index;
    }

    /// \brief Get Batch definition file path.
    ///
    /// Returns Batch definition file path.
    ///
    /// \return Batch definition file path.
    ///
    const wstring& path() const {
      return _path;
    }

    /// \brief Get Install-Only option.
    ///
    /// Returns the Install-Only option for this instance
    ///
    /// \return Install-Only option value.
    ///
    bool installOnly() {
      return _instOnly;
    }

    /// \brief Set Batch title.
    ///
    /// Defines and save Batch title.
    ///
    /// \param[in]  title   : Title to defines and save
    ///
    void setTitle(const wstring& title);

    /// \brief Set Batch index.
    ///
    /// Defines and save Batch index for ordering.
    ///
    /// \param[in]  index   : Index number to defines and save
    ///
    void setIndex(unsigned index);

    /// \brief Set Install-Only option.
    ///
    /// Defines the Install-Only option for this instance
    ///
    /// \param[in]  enable  : Value to set.
    ///
    void setInstallOnly(bool enable);

    /// \brief Get Mod Channel count
    ///
    /// Returns count of referenced Mod Channel in this instance.
    ///
    /// \return Count of referenced Mod Channel
    ///
    size_t modChanCount();

    /// \brief Get Mod Channel UUID
    ///
    /// Returns referenced Mod Channel UUID at specified index.
    ///
    /// \param[in]  i    : Mod Channel index.
    ///
    /// \return Mod Channel's UUID.
    ///
    wstring chnUuid(unsigned i);

    /// \brief Discard Mod Channel
    ///
    /// Removes reference and install list of the  Mod Channel
    /// with the specified UUID.
    ///
    /// \param[in]  uuid : Mod Channel UUID.
    ///
    /// \return True if operation succeed, false if reference not found.
    ///
    bool chnDiscard(const wstring& uuid);

    /// \brief Clear install list.
    ///
    /// Clear the install list of the specified Mod Channel, if
    /// Mod Channel has no reference in the current Batch this operation has
    /// no effect.
    ///
    /// \param[in]  pModChan    : Pointer to Mod Channel.
    ///
    void instClear(const OmModChan* pModChan);

    /// \brief Add package to install list.
    ///
    /// Add the given Package references to the installation list of the
    /// specified Mod Channel.
    ///
    /// \param[in]  pModChan    : Pointer to Mod Channel object.
    /// \param[in]  pPkg    : Pointer to Package object to reference.
    ///
    void instAdd(const OmModChan* pModChan, const OmPackage* pPkg);

    /// \brief Remove package from install list.
    ///
    /// Remove the specified Package references from the installation list of
    /// the specified Mod Channel.
    ///
    /// \param[in]  pModChan    : Pointer to Mod Channel object.
    /// \param[in]  ident   : Package identity to search and remove.
    ///
    /// \return true if reference was removed, false otherwise
    ///
    bool instRem(const OmModChan* pModChan, const wstring ident);

    /// \brief Get install list size.
    ///
    /// Returns size of the install list for the specified Mod Channel.
    ///
    /// \param[in]  pModChan    : Pointer to Mod Channel object.
    ///
    /// \return Size of install list or 0 if Mod Channel not found.
    ///
    size_t instSize(const OmModChan* pModChan);

    /// \brief Get install list package
    ///
    /// Returns the found package, in the given Mod Channel, corresponding
    /// to the referenced item in the install list.
    ///
    /// \param[in]  pModChan    : Pointer to Mod Channel object.
    /// \param[in]  i       : Index of reference in install list.
    ///
    /// \return Pointer to Package object or nullptr if not found.
    ///
    OmPackage* instGet(const OmModChan* pModChan, unsigned i);


    /// \brief Get install list
    ///
    /// Returns the list of packages, in the given Mod Channel,
    /// corresponding to referenced install list items.
    ///
    /// \param[in]  pModChan    : Pointer to Mod Channel object.
    /// \param[in]  pkg_ls  : Array that receive list of Package objects.
    ///
    /// \return Count of found item.
    ///
    size_t instGetList(const OmModChan* pModChan, vector<OmPackage*>& pkg_ls);

    /// \brief Repair config.
    ///
    /// Verify then remove or repair invalid or broken references according
    /// current bound context.
    ///
    /// \return True if operation succeed, false if bound context missing.
    ///
    bool repair();

    /// \brief Rename Batch definition file.
    ///
    /// Rename the Batch definition file associated with this instance.
    ///
    /// \param[in]  name   : New Batch file name without extension.
    ///
    bool rename(const wstring& name);

    /// \brief Close batch file
    ///
    /// Close and reset the current batch data and configuration file.
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

    /// \brief Add log.
    ///
    /// Add entry to log file.
    ///
    void log(unsigned level, const wstring& head, const wstring& detail);

    /// \brief Get Mod Hub.
    ///
    /// Returns Batch related Mod Hub
    ///
    /// \return Pointer to Batch related Mod Hub.
    ///
    OmModHub* pModHub() const {
      return _modhub;
    }

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    OmModHub*           _modhub;

    OmConfig            _config;

    wstring             _path;

    wstring             _uuid;

    wstring             _title;

    unsigned            _index;

    bool                _instOnly;

    wstring             _error;

    bool                _migrate();
};

#endif // OMBATCH_H
